#include "process.h"
#include <iostream>
#include <fstream>
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
		//glp_intopt(mip, NULL);
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

	glp_mpl_
	return 0;
}

int MyPrintSolution(glp_prob* lp, char* solution_file)
{
	int n=glp_get_num_cols(lp);
	ofstream out(solution_file);
	if (!out)
		return 1;
	for(int i=1; i<=n; i++)
	{
		out << glp_get_col_name(lp,i) << " = " << glp_get_col_prim(lp,i) << endl;
	}
	return 0;
	out.close();
}