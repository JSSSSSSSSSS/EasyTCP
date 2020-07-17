/*
注意！！！
	在release版本下：
	内存池不能用于std::cin，因为std::cin中涉及_facet的使用，_facet会在静态变量后面释放内存。
	会造成内存池的内存已经释放，但是_facet又释放一遍的异常。


每个块的结构：
	|——————|————————|
	|  头部信息	 | 实际可用的内存 |
	|——————|————————|

	头部信息：所属内存池的指针，便于释放时，回收到相应的池中
	实际可用内存：分配出去的内存，用于外部构造对象
*/

#pragma once
#ifndef __MemMgr__
#define __MemMgr__

#ifdef _DEBUG
	#ifndef xPrintf
		#include<stdio.h>
		#define xPrintf(...) printf(__VA_ARGS__)
	#endif
	#ifndef xGet
		#define xGet(block) {block->_isreturn = false;}
	#endif
	#ifndef xReturn
		#define xReturn(block) {block->_isreturn = true;}
	#endif
	#ifndef xSetid
		#define xSetid(block,id) {block->_nid = id;}
	#endif
	#ifndef xRemainAdd
		#define xRemainAdd() {_remain++;}
	#endif
	#ifndef xRemainRe
		#define xRemainRe() {_remain--;}
	#endif
#else
	#ifndef xPrintf
		#define xPrintf(...)
	#endif
	#ifndef xGet
		#define xGet(block) 
	#endif
	#ifndef xReturn
		#define xReturn(block) 
	#endif
	#ifndef xSetid
		#define xSetid(block,id) 
	#endif
	#ifndef xRemainAdd
		#define xRemainAdd() 
	#endif
	#ifndef xRemainRe
		#define xRemainRe() 
	#endif
#endif // _DEBUG

#include<mutex>
#define MAX_MEMORY_SIZE		256
#define POOL_16_NUM			100
#define POOL_32_NUM			2000000
#define POOL_64_NUM			2000000
#define POOL_128_NUM		4000000
#define POOL_256_NUM		100



class MemPool;
//块的头部描述信息
struct MemBlock
{
	MemPool*	_pool;						//所属内存池指针
	MemBlock*	_nextblock;					//下一个可用块
	bool		_ispool;					//是否在内存池中
#ifdef _DEBUG
	int			_nid;
	bool		_isreturn;					//是否归还
#endif
};

class MemPool
{
public:
	MemPool();
	virtual ~MemPool();
	//初始化内存池
	bool InitMemPool();
	//分配内存
	void* MemAlloc(size_t nsize);
	//释放内存
	void MemDelete(void* p);
protected:
	void*		_begin;			//指向内存池起始地址
	MemBlock*	_firstBlock;	//指向第一个可用块
	size_t		_blockSize;		//每一个块的大小
	size_t		_blockNum;		//块的总数量
	std::mutex	_mutex;

#ifdef _DEBUG
	size_t		_remain;		//剩余块的数量
#endif
};

//内存池工具，借助模板便于在声明类成员变量时初始化MemoryAlloc的成员数据
template<size_t blocksize,size_t blocknum>
class MemPoolTor :public MemPool
{
public:
	MemPoolTor()
	{
		_blockSize = blocksize;
		_blockNum = blocknum;
#ifdef _DEBUG
		_remain = blocknum;
#endif // _DEBUG
	}
};
//单例内存池管理类
class MemMgr
{
public:
	~MemMgr();
	static MemMgr& Instance();
	//申请内存
	void* Alloc(size_t nsize);
	//释放内存
	void Free(void* p);
private:
	MemMgr();
	//初始化映射数组
	void InitMapArray(size_t begin, size_t end, MemPool* pool);
private:
	MemPoolTor<16, POOL_16_NUM>			_pool_16;
	MemPoolTor<32, POOL_32_NUM>			_pool_32;
	MemPoolTor<64, POOL_64_NUM>			_pool_64;
	MemPoolTor<128, POOL_128_NUM>		_pool_128;
	MemPoolTor<256, POOL_256_NUM>		_pool_256;
	MemPool*							_poolIndex[MAX_MEMORY_SIZE + 1];
};

#endif // !__MemMgr__



