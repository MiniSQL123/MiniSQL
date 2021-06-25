#ifndef _INTERPRETER_H_
#define	_INTERPRETER_H_ 1

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <fstream>
#include "catalog_manager.h"
#include "api.h"
#include "basic.h"
#include <regex>
void Stringsplit(const std::string& str, const std::string& split, std::vector<std::string>& res);

class Interpreter{
public:
    Interpreter();
	void getQuery();
    void ProcessQuery();
    
private:
    void Inter_Select();
    void Inter_Drop_Table();
    void Inter_Drop_Index();
    void Inter_Create_Table();
    void Inter_Create_Index();
    void Inter_Insert();
    void Inter_Delete();
    void Inter_Exit();
    void Inter_File();
    void Standardize();
    std::string query;
    std::vector<std::string> split_query;
    std::string getWord(int pos,int &end_pos);
    std::string getLower(std::string str,int pos);
    std::string getRelation(int pos,int &end_pos);
    int getType(int pos,int &end_pos);
    int getBits(int num);
    int getBits(float num);
};

template <class Type>
Type stringToNum(const std::string& str)
{
    std::istringstream iss(str);
    Type num;
    iss >> num;
    return num;
}
#endif
