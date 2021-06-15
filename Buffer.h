#ifndef Buffer_h
#define Buffer_h
#include "Structure.h"
using namespace std;
#define BLOCKSIZE 4096
#define MaxBlock 100

class Block{
public:
    string FileName;
    bool isValid;  //是否有效 delete
    bool isUse;    //是否被使用
    bool isModify; //这个block是否被修改
    bool isPin;    //是否被锁定
    int Offset;    //位于File下的第 Offset 个块
    int LastVistTime;//=clock()
    unsigned char value[BLOCKSIZE+1];//最后一个存储'\0',空白字符'#'
    Block(){
    }
};
/***********************************
 * 创建表 不需要操作
 * 删除表 buffer中的信息Invalid/清空，remove文件
 * 创建Index 不需要操作
 * 删除Index buffer中的信息Invalid/清空，remove文件
 * 插入 返回插入的block的Index，pos
 * 选择 返回特定Table文件下的所有块
 * 删除 返回特定Table文件下的所有块
 * **********************************/
class BufferManager{
public:
    Block BufferBlock [MaxBlock];
    //
    void DropTable(Table& TableInf);
    void DropIndex(Index& IndexInf);
    Pos getInsertPos(Table& TableInf);
    int getBlockIndex( string FileName, int Offset);//获取指定block在buffer中的编号
    //
    void setInvalid(int BlockIndex);
    void RemoveFile(string FileName);
    void ClearBlock(int BlockIndex);
    void ModifyBlock(int BlockIndex);//标记block被修改
    void UseBlock(int BlockIndex);//更新block的LastVisitTime
    
    int isInBuffer(string FileName,int Offset);//判断指定block是否在buffer
    
    
    void WriteBlockToFile(int BlockIndex); //单独块写回
    void WriteBufferToFile();//所有块写回
    
    void ReadToBlock( string FileName, int Offset, int BlockIndex,bool isPin);
    
    int getEmptyBlockIndex();//空的block
    int getEmptyBlockIndexExcept(string FileName);//寻找内存中空的block，并且不能替换掉给定的文件
    
    int AddBlocktoFile(Table& TableInf);//**文件后插入新的block，返回新的block在内存中的编号
    int AddBlocktoFile(Index& IndexInf);
private:
};

#endif