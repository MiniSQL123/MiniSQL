#ifndef Structure_h
#define Structure_h

#define AttributeNum 32 //the max number of Attribute
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
using namespace std;
#define INTTYPE -1
#define FLOATTYPE 0
struct Attribute{
    int type[AttributeNum];          //data type
    string name[AttributeNum];       //attribute name
    bool unique[AttributeNum];       //unique
    int num;                         //the number of Attribute
};
struct Index{
    int num;            //the number of index in a Table
    int AttrIndex[10]; //the index of attribute that has a index
    string IndexName[10];
};
class Data{
public:
    int type; 
    Data(){}
    Data(const Data& otherData);
    virtual ~Data(){}
};
class Datai : public Data{
public:
    int data;
    Datai(){}
    Datai(int i);
    Datai(const Datai& otherDatai);
    ~Datai(){}
};

class Dataf : public Data{
public:
    float data;
    Dataf(){}
    Dataf(float f);
    Dataf(const Dataf& otherDataf);
    ~Dataf(){}
};

class Datac : public Data{
public:
    string data;
    Datac(){}
    Datac(string c);
    Datac(const Datac& otherDatac);
    ~Datac(){}
};

class Tuple{
public:
    vector<Data*> data; //data for each attribute
public:
    Tuple(){}
    Tuple(const Tuple& t);
    ~Tuple();

    int GetNum() const ;//return the number of the data.
    void AddData(Data* tmp) ;//add a new data to the tuple.
    void ShowTuple() const;
    Data* operator[](unsigned short i);
};
class Table{
public:
    string TableName;
    Attribute attr;    //attributes
    vector< Tuple* > t;//pointers to each tuple
    int primary_index; //the index of primary key. -1 means no primary key.
    int blockNum;      //total number of blocks occupied in data file;
    Index index;

    Table(){}
    Table(string _TableName,Attribute _attr,int _blockNum);
    Table(const Table& tmp);
    ~Table();

    Attribute GetAttribute() const;
    Index GetIndex() const;
    string GetTableName() const;
    int GetAttrNum() const;
    int GetTupleNum() const;
    void AddIndex(int _AttrIndex,string _IndexName);
    void DropIndex(string _IndexName);
    void CopyIndex(Index otherIndex);
    void SetPrimary(int _primary_index);
    void ShowTable() const;
    void AddTuple(Tuple* NewTuple);
    int DataSize() const;
};

typedef enum{eq,leq,l,geq,g,neq} WHERE;

struct where{
    int AttrIndex;
    Data* d;
    WHERE flag;
};


class TableException: public exception{
public:
    TableException(string s):text(s){}
    string what(){
        return text;
    }
private:
    string text;
};

struct Pos{
    int BlockIndex;
    int StartPos;
};
#endif
