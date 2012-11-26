#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "problem.h"

using namespace std;

void csvline_populate(vector<string> &record, const string& line, char delimiter);

int main(int argc, char *argv[])
{
     vector<string> row;
	 vector<vector<string>> table;
     string line;
	 //for(int i=0; i<argc-1; i++) {cout<<argv[i]<<endl;}
     ifstream in("input.csv");
     if (in.fail())  { cout << "File not found" <<endl; return 0; }
       
     int ind=0;
	 while(getline(in, line)  && in.good() )
     {
		 table.push_back(vector<string>());
		 csvline_populate(table[ind], line, ',');
		 //table.push_back(row);
		 /*cout<< ind << ") ";
         for(int i=0, leng=row.size(); i<leng; i++)
             cout << row[i] << "\t";
         cout << endl;*/
		 ind++;
     }
     in.close();

	 
	 for (int i=0, leng=table.size(); i<leng; i++)
	 {
		 cout << i << ") ";
		 for(int j=0, leng2=table[i].size(); j<leng2; j++)
		 {
			 //cout << table[i][j] << "\t";

			 string _n_("n");
			 /*cout << _n_ <<endl;
			 cout << table[i][j].compare(_n_) << ' ';*/
			 if (table[i][j].compare(_n_)==1){
				/*int n=atoi(table[i+1][j].c_str());
				int p=atoi(table[i+1][j+1].c_str());
				CMyProblem prob(n,p);
				for (int k=0; k<n; k++)
				{
					prob.m_Set_r(k,atoi(table[i+1+k][j+2].c_str()));
					prob.m_Set_w(k,atof(table[i+1+k][j+3].c_str()));
				}*/
				stringstream ss;
				ss<<j<<'_'<<i<<".dat";
				string fn;
				ss>>fn;
				cout << fn << endl;
				//prob.m_Write(fn.c_str());
			 }
		 }
         cout << endl;
	 }
	 

     return 0;
}

void csvline_populate(vector<string> &record, const string& line, char delimiter)
{
     int linepos=0;
     int inquotes=false;
     char c;
    int linemax=line.length();
     string curstring;
     record.clear();
        
     while(line[linepos]!=0 && linepos < linemax)
     {
        
         c = line[linepos];
        
         if (!inquotes && curstring.length()==0 && c=='"')
         {
             //beginquotechar
             inquotes=true;
         }
         else if (inquotes && c=='"')
         {
             //quotechar
             if ( (linepos+1 <linemax) && (line[linepos+1]=='"') ) 
             {
                 //encountered 2 double quotes in a row (resolves to 1 double quote)
                 curstring.push_back(c);
                 linepos++;
             }
             else
             {
                 //endquotechar
                 inquotes=false; 
             }
         }
         else if (!inquotes && c==delimiter)
         {
             //end of field
             record.push_back( curstring );
             curstring="";
         }
         else if (!inquotes && (c=='\r' || c=='\n') )
         {
             record.push_back( curstring );
             return;
         }
         else
         {
             curstring.push_back(c);
         }
         linepos++;
     }
     record.push_back( curstring );
     return;
}