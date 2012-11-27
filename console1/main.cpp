#include <iostream>
#include "problem.h"
#include "process.h"

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


void main(int argc, char* argv[])
{
	char * filename = new char[256];
	char * filename2 = new char[256];
	char * prob_name = new char[256];
	CMyProblem P(0,0);
	if (argc<2)
	{
		cout << "Problem name: ";
		cin >> prob_name;
		P = ReadProblemFromStream(cin);
	}
	else
	{
		ifstream in(argv[1]);
		if (!in)
		{
			cout << argv[1] << " not found" << endl;
			return;
		}
		try
		{
			P = ReadProblemFromStream(in);
		}
		catch(WrongFileException)
		{
			cout << "Wrong file format" << endl;
		}
		strcpy(prob_name,argv[1]);
	}

	strcpy(filename,prob_name);
	strcpy(filename+strlen(filename),".dat");
	P.WriteMathProg(filename);

	strcpy(filename2,prob_name);
	strcpy(filename2+strlen(filename2),".csv");

	ProcessProblem("m2_int.mod",filename,filename2);

	/*char * filename = new char[256];
	while(1)
	{
		cout << "Name: ";
		cin >> filename;
		if (strcmp(filename,"exit")==0)
			break;
		strcpy(filename+strlen(filename),".dat");
		CMyProblem P = ReadProblemFromKeyboard();
		P.WriteMathProg(filename);
	}
	delete []filename;*/
}