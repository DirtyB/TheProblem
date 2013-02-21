#include "problem.h"
#include <cmath>
#include <time.h>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#define MAXSTACK 2048 // максимальный размер стека

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

	Set_p(_p);
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
	original_index.resize(n,0);
	for(int i=0;i<n;i++)
		original_index[i]=i;

	lp = glp_create_prob();
}

void CMyProblem::_copy(const CMyProblem &src) 
{
	name = src.name;	
	n=src.n;
	p=src.p;
	r=src.r;
	w=src.w;
	original_index=src.original_index;
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
		sql::ResultSet *res;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://localhost:3306", "root", "TestPassword");
		/* Connect to the MySQL test database */
		con->setSchema("theproblem");
		
		//cout<< "Connected succesfully";

		ProblemStmt = con->prepareStatement("SELECT name, n, p FROM problems WHERE idproblems=?");
		ProblemStmt->setInt(1, idproblem);
		res = ProblemStmt->executeQuery();
		delete ProblemStmt;
		
		//cout<< "Statment executed succesfully";

		res->next();
		name = res->getString("name");
		n = res->getInt("n");
		p = res->getInt("p");
		delete res;

		//cout << name << n << p;

		_alloc(n);

		ProblemStmt = con->prepareStatement(
			"SELECT i, r, w FROM parameters WHERE idproblems=?"
			);
		ProblemStmt->setInt(1, idproblem);
		res = ProblemStmt->executeQuery();
		int i; //int r; double w;
		while(res->next())
		{
			i = res->getInt("i");
			Set_r(i-1,res->getInt("r"));
			Set_w(i-1,res->getDouble("w"));
			//cout << res->getInt("i") << " " << res->getInt("r") << " " << res->getDouble("w") << endl;
		}
		delete res;
		delete ProblemStmt;

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


int CMyProblem::WriteToDB()
{
	int idproblems;
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement *PrepStmt;
		sql::ResultSet *res;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://localhost:3306", "root", "TestPassword");
		/* Connect to the MySQL test database */
		con->setSchema("theproblem");
		
		PrepStmt = con->prepareStatement(
			"INSERT INTO  problems(name, n, p) values (?,?,?)"
			);
		PrepStmt->setString(1, name);
		PrepStmt->setInt(2, n);
		PrepStmt->setInt(3, p);
		PrepStmt->execute();
		delete PrepStmt;
		
		PrepStmt = con->prepareStatement(
			"SELECT LAST_INSERT_ID()"
			);
		res = PrepStmt->executeQuery();
		delete PrepStmt;
		res->next();
		idproblems = res->getInt(1);
		delete res;

		PrepStmt = con->prepareStatement(
			"INSERT INTO parameters(idproblems, i, r, w) values (?,?,?,?)"
			);
		PrepStmt->setInt(1, idproblems);
		for (int i=0; i<n; i++)
		{
			PrepStmt->setInt(2, i+1);
			PrepStmt->setInt(3, r[i]);
			PrepStmt->setDouble(4, w[i]);
			PrepStmt->execute();
		}
		delete PrepStmt;

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
	return idproblems;
}

int myRandom(int min, int max)
{
	return min + (rand() % (int)(max - min + 1));
}

void CMyProblem::GenerateRandomProblem(int _n, int _p, string _name, int max_r, int max_w)
{

	_alloc(_n);
	Set_p(_p);
	Set_name(_name.c_str());
	for (int i=0; i<n; i++)
	{
		r[i]=myRandom(0,(max_r!=0)?max_r:p*n+1);
		w[i]=myRandom(1,(max_w!=0)?max_w:100);
		//cout << r[i] << ' ' << w[i]<< endl;
	}
	//cout << endl;
}

void CMyProblem::Set_orig_index(int index, int value)
{
	/*if (!InBounds(index))
		return;
	if (value<0)
		original_index[index]=0;
	else*/
		original_index[index]=value;
}

int CMyProblem::Get_orig_index(int index) const
{
	/*if (!InBounds(index))
		return 0;*/
	return original_index[index];
}	

inline void CMyProblem::_swap(int i1, int i2)
{
	int temp_r=r[i1];
	double temp_w=w[i1];
	int temp_orig_index = original_index[i1];

	r[i1]=r[i2];
	w[i1]=w[i2];
	original_index[i1]=original_index[i2];

	r[i2]=temp_r;
	w[i2]=temp_w;
	original_index[i2]=temp_orig_index;
}


void CMyProblem::SortByR() //сортируем в порядке возрастания r[i]. Быстрая сортировка
{
	long i, j; // указатели, участвующие в разделении
	long lb, ub; // границы сортируемого в цикле фрагмента
	 
	long lbstack[MAXSTACK], ubstack[MAXSTACK]; // стек запросов
	// каждый запрос задается парой значений,
	// а именно: левой(lbstack) и правой(ubstack)
	// границами промежутка
	long stackpos = 1; // текущая позиция стека
	long ppos; // середина массива
	int pivot; // опорный элемент
	//T temp;
	 
	lbstack[1] = 0;
	ubstack[1] = n-1;
	 
	do {
		// Взять границы lb и ub текущего массива из стека.
		lb = lbstack[ stackpos ];
		ub = ubstack[ stackpos ];
		stackpos--;
		 
		do {
			// Шаг 1. Разделение по элементу pivot
			ppos = ( lb + ub ) >> 1;
			i = lb; j = ub; pivot = r[ppos];
			do {
				while ( r[i] < pivot ) i++;
				while ( pivot < r[j] ) j--;
				if ( i <= j ) {
					//temp = a[i]; r[i] = a[j]; a[j] = temp;
					_swap(i,j);
					i++; j--;
				}
			} while ( i <= j );
	 
			// Сейчас указатель i указывает на начало правого подмассива,
			// j - на конец левого (см. иллюстрацию выше), lb ? j ? i ? ub.
			// Возможен случай, когда указатель i или j выходит за границу массива
			 
			// Шаги 2, 3. Отправляем большую часть в стек и двигаем lb,ub
			if ( i < ppos ) { // правая часть больше
				if ( i < ub ) { // если в ней больше 1 элемента - нужно
					stackpos++; // сортировать, запрос в стек
					lbstack[ stackpos ] = i;
					ubstack[ stackpos ] = ub;
				}
				ub = j; // следующая итерация разделения
				// будет работать с левой частью
				} else { // левая часть больше
					if ( j > lb ) {
					stackpos++;
					lbstack[ stackpos ] = lb;
					ubstack[ stackpos ] = j;
				}
				lb = i;
			}
		} while ( lb < ub ); // пока в меньшей части более 1 элемента
	} while ( stackpos != 0 ); // пока есть запросы в стеке
}

CMyProblem CMyProblem::_subproblem(int first_row, int rows, int offset, int part)
{
	CMyProblem SP(rows,p);
	char new_name[256];//опасность!!
	//cout << name << " " << part << endl;
	sprintf(new_name,"%s_%d",name.c_str(),part);
	//cout << new_name << endl;
	SP.Set_name(new_name);
	//cout << SP.Get_name()<< endl;
	for(int i=0; i<rows; i++)
	{
		SP.Set_r(i,r[first_row+i]-offset);
		SP.Set_w(i,w[first_row+i]);
		SP.Set_orig_index(i,original_index[first_row+i]);
	}
	return SP;
}

int CMyProblem::Decomposite(vector<CMyProblem>& subproblems)
{
	SortByR();
	int offset=0;
	int rows=0;
	subproblems.clear();
	for(int i=0;i<n;i++)
	{
		if (r[i]-offset < rows*p)
		{
			rows++;
		}
		else
		{
			if (rows>0)
			{
				cout << i-rows << " " << rows << " " << offset << " " << subproblems.size() << endl;
				subproblems.push_back(_subproblem(i-rows,rows,offset,subproblems.size()));
				cout << subproblems[subproblems.size()-1].Get_name() << endl;
			}
			offset=r[i];
			rows=1;
		}
	}
	cout<< n-rows << " " << rows << " " << offset << " " << subproblems.size() << endl;
	subproblems.push_back(_subproblem(n-rows,rows,offset,subproblems.size()));
	return subproblems.size();
}