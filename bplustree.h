#ifndef _BPLUSTREE_H_
#define _BPLUSTREE_H_ 1

#include <vector>
#include <string>
#include <string.h>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "basic.h"
#include "const.h"
#include "exception.h"
#include "buffer_manager.h"
#include "template_function.h"
using namespace std;
extern BufferManager BM;
extern clock_t time_bpio;

//模板类Node，直接适配int，float, string
template <typename T>
class Node {
public:
    unsigned int key_num;
    Node* parent;
    std::vector <T> keys;
    std::vector <Node*> children;// 指向子结点的指针容器
    std::vector <int> vals;
    Node* next_leaf;// 指向下一个叶结点的指针
    bool is_leaf;
    int degree;// degree of tree

public:
    Node(int in_degree, bool Leaf = false);
    ~Node(){  };
    bool isRoot();
    bool findKey(T key, unsigned int &index);
    //返回Node指针；将一个结点分裂成两个结点，新结点为本结点的下一个结点
    //同时通过key引用返回上层需要更新的Key
    Node* splitNode(T &key);
    // 返回添加的position
    unsigned int addKey(T &key);
    //返回添加的position
    //val是block_id，由外部提供
    unsigned int addKey(T &key, int val);
	//index是指key值对应的index
    bool deleteKeyByIndex(unsigned int index);
    //返回指定范围的values
        // 从Index开始，返回<Key的values
    bool findRangeIndexKey(unsigned int index, T& key, std::vector<int>& vals);
        // 从Index开始，返回Index之后的所有values
    void findRangeAfterIndex(unsigned int index, std::vector<int>& vals);
};

template <typename T>
class BPlusTree {
private:
    typedef Node<T>* Tree;
    struct nodePos { // 用于定位某个key
        Tree pNode; //包含对应key的结点指针
        unsigned int index; //key在结点中的index
        bool is_found; //是否找到该key
    };

private:
    std::string file_name; // 文件名，用于与buffer交互；创建索引文件等。
    Tree root;
    Tree first_leaf;// 叶结点的左部头指针
    unsigned int key_num;
    unsigned int level;
    unsigned int node_num;
    int key_size;
    int degree;

public:
    BPlusTree(std::string m_name, int key_size, int degree);
    ~BPlusTree();

    int findValue(T &key);
	// 返回：是否插入成功
    bool insertKey(T &key, int val);
	// 返回：是否删除成功
    bool deleteKey(T &key);
	// 输入树的根结点，递归地释放内存
    void dropTree(Tree node);
    //输入：key1，key2，返回vals的容器
    //功能：返回范围搜索结果，将value放入vals容器中
    void searchRange(T &key1, T &key2, std::vector<int>& vals, int flag);

	//从磁盘读取所有数据
    void readFromDiskAll();
	//将新数据写入磁盘
    void writtenbackToDiskAll();
	//在磁盘中读取某一块的数据
    void readFromDisk(char *p, char* end);

private:
    void initTree();// 分配内存
    bool insertMaintain(Tree pNode);// insert某key后结点满情况
    bool deleteMaintain(Tree pNode);// delete某key后可能出现结点空情况
    void findLeaf(Tree pNode, T key, nodePos &snp);
    //获取文件大小
    void getFile(std::string file_path);// 创建对应文件
    int getBlockNum(std::string table_name);
};

template <class T>
Node<T>::Node(int in_degree, bool Leaf):
	key_num(0),
	parent(NULL),
	next_leaf(NULL),
	is_leaf(Leaf),
	degree(in_degree)
{   // 最大能存到degree+1个key, vals; 最大能存到degree+2个pointer
    // degree都记的是基数，实际能存x个key，其中x都是偶数
    for (unsigned i = 0; i < degree+1; i++) { 
        // 这里的i < degree+1的+1是预留给“爆了”的结点进行调整的
        children.push_back(NULL);
        keys.push_back(T());
        vals.push_back(int());
    }
    children.push_back(NULL);
}

template <class T>
bool Node<T>::isRoot()
{
    if(!parent)return false;
    else return true;
}

//如不存在，则返回最接近的index
//函数返回bool值说明是否找到
template <class T>
bool Node<T>::findKey(T key, unsigned int &index)
{
    if (key_num == 0) { //结点内key数量为0
        index = 0;
        return false;
    } else {
        if (keys[key_num-1] < key) { // key是否超出最大值
            index = key_num;
            return false;
        } else if (keys[0] > key) { // key是否小于最小值
            index = 0;
            return false;
        } else {
		// 可能存在，二分搜索
            unsigned int low = 0, high = key_num-1, mid = 0;

            while (high > low+1) {
                mid = (high + low) / 2;
                if (keys[mid] == key) {
                    index = mid;
                    return true;
                } else if (keys[mid] < key) {
                    low = mid;
                } else if (keys[mid] > key) {
                    high = mid;
                }
            }
            // 没找到情况必然是low+1 == high
            if (keys[low] >= key) {
                index = low;
                return (keys[low] == key);// 如果是==的话，后续函数应该到children[low+1]里去找
            } else if (keys[high] >= key) {
                index = high;
                return (keys[high] == key);
            } else if(keys[high] < key) {
                index = ++ high;
                return false;
            }
        }//二分搜索结束
    }
    return false;
}

// 新结点为本结点的下一个结点，通过key引用返回去到上层的key值
template <class T>
Node<T>* Node<T>::splitNode(T &key)
{
    unsigned int min_node_num = (degree - 1) / 2; // 相当于取下整
    Node* new_node = new Node(degree, this->is_leaf);
    // 叶子结点被调用了此函数时，其实已经“爆了”
    if (is_leaf) {
        key = keys[min_node_num + 1];
        // 新结点持有min_node_num个Key或者val
        // 原结点保留min_node_num+1个Key
        for (unsigned int i = min_node_num + 1; i < degree; i++) {
            new_node->keys[i-min_node_num-1] = keys[i];
            keys[i] = T();
            new_node->vals[i-min_node_num-1] = vals[i];
            vals[i] = int();
        }
		// 新结点为本结点的下一个结点
        new_node->next_leaf = this->next_leaf;
        this->next_leaf = new_node;
        new_node->parent = this->parent;

        new_node->key_num = min_node_num;
        this->key_num = min_node_num + 1;
    } else if (!is_leaf) {  //非叶结点情况
        key = keys[min_node_num]; // keys[min_node_num]是已经溢出的key
        // 先拷贝“指针”到新结点，“指针”比“key”多1个
        for (unsigned int i = min_node_num + 1; i < degree+1; i++) {
            new_node->children[i-min_node_num-1] = this->children[i];
            new_node->children[i-min_node_num-1]->parent = new_node;
            this->children[i] = NULL;
        }
		// 拷贝key值至新结点
        for (unsigned int i = min_node_num + 1; i < degree; i++) {
            new_node->keys[i-min_node_num-1] = this->keys[i];
            this->keys[i] = T();
        }
		// 调整结点相互位置关系
        this->keys[min_node_num] = T();
        new_node->parent = this->parent;

        new_node->key_num = min_node_num;
        this->key_num = min_node_num;
    }
    return new_node;
}

// 在枝干结点中添加key值，并返回添加的位置
template <class T>
unsigned int Node<T>::addKey(T &key)
{
    if (key_num == 0) {
        keys[0] = key;
        key_num ++;
        return 0;
    } else {
        unsigned int index = 0;

        if(!findKey(key, index)){ // 不存在重复key
			// 挪位置
            for (unsigned int i = key_num; i > index; i--)
                keys[i] = keys[i-1];
            keys[index] = key;

			// 子结点指针
            for (unsigned int i = key_num + 1; i > index+1; i--)
                children[i] = children[i-1];
            children[index+1] = NULL;
            key_num++;

            return index;            
        }
    }
    return 0;
}

// 在叶结点中添加key值，并返回添加的位置
template <class T>
unsigned int Node<T>::addKey(T &key, int val)
{	
    if (!is_leaf) return -1;

    if (key_num == 0) {
        keys[0] = key;
        vals[0] = val;
        key_num ++;
        return 0;
    } else { //正常插入
        unsigned int index = 0;
        if(!findKey(key, index)){
			//逐个调整key值
            for (unsigned int i = key_num; i > index; i--) {
                keys[i] = keys[i-1];
                vals[i] = vals[i-1];
            }
            keys[index] = key;
            vals[index] = val;
            key_num++;
            return index;
        }
    }
    return 0;
}

//在结点中删除index对应的key值。返回值：是否删除成功。
template <class T>
bool Node<T>::deleteKeyByIndex(unsigned int index)
{
    if(index > key_num) return false;
    else {
        if (is_leaf) { // leaf
            for (unsigned int i = index; i < key_num-1; i++) {
                keys[i] = keys[i+1];
                vals[i] = vals[i+1];
            }
            keys[key_num-1] = T();
            vals[key_num-1] = int();
        } else { // internal
			//调整key值
            for(unsigned int i = index; i < key_num-1; i++)
                keys[i] = keys[i+1];

			//调整子结点指针
            for(unsigned int i = index+1;i < key_num;i ++)
                children[i] = children[i+1];

            keys[key_num-1] = T();
            children[key_num] = NULL;
        }

        key_num--;
        return true;
    }
    return false;
}

// 起始index,终止key，返回结果vector容器
template <class T>
bool Node<T>::findRangeIndexKey(unsigned int index, T& key, std::vector<int>& vals_output)
{
	unsigned int i;
	for (i = index; i < key_num && keys[i] <= key; i++)
		vals_output.push_back(vals[i]);

	if (keys[i] >= key)
		return true;
	else
		return false;
}

template <class T>
void Node<T>::findRangeAfterIndex(unsigned int index, std::vector<int>& vals_output)
{
	unsigned int i;
	for (i = index; i < key_num; i++)
		vals_output.push_back(vals[i]);
}




template <class T>
BPlusTree<T>::BPlusTree(std::string in_name, int keysize, int in_degree):
	file_name(in_name),
	key_num(0),
	level(0),
	node_num(0),
	root(NULL),
	first_leaf(NULL),
	key_size(keysize),
	degree(in_degree)
{
    initTree();
    clock_t start = clock();
    readFromDiskAll();
    clock_t end = clock();
    time_bpio += (end-start);
}

template <class T>
BPlusTree<T>:: ~BPlusTree()
{
    dropTree(root);
    key_num = 0;
    root = NULL;
    level = 0;
}

template <class T>
void BPlusTree<T>::initTree()
{
    root = new Node<T>(degree, true);
    key_num = 0;
    level = 1;
    node_num = 1;
    first_leaf = root;
}

//用于查找某key值所处的叶结点位置。可能递归
template <class T>
void BPlusTree<T>::findLeaf(Tree pNode, T key, nodePos &np)
{
    unsigned int index = 0;
	//在对应结点内查找key值
    if (pNode->findKey(key, index)) {
		//若此结点是叶结点，则查找成功
        if (pNode->is_leaf) {
            np.pNode = pNode;
            np.index = index;
            np.is_found = true;
        } else {
			//此结点不是子结点，查找它的下一层
            // 注意是index+1，配合node->findKey，返回为true的时候就应该用index+1继续查
            pNode = pNode->children[index+1];
            while (!pNode->is_leaf) {
                pNode = pNode->children[0];
            }
			// 因为已找到key值，所以其最底层叶结点index[0]即为该key
            np.pNode = pNode;
            np.index = 0;
            np.is_found = true;
        }

    } else { //本结点内未找到该key
        if (pNode->is_leaf) {
			//若此结点已经是叶结点则查找失败
            np.pNode = pNode;
            np.index = index;
            np.is_found = false;
        } else {
			//递归寻找下一层
            findLeaf(pNode->children[index], key, np);// 在此需要配合node->findKey所返回的最接近的index
        }
    }
	return;
}

//返回是否插入成功
template <class T>
bool BPlusTree<T>::insertKey(T &key, int val)
{
    nodePos np;
    if (!root)
		initTree();
	//查找插入值是否存在
    findLeaf(root, key, np);
    if (np.is_found) { //已存在
        return false;
    } else { //不存在，可以插入
        np.pNode->addKey(key, val);
		//插入后结点满，需要进行调整
        if (np.pNode->key_num == degree) {
            insertMaintain(np.pNode);
        }
        key_num++;
        return true;
    }
    return false;
}

//用于插入某key后若结点满，对整棵树进行调整
template <class T>
bool BPlusTree<T>::insertMaintain(Tree pNode)
{
    T key;
    Tree new_node = pNode->splitNode(key);
    node_num++;

	//当前结点为根结点情况
    if (pNode->isRoot()) {
        Tree root = new Node<T>(degree, false);
        if(root){
            level ++;
            node_num ++;
            this->root = root;
            pNode->parent = root;
            new_node->parent = root;
            root->addKey(key);
            root->children[0] = pNode;
            root->children[1] = new_node;
            return true;
        }
    } else { //当前结点非根结点
        Tree parent = pNode->parent;
        unsigned int index = parent->addKey(key);

        parent->children[index+1] = new_node;
        new_node->parent = parent;
		//递归进行调整
        if(parent->key_num == degree)// 等于degree的时候是已经爆了而不是已经满了
            // TODO 我怀疑是最初getDegree定义的时候定义错了。
            // 因为getDegree算出来的，对应书上的其实是N-1而不是N
            return insertMaintain(parent);
        return true;
    }

    return false;
}

template <class T>
int BPlusTree<T>::findValue(T& key)
{
    if (!root)
        return -1;
    nodePos np;
    findLeaf(root, key, np);

    if (!np.is_found) return -1;
    else return np.pNode->vals[np.index];
}

//返回是否删除成功
template <class T>
bool BPlusTree<T>::deleteKey(T &key)
{
    nodePos np;
	//根结点不存在
    if (!root) return false;
    else { //正常进行操作
		//查找位置
        findLeaf(root, key, np);
        if (!np.is_found) return false;
        else { //正常找到进行删除
            if (np.pNode->isRoot()) { //当前为根结点
                np.pNode->deleteKeyByIndex(np.index);
                key_num--;
                return deleteMaintain(np.pNode);
            } else {
                if (np.index == 0 && first_leaf != np.pNode) {
                    // TODO 为什么这种情况要独立考虑
                    // np.index == 0意味着会影响到上层的KEY
					//key存在于枝干结点上
					//到上一层去更新枝干层
                    unsigned int index = 0;

                    Tree now_parent = np.pNode->parent;
                    bool if_found_in_ancestors = now_parent->findKey(key, index);
                    while (!if_found_in_ancestors) {
                        if(now_parent->parent)
                            now_parent = now_parent->parent;
                        else
                            break;
                        if_found_in_ancestors = now_parent->findKey(key,index);
                    }

                    now_parent -> keys[index] = np.pNode->keys[1];

                    np.pNode->deleteKeyByIndex(np.index);
                    key_num--;
                    return deleteMaintain(np.pNode);

                } else {
                    np.pNode->deleteKeyByIndex(np.index);
                    key_num--;
                    return deleteMaintain(np.pNode);
                }
            }
        }
    }

    return false;
}

// TODO 其实没懂为什么leaf node是min_key_num而internal node是min_key_num-1
//用于删除某key后可能出现结点空情况，对整棵树进行调整
template <class T>
bool BPlusTree<T>::deleteMaintain(Tree pNode)
{
    unsigned int min_key_num = (degree - 1) / 2;
	//三种不需要调整的情况
    // TODO 看不懂
    if (((pNode->is_leaf) && (pNode->key_num >= min_key_num)) ||
	    ((degree != 3) && (!pNode->is_leaf) && (pNode->key_num >= min_key_num - 1)) ||
		((degree == 3) && (!pNode->is_leaf) && (pNode->key_num < 0))) {
        return  true;
    }
    if (pNode->isRoot()) { //当前结点为根结点
        if (pNode->key_num > 0) //不需要调整
            return true;
        else { 
            if (root->is_leaf) { // 根没有孩子
                dropTree(pNode);
                root = NULL;
                first_leaf = NULL;
                level--;
            }
            else { // 根有一个孩子
                root = pNode -> children[0];
                root -> parent = NULL;
                delete pNode;
                level--;
                node_num--;
            }
        }
    } else { //非根节点情况
        Tree parent = pNode->parent, brother = NULL;
        if (pNode->is_leaf) { //当前为叶节点
            unsigned int index = 0;
            parent->findKey(pNode->keys[0], index);

			//选择左兄弟
            if ((parent->children[0] != pNode) && (index + 1 == parent->key_num)) {
                // pNode不是第一个孩子，且pNode是parent的最后一个孩子
                // 所以pNode只能选左兄弟
                brother = parent->children[index];
                if(brother->key_num > min_key_num) {
                    // brother有多余的key
                    for (unsigned int i = pNode->key_num; i > 0; i--) {
                        pNode->keys[i] = pNode->keys[i-1];
                        pNode->vals[i] = pNode->vals[i-1];
                    }
                    // 把左兄弟最大的key移为了pNode的第一个key
                    pNode->keys[0] = brother->keys[brother->key_num-1];
                    pNode->vals[0] = brother->vals[brother->key_num-1];
                    brother->deleteKeyByIndex(brother->key_num-1);

                    pNode->key_num++;
                    parent->keys[index] = pNode->keys[0];
                    return true;

                } else {
                    // brother刚好是min_key_num，故把pNode并入左兄弟
                    parent->deleteKeyByIndex(index);
                    // 把pNode的所有Keys和vals都并入其左兄弟
                    for (int i = 0; i < pNode->key_num; i++) {
                        brother->keys[i+brother->key_num] = pNode->keys[i];
                        brother->vals[i+brother->key_num] = pNode->vals[i];
                    }
                    brother->key_num += pNode->key_num;
                    brother->next_leaf = pNode->next_leaf;

                    delete pNode;
                    node_num--;
                    // 因为删除了parent的一个index，故需要maintain parent
                    return deleteMaintain(parent);
                }

            } else {
                // 选右兄弟
                if(parent->children[0] == pNode)
                    brother = parent->children[1];
                else
                    brother = parent->children[index+2];
                if(brother->key_num > min_key_num) {
                    // 右兄弟有多余的key，把右兄弟多的key分给pNode
                    pNode->keys[pNode->key_num] = brother->keys[0];
                    pNode->vals[pNode->key_num] = brother->vals[0];
                    pNode->key_num++;
                    brother->deleteKeyByIndex(0);
                    // 因为brother最小的key变了，故更新parent处的key
                    if(parent->children[0] == pNode)
                        parent->keys[0] = brother->keys[0];
                    else
                        parent->keys[index+1] = brother->keys[0];
                    return true;

                } else {
                    // 右兄弟刚好有min_key_num，故把右兄弟并入pNode
                    for (int i = 0; i < brother->key_num; i++) {
                        pNode->keys[pNode->key_num+i] = brother->keys[i];
                        pNode->vals[pNode->key_num+i] = brother->vals[i];
                    }
                    // 因为右兄弟将要不存在了，故更新Parent的key
                    if (pNode == parent->children[0])
                        parent->deleteKeyByIndex(0);
                    else
                        parent->deleteKeyByIndex(index+1);
                    pNode->key_num += brother->key_num;
                    pNode->next_leaf = brother->next_leaf;
                    delete brother;
                    node_num--;
                    // 删除了parent的一个key，故继续maintain parent
                    return deleteMaintain(parent);
                }
            }

        } else { // pNode is a internal node
            unsigned int index = 0;
            parent->findKey(pNode->children[0]->keys[0], index);
            // 选择pNode的左兄弟
            if ((parent->children[0] != pNode) && (index + 1 == parent->key_num)) {
                // pNode不是first_child，且pNode是last_child
                // 所以pNode只能找左兄弟
                brother = parent->children[index];
                // 左兄弟有“多余”的key
                if (brother->key_num > min_key_num - 1) {
                    // pNode原有的key向右移，腾出位置
                    pNode->children[pNode->key_num+1] = pNode->children[pNode->key_num];
                    for (unsigned int i = pNode->key_num; i > 0; i--) {
                        pNode->children[i] = pNode->children[i-1];
                        pNode->keys[i] = pNode->keys[i-1];
                    }
                    // pNode接收左兄弟的last_key
                    pNode->children[0] = brother->children[brother->key_num];
                    pNode->keys[0] = parent->keys[index];
                    pNode->key_num++;

                    // pNode所含的最小key改变，故更新parent
                    parent->keys[index]= brother->keys[brother->key_num-1];
                    // 更新被移交结点的father
                    if (brother->children[brother->key_num])
                        brother->children[brother->key_num]->parent = pNode;
                    brother->deleteKeyByIndex(brother->key_num-1);

                    return true;

                } else {
                    // 左兄弟刚好有min_key_num-1，故将pNode并入左兄弟
                    brother->keys[brother->key_num] = parent->keys[index];
                    parent->deleteKeyByIndex(index);
                    brother->key_num++;

                    for (int i = 0; i < pNode->key_num; i++) {
                        brother->children[brother->key_num+i] = pNode->children[i];
                        brother->keys[brother->key_num+i] = pNode->keys[i];
                        brother->children[brother->key_num+i]-> parent= brother;
                    }
                    brother->children[brother->key_num+pNode->key_num] = pNode->children[pNode->key_num];
                    brother->children[brother->key_num+pNode->key_num]->parent = brother;

                    brother->key_num += pNode->key_num;

                    delete pNode;
                    node_num --;
                    // parent失去一个key，故maintain parent
                    return deleteMaintain(parent);
                }

            } else {
                // 选择pNode的右兄弟
                if (parent->children[0] == pNode)
                    brother = parent->children[1];
                else
                    brother = parent->children[index+2];
                if (brother->key_num > min_key_num - 1) {
                    // 右兄弟有“多余”的结点
                    pNode->children[pNode->key_num+1] = brother->children[0];
                    pNode->keys[pNode->key_num] = brother->keys[0];
                    pNode->children[pNode->key_num+1]->parent = pNode;
                    pNode->key_num++;

                    if (pNode == parent->children[0])
                        parent->keys[0] = brother->keys[0];
                    else
                        parent->keys[index+1] = brother->keys[0];

                    brother->children[0] = brother->children[1];
                    brother->deleteKeyByIndex(0);

                    return true;
                } else {
                    // 右兄弟刚好有min_key_num-1，故将右兄弟并入pNode
                    pNode->keys[pNode->key_num] = parent->keys[index];

                    if(pNode == parent->children[0])
                        parent->deleteKeyByIndex(0);
                    else
                        parent->deleteKeyByIndex(index+1);

                    pNode->key_num++;

                    for (int i = 0; i < brother->key_num; i++) {
                        pNode->children[pNode->key_num+i] = brother->children[i];
                        pNode->keys[pNode->key_num+i] = brother->keys[i];
                        pNode->children[pNode->key_num+i]->parent = pNode;
                    }
                    pNode->children[pNode->key_num+brother->key_num] = brother->children[brother->key_num];
                    pNode->children[pNode->key_num+brother->key_num]->parent = pNode;

                    pNode->key_num += brother->key_num;

                    delete brother;
                    node_num--;
                    // parent失去一个key，故maintain parent
                    return deleteMaintain(parent);
                }

            }

        }

    }

    return false;
}

// 递归删除整棵树并释放内存空间
template <class T>
void BPlusTree<T>::dropTree(Tree node)
{
    if (!node) return;
    if (!node->is_leaf) {
        for(unsigned int i = 0; i <= node->key_num; i++) {
            dropTree(node->children[i]);
            node->children[i] = NULL;
        }
    }
    delete node;
    node_num--;
    return;
}

//输入：key1，key2，返回vals的容器
//功能：返回范围搜索结果，将value放入vals容器中
template <class T>
void BPlusTree<T>::searchRange(T& key1, T& key2, std::vector<int>& vals, int flag)
{
	if (!root)
		return;

	if (flag == 2) { // 找(key1, all)
		nodePos np1;
		findLeaf(root, key1, np1);

		Tree pNode = np1.pNode;
		unsigned int index = np1.index;

        while(1)
        {
            pNode->findRangeAfterIndex(index, vals);
            index = 0;
            if(!pNode->next_leaf)break;
            else pNode = pNode->next_leaf;
        }
	} else if (flag == 1) { // 找（key2, all） 
		nodePos np2;
		findLeaf(root, key2, np2);

		Tree pNode = np2.pNode;
		unsigned int index = np2.index;

        while(1)
        {
            pNode->findRangeAfterIndex(index, vals);
            index = 0;
            if(!pNode->next_leaf)break;
            else pNode = pNode->next_leaf;
        }
	} else { // 找(key1, key2)或者(key2, key1)
		nodePos np1, np2;
	    findLeaf(root, key1, np1);
		findLeaf(root, key2, np2);
		bool finished = false;
		unsigned int index;

		if (key1 <= key2) {
			Tree pNode = np1.pNode;
			index = np1.index;
			do {
				finished = pNode->findRangeIndexKey(index, key2, vals);
				index = 0;
				if (pNode->next_leaf == NULL)
					break;
				else
					pNode = pNode->next_leaf;
			} while (!finished);
		} else {
			Tree pNode = np2.pNode;
			index = np2.index;
			do {
				finished = pNode->findRangeIndexKey(index, key1, vals);
				index = 0;
				if (pNode->next_leaf == NULL)
					break;
				else
					pNode = pNode->next_leaf;
			} while (!finished);
		}
	}

	std::sort(vals.begin(),vals.end());
    // "hello,world" -> "helo,world." 去重
    vals.erase(unique(vals.begin(), vals.end()), vals.end());
	return;
}

//获取文件大小
    // 创建文件
template <class T>
void BPlusTree<T>::getFile(std::string fname) {
    FILE* f = fopen(fname.c_str() , "r");
    if (f == NULL) {
        f = fopen(fname.c_str(), "w+");
        fclose(f);
        f = fopen(fname.c_str() , "r");
    }
    fclose(f);
    return;
}
    // 获取块号
template <class T>
int BPlusTree<T>::getBlockNum(std::string table_name)
{
    char* p;
    int block_num = -1;
    do {
        p = BM.getPage(table_name , block_num + 1);
        block_num++;
    } while(p[0] != '\0');
    return block_num;
}

template <class T>
void BPlusTree<T>::readFromDiskAll()
{
    std::string fname = "./database/index/" + file_name;

    getFile(fname);// 创建文件
    int block_num = getBlockNum(fname);

	if (block_num <= 0)// empty table
        block_num = 1;

	for (int i = 0; i < block_num; i++) {
        char* p = BM.getPage(fname, i);
		readFromDisk(p, p+PAGESIZE);
    } // 一次把整个表都读到BUFFER里
}

template <class T>
void BPlusTree<T>::readFromDisk(char* p, char* end)
{
    T key;
    int value;
    
    // # keys[0] vals[0] # keys[1] vals[1] ...
    for (int i = 0; i < PAGESIZE; i++)
        if (p[i] != '#')
            return;
        else {
            i += 2;
            char tmp[100];
            int j;
            
            for (j = 0; i < PAGESIZE && p[i] != ' '; i++)// 去除空格
                tmp[j++] = p[i];
            tmp[j] = '\0';
            //tmp类似于 keys[0]'\0'...
            std::string s(tmp);
            std::stringstream stream(s);
            stream >> key;
            
            memset(tmp, 0, sizeof(tmp));
            
            i++;
            for (j = 0; i < PAGESIZE && p[i] != ' '; i++)
                tmp[j++] = p[i];
            tmp[j] = '\0';
            //tmp类似于 vals[0]'\0'...
            std::string s1(tmp);
            std::stringstream stream1(s1);
            stream1 >> value;
            
            insertKey(key, value);
        }
}


template <class T>
void BPlusTree<T>::writtenbackToDiskAll()
{
    std::string fname = "./database/index/" + file_name;

    getFile(fname);
	int block_num = getBlockNum(fname);

    Tree ntmp = first_leaf;
	int i, j;
    
    // 将所有叶子结点的key, val写入page
    for (j = 0, i = 0; ntmp != NULL; j++) {
		char* p = BM.getPage(fname, j);
        int offset = 0;
        
		memset(p, 0, PAGESIZE); // 将page清空
        
        // # keys[0] vals[0] # keys[1] vals[1] ...
        for (i = 0; i < ntmp->key_num; i++) {
            p[offset++] = '#';
            p[offset++] = ' ';
            
            copyString(p, offset, ntmp->keys[i]);
            p[offset++] = ' ';
            copyString(p, offset, ntmp->vals[i]);
            p[offset++] = ' ';
		}
        
        p[offset] = '\0';// 标志一个page的结束

		int page_id = BM.getPageId(fname, j);
		BM.modifyPage(page_id);
        
        ntmp = ntmp->next_leaf;
    }

    // 没用完的block都填0并标记已修改。
    while (j < block_num) {
		char* p = BM.getPage(fname, j);
		memset(p, 0, PAGESIZE);

		int page_id = BM.getPageId(fname, j);
		BM.modifyPage(page_id);

        j++;
    }

	return;

}

#endif
