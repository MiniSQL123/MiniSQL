#include "Structure.h"
using namespace std;

Data::Data(const Data& otherData){
    type=otherData.type;
}

Datai::Datai(int i):data(i){
    type=INTTYPE;
}
Datai::Datai(const Datai& otherDatai){
    type=otherDatai.type;
    data=otherDatai.data;
}

Dataf::Dataf(float f):data(f){
    type = FLOATTYPE;
}
Dataf::Dataf(const Dataf& otherDataf){
    type=otherDataf.type;
    data=otherDataf.data;
}

Datac::Datac(string c):data(c){
    type = c.length();
}
Datac::Datac(const Datac& otherDatac){
    type=otherDatac.type;
    data=otherDatac.data;
}

Tuple::Tuple(const Tuple& t){
    Data* new_data;
    for(int i=0;i<t.GetNum();i++){
        if(t.data[i]->type==INTTYPE){
            new_data = new Datai( ((Datai *)t.data[i])->data );
            this->AddData(new_data);
        }
        else if(t.data[i]->type==FLOATTYPE){
            new_data = new Dataf( ((Dataf *)t.data[i])->data );
            this->AddData(new_data);
        }
        else if(t.data[i]->type>0){
            new_data = new Datac( ((Datac *)t.data[i])->data );
            this->AddData(new_data);
        }
    }
}
Tuple::~Tuple()
{
    for(int i=0;i<data.size();i++){
        if(data[i]!=NULL){
            if(data[i]->type==INTTYPE)
                delete (Datai*) data[i];
            else if(data[i]->type==FLOATTYPE)
                delete (Dataf*) data[i];
            else if(data[i]->type>0)
                delete (Datac*) data[i];
        } 
    }
}
int Tuple::GetNum() const{
    return (int) data.size();
}
void Tuple::AddData(Data* tmp){
    data.push_back(tmp);
}
void Tuple::ShowTuple() const{
    for(int i=0;i<data.size();i++){
        if(data[i]==NULL)
            cout<<"NULL"<<"\t";
        else if(data[i]->type==INTTYPE)
            cout << ((Datai*)data[i])->data << "\t";
        else if(data[i]->type==FLOATTYPE)
            cout << ((Dataf*)data[i])->data << "\t";
        else cout << ((Datac*)data[i])->data << "\t";
    }
    cout<<endl;
}
Data* Tuple::operator[](unsigned short i){
    if(i>=data.size())
        throw out_of_range("Out of the tuple range");
    return data[i];
}

Table::Table(string _TableName,Attribute _attr,int _blockNum)
:TableName(_TableName),attr(_attr),blockNum(_blockNum)
{ 
    primary_index=-1;
    //for(int i=0;i<AttributeNum;i++)
    //    attr.unique[i]=0;
    index.num=0;
}
Table::Table(const Table& tmp){
    this->TableName=tmp.TableName;
    this->attr     =tmp.attr; 
    for(int i=0;i<tmp.GetTupleNum();i++){
        Tuple* new_tuple=new Tuple( *(tmp.t[i]) );
        this->AddTuple(new_tuple);
    }
    this->primary_index=tmp.primary_index;
    this->blockNum=tmp.blockNum;
    this->index=tmp.index;
}
Table::~Table()
{
    for(int i=0;i<GetTupleNum();i++)
    if(t[i]!=NULL)
        delete t[i];
}
Attribute Table::GetAttribute() const{
    return attr;
}
Index Table::GetIndex() const {
    return index;
}
string Table::GetTableName() const{
    return TableName;
}
int Table::GetAttrNum() const{
    return attr.num;
}
int Table::GetTupleNum() const{
    return (int) t.size();
}
void Table::AddIndex(int _AttrIndex,string _IndexName){
    
    for(int j=0;j<index.num;j++){
        if(index.AttrIndex[j]==_AttrIndex)
            throw TableException("This attribute already has index!");
        if(index.IndexName[j]==_IndexName)
            throw TableException("This IndexName already exists!");
    }
    int i=index.num;
    index.AttrIndex[i]=_AttrIndex;
    index.IndexName[i]=_IndexName;
    index.num++;
}
void Table::DropIndex(string _IndexName)
{
    int is=-1;
    for(int j=0;j<index.num;j++)
        if(index.IndexName[j]==_IndexName){
            is=j;
            break;
        }
    if(is==-1)
        throw TableException("No index named " + _IndexName + " on " + TableName + "!");
    index.AttrIndex[is]=index.AttrIndex[index.num-1];
    index.IndexName[is]=index.IndexName[index.num-1];
    index.num--;    
}
void Table::CopyIndex(Index otherIndex)
{
    index=otherIndex;
}
void Table::SetPrimary(int _primary_index)
{
    if(primary_index!=-1)
        throw("The table "+TableName+" already has primary key "+attr.name[primary_index]+" !");
    primary_index=_primary_index;
}
void Table::ShowTable() const
{
    cout<<"TableName: "<<TableName<<endl;
    for(int i=0;i<attr.num;i++){
        cout.width(10);
        cout.setf(ios::left);
        cout.fill(' ');
        cout<<attr.name;
    }
    cout<<endl;
    for(int i=0;i<GetTupleNum();i++){
        for(int j=0;j<t[i]->GetNum();j++){
            cout.width(10);
            cout.setf(ios::left);
            cout.fill(' ');
            cout<<t[i]->data[j];
        }
        cout<<endl;
    }
    cout<<endl;
}
void Table::AddTuple(Tuple* NewTuple) {
    if(NewTuple->GetNum()!=attr.num)
        throw TableException("Illegal Tuper Inserted: unequal column size!");
    for(int i=0;i<GetAttrNum();i++){
        if(attr.type[i]==INTTYPE   && NewTuple->data[i]->type!=INTTYPE)
            throw TableException("Illegal Attribute type!");
        if(attr.type[i]==FLOATTYPE && NewTuple->data[i]->type!=FLOATTYPE)
            throw TableException("Illegal Attribute type!");
        if(attr.type[i]>0 && NewTuple->data[i]->type<=0)
            throw TableException("Illegal Attribute type!");
    }
    t.push_back(NewTuple);
}   
int Table::DataSize() const{
    int res=0;
    for(int i=0;i<attr.num;i++){
        if(attr.type[i]==INTTYPE) res+=sizeof(int);
        else if(attr.type[i]==FLOATTYPE) res+=sizeof(float);
        else res+=attr.type[i];
    }
    return res;
}




