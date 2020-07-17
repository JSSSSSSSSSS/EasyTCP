#include"Alloctor.h"

//全局实现
void* operator new(size_t nsize)
{
	return MemMgr::Instance().Alloc(nsize); 
}; 
void operator delete(void* p)
{
	MemMgr::Instance().Free(p); 
}; 
void* operator new[](size_t nsize)
{
	return MemMgr::Instance().Alloc(nsize); 
}; 
void operator delete[](void* p)
{
	MemMgr::Instance().Free(p);
};