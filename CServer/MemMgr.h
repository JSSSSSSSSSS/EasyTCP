/*
ע�⣡����
	��release�汾�£�
	�ڴ�ز�������std::cin����Ϊstd::cin���漰_facet��ʹ�ã�_facet���ھ�̬���������ͷ��ڴ档
	������ڴ�ص��ڴ��Ѿ��ͷţ�����_facet���ͷ�һ����쳣��


ÿ����Ľṹ��
	|������������|����������������|
	|  ͷ����Ϣ	 | ʵ�ʿ��õ��ڴ� |
	|������������|����������������|

	ͷ����Ϣ�������ڴ�ص�ָ�룬�����ͷ�ʱ�����յ���Ӧ�ĳ���
	ʵ�ʿ����ڴ棺�����ȥ���ڴ棬�����ⲿ�������
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
//���ͷ��������Ϣ
struct MemBlock
{
	MemPool*	_pool;						//�����ڴ��ָ��
	MemBlock*	_nextblock;					//��һ�����ÿ�
	bool		_ispool;					//�Ƿ����ڴ����
#ifdef _DEBUG
	int			_nid;
	bool		_isreturn;					//�Ƿ�黹
#endif
};

class MemPool
{
public:
	MemPool();
	virtual ~MemPool();
	//��ʼ���ڴ��
	bool InitMemPool();
	//�����ڴ�
	void* MemAlloc(size_t nsize);
	//�ͷ��ڴ�
	void MemDelete(void* p);
protected:
	void*		_begin;			//ָ���ڴ����ʼ��ַ
	MemBlock*	_firstBlock;	//ָ���һ�����ÿ�
	size_t		_blockSize;		//ÿһ����Ĵ�С
	size_t		_blockNum;		//���������
	std::mutex	_mutex;

#ifdef _DEBUG
	size_t		_remain;		//ʣ��������
#endif
};

//�ڴ�ع��ߣ�����ģ��������������Ա����ʱ��ʼ��MemoryAlloc�ĳ�Ա����
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
//�����ڴ�ع�����
class MemMgr
{
public:
	~MemMgr();
	static MemMgr& Instance();
	//�����ڴ�
	void* Alloc(size_t nsize);
	//�ͷ��ڴ�
	void Free(void* p);
private:
	MemMgr();
	//��ʼ��ӳ������
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



