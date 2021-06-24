
#ifndef _INDEX_MANAGER_H_
#define _INDEX_MANAGER_H_ 1

#include <sstream>
#include <string>
#include <map>
#include "basic.h"
#include "buffer_manager.h"
#include "bplustree.h"

class IndexManager {
public:
    IndexManager(std::string table_name);
	~IndexManager();
	void createIndex(std::string path_file, int type);
    void dropIndex(std::string path_file, int type);
    int findIndex(std::string path_file, Data data);
    void insertIndex(std::string path_file, Data data , int block_id);
    void deleteIndexByKey(std::string path_file, Data data);
	void searchRange(std::string path_file, Data data1, Data data2, std::vector<int>& vals);

private:
	typedef std::map<std::string, BPlusTree<int> *> intMap;
	typedef std::map<std::string, BPlusTree<std::string> *> stringMap;
	typedef std::map<std::string, BPlusTree<float> *> floatMap;

	int static const TYPE_FLOAT = 0;
	int static const TYPE_INT = -1;

	intMap indexIntMap;
	stringMap indexStringMap;
	floatMap indexFloatMap;

	int getDegree(int type);

	int getKeySize(int type);
};

#endif
