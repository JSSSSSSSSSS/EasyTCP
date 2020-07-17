#ifndef __Alloctor__
#define __Alloctor__
#include"MemMgr.h"

//È«¾ÖÉùÃ÷
void* operator new(size_t nsize);
void operator delete(void* p);
void* operator new[](size_t nsize);
void operator delete[](void* p);

#endif // !__Alloctor__

