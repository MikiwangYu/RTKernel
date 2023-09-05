#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"
#if OS_MEM_EN
void* OS_Malloc(size_t size);
void* OS_Calloc(unsigned int num, unsigned int size);
void* OS_Realloc(void* mem_address, unsigned int newsize);
void OS_Free(void* ptr);
#else
extern void* malloc(size_t size);
extern void* calloc(unsigned int num,unsigned int size);
extern void* realloc(void* mem_address, unsigned int newsize);
extern void free(void* ptr);
#define OS_Malloc   malloc
#define OS_Calloc   calloc
#define OS_Realloc  realloc
#define OS_Free     free
#endif
void* OS_Heap_Base(void);



#ifdef __cplusplus
}
#endif