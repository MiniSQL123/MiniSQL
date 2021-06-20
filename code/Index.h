#ifndef Index_h
#define Index_h
#include "Structure.h"
using namespace std;
/*
class IndexManager{
public:
	IndexManager(){};
	void Establish(string FileName);
	void Insert(string FileName, Data* key, int Addr);
	void Delete(string FileName, Data* key);
	int Find(string FileName, Data* key);
	void Drop(string FileName);
	int*Range(string FileName, Data*key1, Data*key2);
	~IndexManager(){};
};
*/
class IndexManager{
public:
    IndexManager();
	~IndexManager();
	void CreateIndex(Table& TableInf,Index& IndexInf);
	void 
};
#endif