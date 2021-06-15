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

struct Attribute{
    int type[AttributeNum];          //data type
    string name[AttributeNum];       //attribute name
    bool unique[AttributeNum];       //unique
    int num;                         //the number of Attribute
};

class Data{
public:
    int type; //-3-null -2-int -1-float 0~255-char.
    Data(){}
    Data(const Data& otherData){
        type=otherData.type;
    }
    virtual ~Data(){};
};
class Datai : public Data{
public:
    int data;
    Datai(int i):data(i){
        type = -2;
    };
    Datai(const Datai& otherDatai){
        type=otherDatai.type;
        data=otherDatai.type;
    }
    ~Datai(){}
};

class Dataf : public Data{
public:
    float data;
    Dataf(float f):data(f){
        type = -1;
    };
    Dataf(const Dataf& otherDataf){
        type=otherDataf.type;
        data=otherDataf.data;
    }
    ~Dataf(){}
    
};

class Datac : public Data{
public:
    string data;
    Datac(string c):data(c){
        type = c.length();
    };
    Datac(const Datac& otherDatac){
        type=otherDatac.type;
        data=otherDatac.data;
    }
    ~Datac(){}
};

class Tuple{
public:
    vector<Data*> data; //data for each attribute
public:
    Tuple(){};
    Tuple(const Tuple& t);
    ~Tuple();
    /*
    int length() const{
        return (int)data.size();
    }//return the number of the data.
    
    void addData(Data* d){
        data.push_back(d);
    }//add a new data to the tuper.
    
    Data* operator[](unsigned short i);
    //return the pointer to a specified data item.
    
    void disptuple();
    //display the data in the tuper.
    */
};
class Table{
public:
    string TableName;
    Attribute attr;    //attributes
    vector< Tuple* > t;//pointers to each tuple
    int primary_index; //the index of primary key. -1 means no primary key.
    int blockNum;      //total number of blocks occupied in data file;
    Index index;
};

typedef enum{
    eq,leq,l,geq,g,neq} WHERE;

struct where{
    int AttrIndex;
    Data* d;
    WHERE flag;
};

struct Index{
    int num;
    short location[10];
    string indexname[10];
};

struct Pos{
    int BlockIndex;
    int StartPos;
};
#endif
