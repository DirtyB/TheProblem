#include "process.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct MyException {};

int ProcessProblem(char* model_file, char* data_file, char* solution_file)
{
	glp_prob *mip;
	glp_tran *tran;
	int ret;
	mip = glp_create_prob();
	tran = glp_mpl_alloc_wksp();
	try{
		
		ret = glp_mpl_read_model(tran, model_file, 1);
		if (ret != 0)
		{ 
			cout << "Error on translating model\n";
			throw MyException();
		}
		ret = glp_mpl_read_data(tran, data_file);
		if (ret != 0)
		{ 
			cout << "Error on translating data\n";
			throw MyException();
		}
		ret = glp_mpl_generate(tran, NULL);
		if (ret != 0)
		{ 
			cout << "Error on generating model\n";
			throw MyException();
		}
		glp_mpl_build_prob(tran, mip);
		glp_simplex(mip, NULL);
		glp_intopt(mip, NULL);
		ret = glp_mpl_postsolve(tran, mip, GLP_MIP);
		if (ret != 0)
		{
			cout << "Error on postsolving model\n";
			throw MyException();
		}
		//ret = glp_print_sol(mip, solution_file);
		ret = MyPrintSolution(mip, solution_file);
		if (ret != 0)
		{
			cout <<  "Error on printing solution\n";
			throw MyException();
		}
	}
	catch(MyException){
		glp_mpl_free_wksp(tran);
		glp_delete_prob(mip);
		return ret;
	}
	glp_mpl_free_wksp(tran);
	glp_delete_prob(mip);

	return 0;
}

char* GenerateColName(char* buff, char* name, int i, int j)
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

int MyPrintSolution(glp_prob* lp, char* solution_file)
{
	int n=glp_get_num_cols(lp);
	ofstream out(solution_file);
	if (!out)
		return 1;

	glp_create_index(lp);

	out << "LP solution" << endl;
	PrintSolArray(lp,"x",out);
	PrintSolArray(lp,"y",out);

	out << "MIP solution" << endl;
	PrintSolArray(lp,"x",out,true);
	PrintSolArray(lp,"y",out,true);

	glp_delete_index(lp);

	return 0;
	out.close();
}