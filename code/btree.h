#ifndef btree_h
#define btree_h

#include "Buffer.h"

#define BLOCKSIZE 4096
#define MESSAGE 24
#define POINTERLENGTH 20

using namespace std;

enum nodetype{ Internal, Leaf };
extern BufferManager bf;
class Tree{
public:
	int Number;//记录当前index文件结点数目
	int keylength[3];//对应三种类型的键值：int float string
private:
	int maxchild;//b+树的最大儿子个数
	int order;   //阶数
	int type;    //0 int; 1 float; 2 char
	string name; //index文件名
public:
	Tree(string filename);
	~Tree(){};
	void initialize(Data* key, int Addr, int ktype);
	int find(Data* key);
	void insert(Data* key, int Addr);
	int* split(char* currentBlock, Data* mid, Data* key, int Addr, int leftpos, int rightpos);
	void Internal_insert(char* currentBlock, Data* mid, int leftpos, int rightpos);
	void SplitLeaf(char* block1, char*block2, char* currentBlock, Data* key, int Addr);
	void SplitInternal(char* block1, char*block2, char* currentBlock, Data* mid, int leftpos, int rightpos);
	void Delete(Data* key);
	int* Range(Data*key1, Data*key2);
};

#endif