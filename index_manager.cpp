

#include "index_manager.h"
#include "const.h"
#include "basic.h"
#include "buffer_manager.h"
#include "bplustree.h"
#include "catalog_manager.h"
#include <string>
#include <vector>
#include <map>

IndexManager::IndexManager(std:: string table_name)
{
    CatalogManager catalog;
    Attribute attr = catalog.getAttribute(table_name);
    
    for (int i = 0; i < attr.num; i++)
        if (attr.has_index[i])
            createIndex(attr.name[i] + "_" + table_name, attr.type[i]);
}

IndexManager::~IndexManager()
{
    for (intMap::iterator itrInt = indexIntMap.begin(); itrInt != indexIntMap.end(); itrInt++) {
        if (itrInt->second) { // get Value
            itrInt->second->writtenbackToDiskAll();
            delete itrInt->second;
        }
    }
    for (stringMap::iterator itrString = indexStringMap.begin(); itrString != indexStringMap.end(); itrString++) {
        if (itrString->second) {
            itrString->second->writtenbackToDiskAll();
            delete itrString->second;
        }
    }
    for (floatMap::iterator itrFloat = indexFloatMap.begin(); itrFloat != indexFloatMap.end(); itrFloat++) {
        if(itrFloat->second) {
            itrFloat->second->writtenbackToDiskAll();
            delete itrFloat->second;
        }
    }
}

void IndexManager::createIndex(std::string path_file, int type)
{
    int key_size = getKeySize(type); 
    int degree = getDegree(type);

    if (type == TYPE_INT) { // TODO 不会有内存泄漏吗？
        BPlusTree<int> *tree = new BPlusTree<int>(path_file, key_size, degree);
        indexIntMap.insert(intMap::value_type(path_file, tree));
    } else if(type == TYPE_FLOAT) {
        BPlusTree<float> *tree = new BPlusTree<float>(path_file, key_size, degree);
        indexFloatMap.insert(floatMap::value_type(path_file, tree));
    } else {
        BPlusTree<std::string> *tree = new BPlusTree<std::string>(path_file, key_size, degree);
        indexStringMap.insert(stringMap::value_type(path_file, tree));
    }

	return;
}

void IndexManager::dropIndex(std::string path_file, int type)
{
    if (type == TYPE_INT) {
        intMap::iterator itrInt = indexIntMap.find(path_file);
        if (itrInt == indexIntMap.end()) return;
        else {
            delete itrInt->second;
            indexIntMap.erase(itrInt);
        }
    } else if (type == TYPE_FLOAT) { 
        floatMap::iterator itrFloat = indexFloatMap.find(path_file);
        if (itrFloat == indexFloatMap.end()) return;
        else {
            delete itrFloat->second;
            indexFloatMap.erase(itrFloat);
        }
    } else {
        stringMap::iterator itrString = indexStringMap.find(path_file);
        if (itrString == indexStringMap.end()) return;
        else {
            delete itrString->second;
            indexStringMap.erase(itrString);
        }
    }

	return;    
}

int IndexManager::findIndex(std::string path_file, Data data)
{
    if (data.type == TYPE_INT) {
        intMap::iterator itrInt = indexIntMap.find(path_file);
        if (itrInt == indexIntMap.end()) return -1;
        else return itrInt->second->findValue(data.datai);
    } else if(data.type == TYPE_FLOAT) {
        floatMap::iterator itrFloat = indexFloatMap.find(path_file);
        if (itrFloat == indexFloatMap.end()) return -1;
        else return itrFloat->second->findValue(data.dataf);
    } else {
        stringMap::iterator itrString = indexStringMap.find(path_file);
        if (itrString == indexStringMap.end()) return -1;
        else return itrString->second->findValue(data.datas);
    }
}

void IndexManager::insertIndex(std::string path_file, Data data, int block_id)
{
    if (data.type == TYPE_INT) {
        intMap::iterator itrInt = indexIntMap.find(path_file);
        if (itrInt == indexIntMap.end()) return;
        else itrInt->second->insertKey(data.datai, block_id);
    } else if (data.type == TYPE_FLOAT) {
        floatMap::iterator itrFloat = indexFloatMap.find(path_file);
        if (itrFloat == indexFloatMap.end()) return;
        else itrFloat->second->insertKey(data.dataf, block_id);
    } else {
        stringMap::iterator itrString = indexStringMap.find(path_file);
        if (itrString == indexStringMap.end()) return;
        else itrString->second->insertKey(data.datas, block_id);
    }

    return;
}

void IndexManager::deleteIndexByKey(std::string path_file, Data data)
{
    if (data.type == TYPE_INT) {
        intMap::iterator itrInt = indexIntMap.find(path_file);
        if (itrInt == indexIntMap.end()) return;
        else itrInt->second->deleteKey(data.datai);
    } else if (data.type == TYPE_FLOAT) {
        floatMap::iterator itrFloat = indexFloatMap.find(path_file);
        if (itrFloat == indexFloatMap.end()) return;
        else itrFloat->second->deleteKey(data.dataf);
    } else {
        stringMap::iterator itrString = indexStringMap.find(path_file);
        if(itrString == indexStringMap.end()) return;
        else itrString->second->deleteKey(data.datas);
    }
}

int IndexManager::getDegree(int type)
{
    int degree = (PAGESIZE - sizeof(int)) / (getKeySize(type) + sizeof(int));
    if (degree % 2 == 0) degree -= 1; // TODO degree为啥要-1啊。。
    return degree;
}

int IndexManager::getKeySize(int type)
{
    if (type == TYPE_FLOAT) return sizeof(float);
    else if (type == TYPE_INT) return sizeof(int);
    else if (type > 0) return type;
    else return -100;
}

void IndexManager::searchRange(std::string path_file, Data data1, Data data2, std::vector<int>& vals)
{
    int flag = 0;
    //检测数据类型是否匹配
    if (data1.type == -2) {
        flag = 1;
    } else if (data2.type == -2) {
        flag = 2;
    }

    if (data1.type == TYPE_INT) {
        intMap::iterator itrInt = indexIntMap.find(path_file);
        if (itrInt == indexIntMap.end()) return;
        else itrInt->second->searchRange(data1.datai, data2.datai, vals, flag);
    } else if (data1.type == TYPE_FLOAT) {
        floatMap::iterator itrFloat = indexFloatMap.find(path_file);
        if (itrFloat == indexFloatMap.end()) return;
        else itrFloat->second->searchRange(data1.dataf, data2.dataf, vals, flag);
    } else {
        stringMap::iterator itrString = indexStringMap.find(path_file);
        if(itrString == indexStringMap.end()) return;
        else itrString->second->searchRange(data1.datas, data2.datas, vals, flag);
    }
}

