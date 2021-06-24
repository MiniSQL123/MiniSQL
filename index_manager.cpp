//
//  index_manager.cc
//  index_manager
//
//  Created by Xw on 2017/6/5.
//  Copyright © 2017年 Xw. All rights reserved.
//

// TODO 可以改变量名，比如itr

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
    for (intMap::iterator itInt = indexIntMap.begin(); itInt != indexIntMap.end(); itInt++) {
        if (itInt->second) { // get Value
            itInt->second->writtenbackToDiskAll();
            delete itInt->second;
        }
    }
    for (stringMap::iterator itString = indexStringMap.begin(); itString != indexStringMap.end(); itString++) {
        if (itString->second) {
            itString->second->writtenbackToDiskAll();
            delete itString->second;
        }
    }
    for (floatMap::iterator itFloat = indexFloatMap.begin(); itFloat != indexFloatMap.end(); itFloat++) {
        if(itFloat->second) {
            itFloat->second->writtenbackToDiskAll();
            delete itFloat->second;
        }
    }
}

void IndexManager::createIndex(std::string path_file, int type)
{
    int key_size = getKeySize(type); //获取key的size
    int degree = getDegree(type); //获取需要的degree

	//根据数据类型不同，用对应的方法建立映射关系
	//并且先初始化一颗B+树
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
    if(isExistIndex(path_file, type))
    {
        switch(type)
        {
            case TYPE_INT:
                intMap::iterator itInt = indexIntMap.find(path_file);
                delete itInt->second;
                indexIntMap.erase(itInt);
            break;
            case TYPE_FLOAT:
                floatMap::iterator itFloat = indexFloatMap.find(path_file);
                delete itFloat->second;
                indexFloatMap.erase(itFloat);            
            break;
            default:
                stringMap::iterator itString = indexStringMap.find(path_file);
                delete itString->second;
                indexStringMap.erase(itString);            
        }
    }else return;
}

int IndexManager::findIndex(std::string path_file, Data data)
{
    if(isExistIndex(path_file, data.type))
    {
        switch(data.type)
        {
            case TYPE_INT:
                intMap::iterator itInt = indexIntMap.find(path_file);
                return itInt->second->searchVal(data.datai);
                break;
            case TYPE_FLOAT:
                floatMap::iterator itFloat = indexFloatMap.find(path_file);
                return itFloat->second->searchVal(data.dataf);
                break;
            default:
                stringMap::iterator itString = indexStringMap.find(path_file);
                return itString->second->searchVal(data.datas);
        }
    }else return -1;
}

void IndexManager::insertIndex(std::string path_file, Data data, int block_id)
{
    if(isExistIndex(path_file, data.type))
    {
        switch(data.type)
        {
            case TYPE_INT:
                intMap::iterator itInt = indexIntMap.find(path_file);
                itInt->second->insertKey(data.datai, block_id);
                break;
            case TYPE_FLOAT:
                floatMap::iterator itFloat = indexFloatMap.find(path_file);
                itFloat->second->insertKey(data.dataf, block_id);
                break;
            default:
                stringMap::iterator itString = indexStringMap.find(path_file);
                itString->second->insertKey(data.datas, block_id);
        }
    }else return;
}

void IndexManager::deleteIndexByKey(std::string path_file, Data data)
{
    if(isExistIndex(path_file, data.type))
    {
        switch(data.type)
        {
            case TYPE_INT:
                intMap::iterator itInt = indexIntMap.find(path_file);
                itInt->second->deleteKey(data.datai);
                break;
            case TYPE_FLOAT:
                floatMap::iterator itFloat = indexFloatMap.find(path_file);
                itFloat->second->deleteKey(data.dataf);
                break;
            default:
                stringMap::iterator itString = indexStringMap.find(path_file);
                itString->second->deleteKey(data.datas);
        }
    }else return;
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
    /*
    else if (data1.type != data2.type) {
        // cout << "ERROR: in searchRange: Wrong data type!" << endl;
        return;
    }
     */

    if(isExistIndex(path_file, data1.type))
    {
        switch(data1.type)
        {
            case TYPE_INT:
                intMap::iterator itInt = indexIntMap.find(path_file);
                itInt->second->searchRange(data1.datai, data2.datai, vals, flag);
                break;
            case TYPE_FLOAT:
                floatMap::iterator itFloat = indexFloatMap.find(path_file);
                itFloat->second->searchRange(data1.dataf, data2.dataf, vals, flag);
                break;
            default:
                stringMap::iterator itString = indexStringMap.find(path_file);
                itString->second->searchRange(data1.datas, data2.datas, vals, flag);
        }
    }else return;
}

bool IndexManager::isExistIndex(std::string path_file, int type)
{
    if (type == TYPE_INT) {
        intMap::iterator itInt = indexIntMap.find(path_file);
        if (itInt == indexIntMap.end()) return false;
        else return true;
    } else if (type == TYPE_FLOAT) {
        floatMap::iterator itFloat = indexFloatMap.find(path_file);
        if (itFloat == indexFloatMap.end()) return false;
        else return true;
    } else {
        stringMap::iterator itString = indexStringMap.find(path_file);
        if(itString == indexStringMap.end()) return false;
        else return true;
    }    
}