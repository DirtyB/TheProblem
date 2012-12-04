#pragma once;

#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <glpk.h>
using namespace std;

class CMyProblem 
{
public:
	CMyProblem(int _n, int _p);
	CMyProblem(const CMyProblem &src);

	~CMyProblem();

	void Set_r(int index, int value);
	void Set_w(int index, double value);

	void Set_name(char* p_name) { name = p_name; }; //вот тут надо суровую валидацию

	int		Get_r(int index) const;
	double	Get_w(int index) const;
	int		Get_n() const {return n;};
	int		Get_p() const {return p;};
	int		Get_d() const {return d;};

	string Get_name() {return name; };

	bool CMyProblem::IsInvalid() const;

	void Calculate_d();

	int ConstructLP(const char* model_file);
	int SolveLP();
	int SolveMIP();
	int PrintLPSolution(ostream &out);
	int PrintMIPSolution(ostream &out);

	glp_prob* GetProblem() { return lp; };

	void WriteTXT(const char* filename);
	void WriteMathProg(const char* filename);
	
	CMyProblem& operator= (const CMyProblem &src);

private:
	string name;
	int n;
	int p;
	int d;
	//int* r;
	//double* w;
	vector<int> r;
	vector<double> w;

	glp_prob* lp;	

	CMyProblem();


	bool InBounds(int index) const;

	void _alloc(int _n);
	void _copy(const CMyProblem &src);
	void _destroy();

};