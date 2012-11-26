#pragma once
#include <glpk.h>

int ProcessProblem(char* model_file, char* data_file, char* solution_file);

int MyPrintSolution(glp_prob* lp, char* solution_file);