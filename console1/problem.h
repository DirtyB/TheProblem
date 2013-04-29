#pragma once;

#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <glpk.h>
using namespace std;

#define AVAIL_ERROR 1.0e-12


class CMyProblem 
{
public:
	CMyProblem(int _n, int _p);
	CMyProblem(const CMyProblem &src);

	~CMyProblem();

	inline void Set_r(int index, int value);
	inline void Set_w(int index, double value);

	void Set_name(const char* p_name) { name = p_name; }; //вот тут надо суровую валидацию

	int		Get_r(int index) const;
	double	Get_w(int index) const;
	int		Get_n() const {return n;};
	int		Get_p() const {return p;};
	int		Get_d() const {return d;};

	inline string Get_name() {return name; };

	bool CMyProblem::IsInvalid() const;

	void Calculate_d();

	int ConstructLP(const char* model_file);
	int SolveLP();
	int SolveMIP();
	int PrintLPSolution(ostream &out);
	int PrintMIPSolution(ostream &out);

	double RealResult();

	glp_prob* GetProblem() { return lp; };

	void WriteTXT(const char* filename);
	void WriteMathProg(const char* filename);

	void ReadFromDB(int idproblem);
	int WriteToDB();

	void GenerateRandomProblem(int n, int p, string name, int max_r=0, int max_w=0);
	void GenerateRandomProblemWNC(int n, int p, string name, int max_w=0); //без пустых столбцов в расписании

	void SortByR();

	int Decomposite(vector<CMyProblem>& subproblems);
	
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

	vector<int> original_index; // для подзадач или задач с переставленными строками

	/*vector<vector<double>> x;
	vector<vector<double>> y;*/

	glp_prob* lp;	

	CMyProblem();


	bool InBounds(int index) const;

	void _alloc(int _n);
	void _copy(const CMyProblem &src);
	void _destroy();

	inline void _swap(int i1, int i2);
	CMyProblem _subproblem(int first_row, int rows, int offset, int part);


	inline void Set_p(int _p) { p = (_p<1)?1:_p; }

	inline	void Set_orig_index(int index, int value);
	inline	int  Get_orig_index(int index) const;

};