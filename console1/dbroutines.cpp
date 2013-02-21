#include "dbroutines.h"

void inline SQLError(sql::SQLException &e)
{
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " 
			<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << 
			" )" << endl;
}

inline char* GenerateColName(char* buff, char* name, int i, int j)
{
	sprintf(buff, "%s[%d,%d]",name,i,j);
	//cout << buff;
	return buff;
}


inline void GetSolArray(glp_prob* lp,  char* name, vector<vector<double>> &arr, bool integer=false)
{
	arr.clear();
	char buff[256];
	int i=1;
	int j=1;
	double value;
	glp_create_index(lp);
	int col_num=glp_find_col(lp,GenerateColName(buff, name, i, j));
	vector<double> row;
	while( col_num )
	{
		if (!integer)
		{
			value = glp_get_col_prim(lp,col_num);
		}
		else
		{
			value = glp_mip_col_val(lp,col_num);
		}
		row.push_back(value);

		//cout << value << ";";

		j++;
		col_num=glp_find_col(lp,GenerateColName(buff, name, i, j));
		if (!col_num)
		{
			arr.push_back(row);
			//cout << endl;
			row.clear();
			i++;
			j=1;
			col_num=glp_find_col(lp,GenerateColName(buff, name, i, j));
		}
	}
}

void DBWorker::_Connect()
{
	driver=0;
	con=0;
	try{
		driver = get_driver_instance();
		con = driver->connect("tcp://localhost:3306", "root", "TestPassword");
		/* Connect to the MySQL test database */
		con->setSchema("theproblem");
	}
	catch (sql::SQLException &e) {
		SQLError(e);
	}
}

void DBWorker::_Disconnect()
{
	if(con)
		delete con;
}

int DBWorker::GenerateDefaultObjective(int idproblem)
{
	try {
		sql::PreparedStatement *PrepStmt;
		sql::PreparedStatement *PrepStmt2;
		sql::ResultSet *res;

		PrepStmt = con->prepareStatement("SELECT idobjectives FROM objectives WHERE idproblems=? and default_obj=1");
		PrepStmt->setInt(1, idproblem);
		res = PrepStmt->executeQuery();
		delete PrepStmt;
		
		if(res->next())
		{
			int ret = res->getInt(1);
			delete res;
			return ret;
		}
		delete res;

		PrepStmt2 = con->prepareStatement("INSERT INTO objectives(idproblems,direction,default_obj) VALUES(?,'min',1)");
		PrepStmt2->setInt(1, idproblem);
		PrepStmt2->execute();
		delete PrepStmt2;
		
		return GenerateDefaultObjective(idproblem);
	}
	catch (sql::SQLException &e) {
		SQLError(e);
	}
}

int DBWorker::DoRun(int idobjectives, char* modelfile, bool mip)
{
	int idruns;
	try{
		sql::PreparedStatement *PrepStmt;
		sql::ResultSet *res;
		PrepStmt = con->prepareStatement(
			"SELECT idproblems, default_obj FROM objectives WHERE idobjectives=?"
			);
		PrepStmt->setInt(1, idobjectives);
		res = PrepStmt->executeQuery();
		delete PrepStmt;
		if(!res->next())
			return -1; //no objective
		bool def_obj = res->getInt("default_obj");
		int idproblems = res->getInt("idproblems");
		delete res;

		CMyProblem P(0,0);
		P.ReadFromDB(idproblems);
		P.ConstructLP(modelfile);

		if(!def_obj)
		{
			//здесь будет загрузка нестандартной ЦФ
		}

		double time;

		//решение задачи ЛП + запись результатов
		time = omp_get_wtime();
		P.SolveLP();
		time = omp_get_wtime()-time;
		idruns = _RememberRun(P, idobjectives, modelfile, time, false);
		if (mip)
		{
			//решение задачи ЦЛП + запись результатов
			time = omp_get_wtime();
			P.SolveMIP();
			time = omp_get_wtime()-time;
			_RememberRun(P, idobjectives, modelfile, time, true, idruns);
		}

	}
	catch (sql::SQLException &e) {
		SQLError(e);
	}
	return idruns;
}

int DBWorker::_RememberRun(CMyProblem &P, int idobjectives, char* modelfile, double time, bool mip, int idruns)
{
	int idrun = -1;
	try{
		sql::PreparedStatement *PrepStmt;
		sql::ResultSet *res;

		PrepStmt = con->prepareStatement(
			"INSERT INTO runs(idobjectives,modelfile,runtype,res_status,res_value,time_in_seconds,idruns) VALUES(?,?,?,?,?,?,?)"
			);
		PrepStmt->setInt(1, idobjectives);
		PrepStmt->setString(2, modelfile);
		PrepStmt->setString(3, mip ? "mip" : "lp");
		if(!mip)
		{
			PrepStmt->setInt(4, glp_get_status(P.GetProblem()));
			PrepStmt->setDouble(5, glp_get_obj_val(P.GetProblem()));
			PrepStmt->setNull(7,0);
		}
		else
		{
			PrepStmt->setInt(4, glp_mip_status(P.GetProblem()));
			PrepStmt->setDouble(5, glp_mip_obj_val(P.GetProblem()));
			PrepStmt->setInt(7,idruns);
		}
		PrepStmt->setDouble(6, time);
		
		PrepStmt->execute();
		delete PrepStmt;

		if(!mip)
		{
			PrepStmt = con->prepareStatement(
				"SELECT LAST_INSERT_ID()"
				);
			res = PrepStmt->executeQuery();
			delete PrepStmt;
			res->next();
			idrun = res->getInt(1);
			delete res;
		}
		else
		{
			idrun = idruns;
		}

		cout << "Run ID " << idrun << endl;

		PrepStmt = con->prepareStatement(
			"INSERT INTO results(idrun,var_name,i,j,value,runtype) VALUES(?,?,?,?,?,?)"
			);
		PrepStmt->setInt(1, idrun);
		PrepStmt->setString(6, mip ? "mip" : "lp");

		vector<vector<double>> arr;
		for (int vvv=1; vvv>=0; vvv--)
		{
			char* var_name = (vvv ? "x" : "y");
			GetSolArray(P.GetProblem(),var_name,arr,mip);
			PrepStmt->setString(2, var_name);
			int i=1;
			for (std::vector<vector<double>>::iterator it = arr.begin() ; it != arr.end(); ++it)
			{
				int j=1;
				for (std::vector<double>::iterator it2 = (*it).begin() ; it2 != (*it).end(); ++it2)
				{
					PrepStmt->setInt(3, i);
					PrepStmt->setInt(4, j);
					PrepStmt->setDouble(5, *it2);
					PrepStmt->execute();
					j++;
				}
				i++;
			}
		}

		delete PrepStmt;

	}
	catch (sql::SQLException &e) {
		SQLError(e);
	}
	return idrun;
}

int DBWorker::GenerateBathch(char* batch_name)
{
	int idbatches=0;
	try{
		sql::PreparedStatement *PrepStmt;
		sql::ResultSet *res;

		PrepStmt = con->prepareStatement(
			"INSERT INTO batches(name) VALUES(?)"
			);
		PrepStmt->setString(1,batch_name);
		PrepStmt->execute();

		PrepStmt = con->prepareStatement(
				"SELECT LAST_INSERT_ID()"
				);
		res = PrepStmt->executeQuery();
		delete PrepStmt;
		res->next();
		idbatches = res->getInt(1);
		delete res;

		PrepStmt = con->prepareStatement(
			"INSERT INTO batch_elem(idbatches,bat_order,idobjectives) VALUES(?,?,?)"
			);
		PrepStmt->setInt(1,idbatches);

		int i=1;
		CMyProblem P(0,0);
		for(int n=2; n<=15; n++)
		{	
			while (i<=n*10)
			{
				char buff[256];
				sprintf(buff, "b%d-%d_%d",idbatches,n,i);
				P.GenerateRandomProblem(n,3,buff);
				int idproblems = P.WriteToDB();
				int idobjectives = GenerateDefaultObjective(idproblems);
				PrepStmt->setInt(2,i);
				PrepStmt->setInt(3,idobjectives);
				PrepStmt->execute();
				i++;
			}
		}


		delete PrepStmt;
	}
	catch (sql::SQLException &e) {
		SQLError(e);
	}
	return idbatches;
}

void DBWorker::RunBatch(int idbatches, char* modelfile, bool mip){
	try{
		sql::PreparedStatement *PrepStmt;
		sql::ResultSet *res;

		PrepStmt = con->prepareStatement(
			"select bat_order, idobjectives from batch_elem where idbatches=? order by bat_order"
			);
		PrepStmt->setInt(1,idbatches);
		res = PrepStmt->executeQuery();
		delete PrepStmt;
		
		int idobjective;
		while(res->next())
		{
			idobjective = res->getInt(2);
			DoRun(idobjective,modelfile,mip);
		}
		delete res;	
	
	}
	catch (sql::SQLException &e) {
		SQLError(e);
	}
}

void DBWorker::PrintResultToStream(ostream& out, int idrun, bool mip)
{
	try{
		sql::PreparedStatement *PrepStmt;
		sql::PreparedStatement *RowStmt;
		sql::ResultSet *res;

		PrepStmt = con->prepareStatement(
			"select  p.name, p.n, p.p, o.default_obj, o.direction, r.modelfile, r.runtype, s.description status_desc, r.res_value, r.time_in_seconds from runs r, objectives o, problems p, status s where r.idobjectives = o.idobjectives  and o.idproblems = p.idproblems and r.res_status = s.idstatus and idruns = ? and runtype=?;"
			);
		PrepStmt->setInt(1,idrun);
		if (mip)
			PrepStmt->setString(2,"mip");
		else
			PrepStmt->setString(2,"lp");
		res = PrepStmt->executeQuery();
		delete PrepStmt;

		int n=0;
		
		if(res->next())
		{
			out << "Name:;;" << res->getString(1).c_str() << endl;
			out << "n:;;" << res->getInt(2) << endl;
			out << "p:;;" << res->getInt(3) << endl;
			out << "Model file:;;" << res->getString(6).c_str() << endl;
			out << "Run type:;;" << res->getString(7).c_str() << endl;
			out << "Sotution status:;;" << res->getString(8).c_str() << endl;
			out << "Time (in seconds):;;" << res->getDouble(10) << endl;
			out << "Objective value:;;" << res->getDouble(9) << endl;

			n=res->getInt(2);
		}
		delete res;	

		PrepStmt = con->prepareStatement(
			"select  par.i,  par.r, par.w from runs r, objectives o, problems p, parameters par where r.idobjectives = o.idobjectives and o.idproblems = p.idproblems and par.idproblems = p.idproblems and idruns = ? and runtype=? order by i"
			);
		PrepStmt->setInt(1,idrun);
		if (mip)
			PrepStmt->setString(2,"mip");
		else
			PrepStmt->setString(2,"lp");
		res = PrepStmt->executeQuery();
		delete PrepStmt;

		out << "r;w;;x" << endl;
		int l_i=0;
		int l_j=0;
		for(int i=1; i<=n; i++)
		{
			while (l_i<i)
			{
				if (!res->next())
					l_i=n+1;
				l_i = res->getInt(1);
			}
			if (l_i==i)
			{
				out << res->getInt(2) << ";" << res->getInt(3) << ";;";
			}
			else
			{
				out << ";;;";
			}

			out << endl;
		}
	
	}
	catch (sql::SQLException &e) {
		SQLError(e);
	}
}

void _PrintRow(ostream& out, int idrun, bool mip, int i, char* var_name)
{
	try{
		sql::PreparedStatement *PrepStmt;
		sql::ResultSet *res;

		PrepStmt = con->prepareStatement(
			"select j, value from  results res where res.idrun = ?  and res.runtype=?  and i = ? and var_name = ? order by i;"
			);
		PrepStmt->setInt(1,idbatches);
		res = PrepStmt->executeQuery();
		delete PrepStmt;
		
		while(res->next())
		{
		}
		delete res;	
	
	}
	catch (sql::SQLException &e) {
		SQLError(e);
	}
}
}