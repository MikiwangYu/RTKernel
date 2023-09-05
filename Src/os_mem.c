#include "os_mem.h"
static uint32_t buf[OS_MEM_TOTAL_COST / 4];

#if OS_MEM_EN
static uint32_t p;
void* OS_Malloc(size_t size) {
    void* ret = buf + p;
    p += (size + 3) / 4;
    return ret;
}
void* OS_Calloc(unsigned int num, unsigned int size) {
    return OS_Malloc(1ll * num * size);
}
void* OS_Realloc(void* mem_address, unsigned int newsize) {
    return OS_Malloc(newsize);
}
void OS_Free(void* ptr) {
    return;
}
#endif

void* OS_Heap_Base(void) {
    return buf;
}