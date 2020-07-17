#include<stdlib.h>
#include "MemMgr.h"
#include<iostream>

MemPool::MemPool()
	:_begin(nullptr),_firstBlock(nullptr),_blockSize(0),_blockNum(0)
{
}

MemPool::~MemPool()
{
	xPrintf("in ~MemPool\n");
	char* p = (char * )_begin;

	if (_begin)
	{
		free(_begin);
		_begin = nullptr;
	}
	
}

bool MemPool::InitMemPool()
{
	//申请内存
	_begin = malloc((_blockSize+sizeof(MemBlock))*_blockNum);
	if (!_begin)
	{
		return false;
	}
	//整理内存
		//整理第一块内存
	_firstBlock = (MemBlock*)_begin;
	xSetid(_firstBlock, 0);
	xReturn(_firstBlock);
	_firstBlock->_pool = this;
	_firstBlock->_ispool = true;
	_firstBlock->_nextblock = nullptr;
		//整理剩余内存
	auto header = _firstBlock;
	for (int i = 1; i < _blockNum; i++)
	{
		MemBlock* block = (MemBlock*)((char*)_begin + i * (_blockSize+ sizeof(MemBlock)));
		xSetid(block, i);
		xReturn(block);
		block->_pool = this;
		block->_ispool = true;
		block->_nextblock = nullptr;
		header->_nextblock = block;
		header = block;
	}
	return true;
}

void * MemPool::MemAlloc(size_t nsize)
{
	void* _mem = nullptr;
	//检查是否初始化

	std::lock_guard<std::mutex> lg(_mutex);
	if (!_begin)
	{
		InitMemPool();
	}
	//检查是否有空闲块
	if (_firstBlock)
	{
		//如果有，就将第一块分配出去
		xRemainRe();
		xGet(_firstBlock);
		_mem = _firstBlock;
		_firstBlock = _firstBlock->_nextblock;
	}
	else
	{
		//没有空闲块，就向系统申请一个块，分配出去
		MemBlock* block = (MemBlock*)malloc(nsize + sizeof(MemBlock));
		xSetid(block, -1);
		block->_pool = nullptr;
		block->_ispool = false;
		block->_nextblock = nullptr;
		_mem = block;
	}
	//xPrintf("Alloc  ptr<%p>,nid<%d,%d>,nsize<%d>\n", _mem, _blockSize, ((MemBlock*)_mem)->_nid, nsize);
	return _mem;
}

void MemPool::MemDelete(void* p)
{
	MemBlock* block = (MemBlock*)p;
	std::lock_guard<std::mutex> lg(_mutex);
	xRemainAdd();
	xReturn(block);
	block->_nextblock = _firstBlock;
	_firstBlock = block;
	//xPrintf("Delete  ptr<%p>,nid<%d,%d>,remain<%d>\n", block, _blockSize, block->_nid, _remain);
}

//=======================================================================================================
//=======================================================================================================

MemMgr::MemMgr()
{
	InitMapArray(0, MAX_MEMORY_SIZE + 1, nullptr);
	_pool_16.InitMemPool();
	InitMapArray(1, 16 + 1, &_pool_16);
	_pool_32.InitMemPool();
	InitMapArray(17, 32 + 1, &_pool_32);
	_pool_64.InitMemPool();
	InitMapArray(33, 64 + 1, &_pool_64);
	_pool_128.InitMemPool();
	InitMapArray(65, 128 + 1, &_pool_128);
	_pool_256.InitMemPool();
	InitMapArray(129, 256 + 1, &_pool_256);
}

void MemMgr::InitMapArray(size_t begin, size_t end, MemPool * pool)
{
	for (size_t n = begin; n < end; n++)
	{
		_poolIndex[n] = pool;
	}
}


MemMgr::~MemMgr()
{
	//std::cout << "in ~MemMgr" << std::endl;
}

MemMgr & MemMgr::Instance()
{
	static MemMgr	mgr;
	return mgr;
}

void * MemMgr::Alloc(size_t nsize)
{
	MemBlock* block = nullptr;
	if (nsize < MAX_MEMORY_SIZE + 1)
	{
		block = (MemBlock*)_poolIndex[nsize]->MemAlloc(nsize);
	}
	else
	{
		block = (MemBlock*)malloc(nsize + sizeof(MemBlock));
		xSetid(block, -1);
		block->_pool = nullptr;
		block->_ispool = false;
		block->_nextblock = nullptr;
		//std::cout << "Alloc  ptr<" << block << ">, nid<unknown," << block->_nid << ">,nsize <" << nsize << ">" << std::endl;
	}
	xPrintf("Alloc  ptr<%p>,nid<%d>，nsize<%d>\n", block, block->_nid,nsize);
	return (void *)(block+1);
}

void MemMgr::Free(void * p)
{
	if (p)
	{
		MemBlock* block = (MemBlock*)p - 1;
		
		xPrintf("Delete  ptr<%p>,nid<%d>\n", block, block->_nid);
		
		if (block->_ispool)
		{
			block->_pool->MemDelete(block);
		}
		else
		{
			//std::cout << "Delete  ptr<" << block << ">, nid<unknown," << block->_nid << ">" << std::endl;
			free(block);
		}
	}
}
