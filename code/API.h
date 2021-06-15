#ifndef API_h
#define API_h
#include "Structure.h"
#include "Buffer.h"
extern BufferManager bf;
using namespace std;
class API{
public:
    API(){};
    ~API();
    int CreateTable(Table& inTable);
    int DropTable(Table& inTable);
    int CreateIndex(Table& inTable,int inAttr);
    int DropIndex(Table& inTable,int inAttr);
    int InsertRecord(Table& inTable,Tuple& inTuple);
    int DeleteRecord(Table& inTable,vector<where> DeleteWhere);
    Table SelectRecord(Table& inTable,vector<int> SelectAttrIndex,vector<where> SelectWhere);
    Table SelectAll(Table& inTable);
private:
    //RecordManager rm;
};

#endif