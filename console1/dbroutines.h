#pragma once

#include "problem.h"

//#include "windows.h"
#include <iostream>
using namespace std;

#include <omp.h>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>



class DBWorker
{
public:
	
	DBWorker() { _Connect(); };
	~DBWorker() { _Disconnect(); };

	int GenerateDefaultObjective(int idproblems);

	int DoRun(int idobjectives, const char* modelfile, bool mip=false);

	int GenerateBathch(char* batch_name);

	void RunBatch(int idbatches, char* modelfile, bool mip=false);
	void RunBatchOnMultipeModels(int idbatches, vector<string> modelfiles, bool mip);

	void PrintResultToStream(ostream& out, int idrun, bool mip=false);
	void PrintResultsToStreamByObjective(ostream& out, int idobjective);
	void PrintResultsByBatch(int idbatches);


protected:
	sql::Driver *driver;
	sql::Connection *con;

	void _Connect();
	void _Disconnect();

	int _RememberRun(CMyProblem &P, int idobjectives, const char* modelfile, double time, bool mip=false, int idruns=0);

	void _PrintRow(ostream& out, int idrun, bool mip, int i, char* var_name);

};