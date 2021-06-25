
#ifndef _API_H_
#define _API_H_ 1

#include "basic.h"
#include "record_manager.h"
#include "buffer_manager.h"
#include "index_manager.h"

//API接口。作为Interpreter层操作和底层Manager连接的接口
//包含程序的所有功能接口
//API只做初步的程序功能判断，具体的异常抛出仍由各底层Manager完成
class API 
{
public:
	//构造函数
	API();
	//析构函数
	~API();
	Table API_Select_Record(std::string table_name, std::vector<std::string> target_attr, std::vector<Where> where, char operation);
	int API_Delete_Record(std::string table_name, std::string target_attr , Where where);
	void API_Insert_Record(std::string table_name, Tuple& tuple);
	bool API_Create_Table(std::string table_name, Attribute attribute, int primary, Index index);
	bool API_Drop_Table(std::string table_name);
	bool API_Create_Index(std::string table_name, std::string index_name, std::string attr_name);
	bool API_Drop_Index(std::string table_name, std::string index_name);

private:
	RecordManager record;
	CatalogManager catalog;
	Table joinTable(Table &table1, Table &table2, std::string target_attr, Where where);
	Table unionTable(Table &table1, Table &table2, std::string target_attr, Where where);
};

//用于对vector的sort时排序
bool sortcmp(const Tuple &tuple1, const Tuple &tuple2);
//用于对vector对合并时对排序
bool calcmp(const Tuple &tuple1, const Tuple &tuple2);
bool isSatisfied(Tuple& tuple, int target_attr, Where where);
#endif
