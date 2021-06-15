#ifndef Interpreter_h
#define Interpreter_h

#include "Structure.h"
#include "API.h"
#include "Catalog.h"
/********************************
 * 创建表：create table t1(tid int, tname char(8));
 * 删除表：drop table t1;
 * 创建索引：create index T1 on t1(tid);
 * 删除索引：drop index T1 on t1;
 * 选择查询：select xxx from xxx where xxx;
 * 插入记录语句：insert into t1 values(111,’xx’);
 * 删除记录语句：delete from t1 where xxx;
 * 退出系统：exit;
 * *******************************/
using namespace std;
class Interpreter{
private:
    string query;
public:
    int EXEC();//0-退出
    void GetQuery(); //获取请求并存储到query中
    void Normolize();//标准化query
    int  ProcessQuery(); //根据关键字处理请求

    void CreateTable();
    void DropTable();

    void CreateIndex();
    void DropIndex();

    void InsertRecord();
    void DeleteRecord();
    void SelectRecord();
    void SelectAll();

    void Exit();

};
class QueryException:public exception{
public:
    QueryException(string s):text(s){}
    string what(){
        return text;
    }
private:
    string text;
};
#endif