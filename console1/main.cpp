#include <ios>
#include <iostream>
#include "problem.h"
#include <stdlib.h>
#include <time.h>
#include <glpk.h>
//#include "process.h"

using namespace std;

struct WrongFileException{};

CMyProblem ReadProblemFromStream(istream &in)
{
	bool console=(in==cin);
	int n=0;
	int p=0;
	int r;
	double w;
	if (console)
		cout << "n (>=1): ";
	do {
		in >> n;
		if (!in)
			throw WrongFileException();
	} while (n<1);
	if (console)
		cout << "p (>=1): ";
	do {
		in >> p;
		if (!in)
			throw WrongFileException();
	} while (p<1);
	
	CMyProblem P(n,p);

	if (console)
		cout << "r (>0, " << n << " values): ";
	for(int i=0; i<n; i++)
	{
		do {
			in >> r;
			if (!in)
				throw WrongFileException();
		} while (r<0);
		P.Set_r(i,r);
	}

	if (console)
		cout << "w (>0, " << n << " values): ";
	for(int i=0; i<n; i++)
	{
		do {
			in >> w;
			if (!in)
				throw WrongFileException();
		} while (w<0);
		P.Set_w(i,w);
	}

	return P;
}


int Random(int min, int max)
{
	return min + (rand() % (int)(max - min + 1));
}

void ChangeObjective(glp_prob* lp)
{
	int nn = glp_get_num_cols(lp);
	for (int i=1;i<=nn;i++)
	{
		int ccc=Random(0,100);
		//cout << glp_get_col_name(lp,i) << " " << ccc << endl;
		glp_set_obj_coef(lp,i,ccc);
	}
}


void main(int argc, char* argv[])
{
	srand ( time(NULL) );
	char * filename = new char[256];
	char * filename2 = new char[256];
	char * prob_name = new char[256];
	CMyProblem P(0,0);
	istream* in;
	ifstream fin;
	if (argc<2)
	{
		cout << "Problem name: ";
		cin >> prob_name;
		fin.open(prob_name);
		if (!fin)
			in=&cin;
		else
			in=&fin;
	}
	else
	{
		strcpy(prob_name,argv[1]);
		fin.open(prob_name);
		if (!fin)
		{
			cout << prob_name << " not found" << endl;
			return;	
		}
		in=&fin;
	}

	try
	{
		P = ReadProblemFromStream(*in);
	}
	catch(WrongFileException)
	{
		cout << "Wrong file format" << endl;
		return;
	}
	
	P.Set_name(prob_name);

	strcpy(filename2,prob_name);
	strcpy(filename2+strlen(filename2),".csv");

	ofstream out(filename2);
	//��� ����� ����-������ ��������
	out << prob_name << endl;
	out.close();

	int res;


	res = P.ConstructLP("m2_int.mod");
	if (res!=0)
		return;

	ChangeObjective(P.GetProblem());

	P.SolveLP();
	out.open(filename2,ios_base::app);
	P.PrintLPSolution(out);
	out.close();


	for(int i=0; i<15; i++)
	{
		res = P.ConstructLP("m2_int.mod");
		if (res!=0)
			return;

		ChangeObjective(P.GetProblem());

		P.SolveLP();
		out.open(filename2,ios_base::app);
		P.PrintLPSolution(out);
		out.close();
	}

	/*P.SolveMIP();
	out.open(filename2,ios_base::app);
	P.PrintMIPSolution(out);
	out.close();*/
	
	
}