#include <iostream>
#include "problem.h"
#include "process.h"

using namespace std;

CMyProblem ReadProblemFromKeyboard()
{
	int n=0;
	int p=0;
	int r;
	double w;
	cout << "n (>=1): ";
	do {
		cin >> n;
	} while (n<1);
	cout << "p (>=1): ";
	do {
		cin >> p;
	} while (p<1);
	
	CMyProblem P(n,p);

	cout << "r (>0, " << n << " values): ";
	for(int i=0; i<n; i++)
	{
		do {
			cin >> r;
		} while (r<0);
		P.Set_r(i,r);
	}

	cout << "w (>0, " << n << " values): ";
	for(int i=0; i<n; i++)
	{
		do {
			cin >> w;
		} while (w<0);
		P.Set_w(i,w);
	}

	return P;
}


void main()
{
	ProcessProblem("1.mod","1.dat","1.sol");
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