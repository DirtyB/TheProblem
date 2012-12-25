#include "problem.h"
#include <cmath>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

struct MyException {};


bool CMyProblem::IsInvalid() const
{
	return (n==0);
}

bool CMyProblem::InBounds(int index) const
{
	return ((index>=0)&&(index<n));
}

CMyProblem::CMyProblem()
{
	n=0;
	_alloc(0);
}


CMyProblem::CMyProblem(int _n, int _p)
{
	if ((_n<1)||(_p<1))
		n=0;
	else
		n=n;

	_alloc(_n);

	p=_p;
};

CMyProblem::CMyProblem(const CMyProblem &src)
{
	if (src.IsInvalid())
	{
		n=0;
		return;
	}

	_alloc(src.n);
	_copy(src);
}

CMyProblem& CMyProblem::operator= (const CMyProblem &src)
{
	if (this == &src)
		return *this;

	if (src.n!=n)
		_destroy();

	if (src.IsInvalid())
	{
		n=0;
		return *this;
	}

	_alloc(src.n);
	_copy(src);
	return *this;
}

void CMyProblem::_alloc( int _n)
{
	n=_n;
	r.resize(n,0);
	w.resize(n,0);
	lp = glp_create_prob();
	/*try{
		r=new int[n];
		w=new double[n];
		for (int i=0; i<n; i++)
		{
			r[i]=0;
			w[i]=0;
		}
	}
	catch(bad_alloc){
		if (r!=0)
			delete[]r;
		n=0;
	}*/
}

void CMyProblem::_copy(const CMyProblem &src) 
{
	n=src.n;
	p=src.p;
	r=src.r;
	w=src.w;
	glp_copy_prob(lp,src.lp,GLP_ON);
	/*for (int i=0;i<n;i++)
	{
		r[i]=src.r[i];
		w[i]=src.w[i];
	}*/
}

void CMyProblem::_destroy()
{
	glp_delete_prob(lp);
}


CMyProblem::~CMyProblem()
{
	_destroy();
}

void CMyProblem::Set_r(int index, int value)
{
	if (!InBounds(index))
		return;
	if (value<0)
		r[index]=0;
	else
		r[index]=value;
}

void CMyProblem::Set_w(int index, double value)
{
	if (!InBounds(index))
		return;
	if (value<0)
		w[index]=0;
	else
		w[index]=value;
}

int CMyProblem::Get_r(int index) const
{
	if (!InBounds(index))
		return 0;
	return r[index];
}	

double CMyProblem::Get_w(int index) const
{
	if (!InBounds(index))
		return 0.0;
	return w[index];
}	

void CMyProblem::WriteTXT(const char* filename)
{
	ofstream out(filename);
	if (!out)
		return;

	out << n << ' ' << p << endl;
	for (int i=0; i<n; i++)
		out << r[i] << ' ';
	out << endl;
	for (int i=0; i<n; i++)
		out << w[i] << ' ';
	out << endl;

	out.close();
}

void CMyProblem::WriteMathProg(const char* filename)
{
	ofstream out(filename);
	if (!out)
		return;

	out << "data;" << endl;
	out << "" << endl;
	out << "param n:=" << n << ";" << endl;
	out << "param p:=" << p << ";" << endl;

	out << "param r:=";
	for (int i=0; i<n; i++)
		out << "[" << i+1 << "] " << r[i] << " ";
	out << ";" << endl;

	out << "param w:=";
	for (int i=0; i<n; i++)
		out << "[" << i+1 << "] " << w[i] << " ";
	out<< ";" << endl;

	out << "" << endl;
	out << "end;" << endl;

	out.close();
}

void CMyProblem::Calculate_d()
{
	//пока заглушка
	d=0;
	for (int i=0;i<n;i++)
		if (r[i]>d)
			d=r[i];
	d+=n*p;
}

int CMyProblem::ConstructLP(const char* model_file)
{
	//надо проверить чего-нибудь
	glp_erase_prob(lp);

	glp_tran *tran;
	int ret;
	tran = glp_mpl_alloc_wksp();
	try{
		
		ret = glp_mpl_read_model(tran, model_file, 1);
		if (ret != 0)
		{ 
			cerr << "Error on translating model\n";
			throw MyException();
		}
		string data_file = name;
		data_file+=".dat";
		WriteMathProg(data_file.c_str());
		ret = glp_mpl_read_data(tran, data_file.c_str());
		if (ret != 0)
		{ 
			cerr << "Error on translating data\n";
			throw MyException();
		}
		ret = glp_mpl_generate(tran, NULL);
		if (ret != 0)
		{ 
			cerr << "Error on generating model\n";
			throw MyException();
		}
		glp_mpl_build_prob(tran, lp);
		glp_set_prob_name(lp, name.c_str());
		//вот тут я разрушаю транслятор, что делать с postsolve непонятно, но пока оно не надо
		glp_mpl_free_wksp(tran);
	}
	catch(MyException){
		glp_mpl_free_wksp(tran);
		glp_erase_prob(lp);
		return ret;
	}
	return 0;
}

int CMyProblem::SolveLP()
{
	return glp_simplex(lp, NULL);
}

int CMyProblem::SolveMIP()
{
	return glp_intopt(lp, NULL);	
}

inline char* GenerateColName(char* buff, char* name, int i, int j)
{
	sprintf(buff, "%s[%d,%d]",name,i,j);
	return buff;
}

inline bool IsNotBinary(double x)
{
	return abs(abs(x-0.5)-0.5)>AVAIL_ERROR;
}

inline void PrintSolArray(glp_prob* lp,  char* name, ostream &out, bool integer=false)
{
	out << name << endl;
	char buff[256];
	int i=1;
	int j=1;
	double value;
	int col_num=glp_find_col(lp,GenerateColName(buff, name, i, j));
	while( col_num )
	{
		if (!integer)
		{
			value = glp_get_col_prim(lp,col_num);

			//проверка иксов на целочисленность
			if ((strcmp(name,"x")==0)&&(IsNotBinary(value)))
			{
				out<<"!!! ";
			}
		}
		else
		{
			value = glp_mip_col_val(lp,col_num);
		}
		out << value << ";";

		//допишем коэффициенты ещё
		//out << "*(" << glp_get_obj_coef(lp,col_num) << ");";

		j++;
		col_num=glp_find_col(lp,GenerateColName(buff, name, i, j));
		if (!col_num)
		{
			out << endl;
			i++;
			j=1;
			col_num=glp_find_col(lp,GenerateColName(buff, name, i, j));
		}
	}
}

double CMyProblem::RealResult()
{
	char buff[256];
	int i=1;
	int j=1;
	double value;
	int add;
	int result=0;
	int col_num=glp_find_col(lp,GenerateColName(buff, "x", i, j));
	while( col_num )
	{
		value = glp_get_col_prim(lp,col_num);
		if ( abs(1.0-value)<AVAIL_ERROR )
		{
			add=j*w[i-1];
		}

		j++;
		col_num=glp_find_col(lp,GenerateColName(buff, "x", i, j));
		if (!col_num)
		{
			result+=add;
			i++;
			j=1;
			col_num=glp_find_col(lp,GenerateColName(buff, "x", i, j));
		}
	}
	return result;
}

char* DecodeStatus(int status)
{
	switch (status){
		case GLP_OPT: return "solution is optimal";
		case GLP_FEAS: return "solution is feasible (but may not be optimal)";
		case GLP_INFEAS: return "solution is infeasible";
		case GLP_NOFEAS: return "problem has no feasible solution";
		case GLP_UNBND: return "problem has unbounded solution";
		case GLP_UNDEF: return "solution is undefined";
	}
	return "unknown status";
}


int CMyProblem::PrintLPSolution(ostream &out)
{
	glp_create_index(lp);
	out << "LP solution" << endl;
	out << "Dir;" << ((glp_get_obj_dir(lp)==GLP_MIN) ? "min" : "max") << endl;
	out << "f; " << glp_get_obj_val(lp) << ";/*" << RealResult() << "*/" << endl; 
	out << "Status;" << DecodeStatus(glp_get_status(lp)) << endl;
	PrintSolArray(lp,"x",out);
	PrintSolArray(lp,"y",out);
	glp_delete_index(lp);
	return 0;
}

int CMyProblem::PrintMIPSolution(ostream &out)
{
	glp_create_index(lp);
	out << "MIP solution" << endl;
	out << "Dir;" << ((glp_get_obj_dir(lp)==GLP_MIN) ? "min" : "max") << endl;
	out << "f;" << glp_mip_obj_val(lp) << endl; 
	out << "Status;" << DecodeStatus(glp_mip_status(lp)) << endl;
	PrintSolArray(lp,"x",out,true);
	PrintSolArray(lp,"y",out,true);
	glp_delete_index(lp);
	return 0;
}

void CMyProblem::ReadFromDB(int idproblem)
{
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *ProblemStmt;
		sql::PreparedStatement *ParamsStmt;
		sql::ResultSet *res;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://localhost:3306", "root", "TestPassword");
		/* Connect to the MySQL test database */
		con->setSchema("theproblem");
		
		cout<< "Connected succesfully";

		delete con;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " 
			<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << 
			" )" << endl;
	}
}
