#pragma once;

#include <fstream>
#include <string.h>
using namespace std;

class CMyProblem 
{
public:
	CMyProblem(int _n, int _p);
	CMyProblem(const CMyProblem &src);

	~CMyProblem();

	void Set_r(int index, int value);
	void Set_w(int index, double value);

	int		Get_r(int index) const;
	double	Get_w(int index) const;
	int		Get_n() const {return n;} ;
	int		Get_p() const {return p;};

	bool CMyProblem::IsInvalid() const;

	void WriteTXT(const char* filename);
	void WriteMathProg(const char* filename);
	
	CMyProblem& operator= (const CMyProblem &src);

private:
	int n;
	int p;
	int* r;
	double* w;


	CMyProblem(){};

	bool InBounds(int index) const;

	void _alloc(int _n);
	void _copy(const CMyProblem &src);
	void _destroy();

};