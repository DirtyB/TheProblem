#include "problem.h"

bool CMyProblem::IsInvalid() const
{
	return (n==0);
}

bool CMyProblem::InBounds(int index) const
{
	return ((index>=0)&&(index<n));
}

CMyProblem::CMyProblem(int _n, int _p)
{
	n=0;
	if ((_n<1)||(_p<1))
		return;

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
	try{
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
	}
}

void CMyProblem::_copy(const CMyProblem &src) 
{
	n=src.n;
	p=src.p;
	for (int i=0;i<n;i++)
	{
		r[i]=src.r[i];
		w[i]=src.w[i];
	}
}

void CMyProblem::_destroy()
{
	if (n!=0) {
		delete[]r;
		delete[]w;
	}
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