#include"Alloctor.h"

//ȫ��ʵ��
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