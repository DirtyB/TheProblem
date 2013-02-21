#pragma once

#include "problem.h"

class Pool
{
private:
	vector<CMyProblem> problems;
	vector<vector<int>> restore_data;

public:
	void AddProblem(CMyProblem problem, vector<int> restore_data
};