#include "problem.h"

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

inline void PrintSolArray(glp_prob* lp,  char* name, ostream &out, bool integer=false)
{
	out << name << endl;
	char buff[256];
	int i=1;
	int j=1;
	int col_num=glp_find_col(lp,GenerateColName(buff, name, i, j));
	while( col_num )
	{
		if (!integer)
			out << glp_get_col_prim(lp,col_num) << ";";
		else
			out << glp_mip_col_val(lp,col_num) << ";";
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

int CMyProblem::PrintLPSolution(ostream &out)
{
	glp_create_index(lp);
	out << "LP solution" << endl;
	PrintSolArray(lp,"x",out);
	PrintSolArray(lp,"y",out);
	glp_delete_index(lp);
	return 0;
}

int CMyProblem::PrintMIPSolution(ostream &out)
{
	glp_create_index(lp);
	out << "MIP solution" << endl;
	PrintSolArray(lp,"x",out,true);
	PrintSolArray(lp,"y",out,true);
	glp_delete_index(lp);
	return 0;
}