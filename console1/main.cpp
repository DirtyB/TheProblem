#include <ios>
#include <iostream>
#include "problem.h"
#include <stdlib.h>
#include <time.h>
#include <glpk.h>
#include "dbroutines.h"


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

void ChangeObjective(glp_prob* lp, bool bx=true, bool by=true)
{
	int nn = glp_get_num_cols(lp);
	for (int i=1;i<=nn;i++)
	{
		//cout << glp_get_col_name(lp,i) << " " << ccc << endl;
		const char* name = glp_get_col_name(lp,i);
		if (((name[0]=='x')&&bx)||((name[0]=='y')&&by))
		{
			int ccc=Random(0,100);
			glp_set_obj_coef(lp,i,ccc);
		}
		else
		{
			glp_set_obj_coef(lp,i,0);
		}
		//cout << name << " = " << glp_get_obj_coef(lp,i) << endl;
	}
}


void main(int argc, char* argv[])
{
	srand ( time(NULL) );
	char * filename = new char[256];
	char * filename2 = new char[256];
	char * prob_name = new char[256];
	CMyProblem P(0,0);

	P.ReadFromDB(1525);

	//cout << "written as" << P.WriteToDB() << endl;

	//P.WriteMathProg("1.txt");

	DBWorker wrkr;

	//	ofstream out("test.csv");
	//wrkr.PrintResultToStream(out,814,false);
	//wrkr.PrintResultsToStreamByObjective(out,2214);
	//out.close();
	wrkr.PrintResultsByBatch(17);
	return;

	ofstream out;

	/*wrkr.RunBatch(17,"m2_int.mod",true);
	wrkr.RunBatch(17,"m2_u1.mod",true);
	wrkr.RunBatch(17,"m2_u2.mod",true);*/

	/*cout << wrkr.GenerateDefaultObjective(1) << endl;
	cout << wrkr.GenerateDefaultObjective(2) << endl;*/

	//wrkr.DoRun(wrkr.GenerateDefaultObjective(2),"m2_int.mod",true);

	/*int batch = wrkr.GenerateBathch("test batch");

	wrkr.RunBatch(batch,"m2_int.mod",true);

	return;*/

	//P.WriteTXT(P.Get_name().c_str());
	/*P.SortByR();
	strcpy(filename2,P.Get_name().c_str());
	strcpy(filename2+strlen(filename2),"_sorted");	
	P.WriteTXT(filename2);*/

	/*vector<CMyProblem> subprob;
	P.Decomposite(subprob);
	for(int i=0;i<subprob.size();i++)
	{
		cout << subprob[i].Get_name() << endl;
		subprob[i].WriteTXT(subprob[i].Get_name().c_str());
	}
	
	//тут пока закончим
	return;*/


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

	out.open(filename2);
	//тут можно чего-нибудь пописать
	out << prob_name << endl;
	out.close();



	int res;

	res = P.ConstructLP("m2_uuu.mod");
	if (res!=0)
		return;

	P.SolveLP();
	out.open(filename2,ios_base::app);
	P.PrintLPSolution(out);
	out.close();

/*
	for(int i=0; i<15; i++)
	{
		res = P.ConstructLP("m2_int.mod");
		if (res!=0)
			return;

		ChangeObjective(P.GetProblem(),(i<10),(i>=5));
		glp_set_obj_dir(P.GetProblem(),GLP_MIN);

		P.SolveLP();
		out.open(filename2,ios_base::app);
		P.PrintLPSolution(out);
		out.close();
	}

	for(int i=0; i<15; i++)
	{
		res = P.ConstructLP("m2_int.mod");
		if (res!=0)
			return;

		ChangeObjective(P.GetProblem(),(i<10),(i>=5));
		glp_set_obj_dir(P.GetProblem(),GLP_MAX);

		P.SolveLP();
		out.open(filename2,ios_base::app);
		P.PrintLPSolution(out);
		out.close();
	}*/

	P.SolveMIP();
	out.open(filename2,ios_base::app);
	P.PrintMIPSolution(out);
	out.close();
	
	
}