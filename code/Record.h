#ifndef Record_h
#define Record_h

#include "Structure.h"
#include "API.h"
using namespace std;

class RecordManager{
public:
    RecordManager(){};
    ~RecordManager();
    Table SelectRecord(Table& TableInf, vector<int> SelectAttrIndex, vector<where> SelectWhere);
    Table SelectAll(Table& TableInf);
    
    bool isSatisfied(Table& TableInf, Tuple& t, vector<where> SelectWhere); 
    bool CreateIndex(Table& TableInf, int AttrIndex);
	void Insert(Table& TableInf, Tuple& singleTuple);
	Tuple* Char2Tuple(Table& TableInf, char* stringRow);
	void InsertWithIndex(Table& TableInf, Tuple& singleTuple);
	char* Tuple2Char(Table& TableInf, Tuple& singleTuple);

	int Delete(Table& TableInf, vector<where> SelectWhere);

	bool DropTable(Table& TableInf);
	bool CreateTable(Table& TableInf);

	Table SelectProject(Table& TableInf, vector<int> SelectAttrIndex);
	Tuple String2Tuper(Table& TableInf, string stringRow);
    bool UNIQUE(Table& TableInf, where w, int loca);
};

#endif