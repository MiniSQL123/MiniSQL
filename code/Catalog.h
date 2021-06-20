#ifndef Catalog_h
#define Catalog_h
#include "Structure.h"
using namespace std;
/********************************
 * 表的名字 所占的数据块数 
 * 属性数 所有的（属性名+数据类型+属性是否唯一） 主键位置 
 * 索引数 所有（索引名+索引位置）
 * 
 * 
 * *****************************/
/*
class CataManager{
public:
    CataManager(){};
    void create_table(string s, Attribute atb, short primary, Index index);
    bool hasTable(std::string s);
    Table* getTable(std::string s);
    void create_index(std::string tname, std::string aname, std::string iname);
    void drop_table(std::string t);
    void drop_index(std::string tname, std::string iname);
    void show_table(std::string tname);
    void changeblock(std::string tname, int bn);
};
*/
class CatalogManager{
public:
    void CreateTable(Table& inTable);
    //void DropTable(Table& inTable);
    bool hasTable(string TableName);
    bool hasIndex(string IndexName);
    Table* getTable(string TableName);
    void CreateIndex(string TableName, string AttrName, string IndexName);
    void DropTable(string TableName);
    void DropIndex(string TableName,string IndexName);
private:
};

#endif