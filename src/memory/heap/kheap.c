#include "kheap.h"
#include "heap.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

void kheap_init()
{
    int total_table_entries = SERPAEOS_HEAP_SIZE_BYTES / SERPAEOS_HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)(SERPAEOS_HEAP_TABLE_ADDRESS);
    kernel_heap_table.total = total_table_entries;

    void* end = (void*)(SERPAEOS_HEAP_ADDRESS + SERPAEOS_HEAP_SIZE_BYTES);
    int res = heap_create(&kernel_heap, (void*)(SERPAEOS_HEAP_ADDRESS), end, &kernel_heap_table);
    if (res < 0)
    {
        panic("Failed to create heap\n");
    }

}

void* kmalloc(size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

void* kzalloc(size_t size)
{
    void* ptr = kmalloc(size);
    if (!ptr)
        return 0;

    memset(ptr, 0x00, size);
    return ptr;
}


void kfree(void* ptr)
{
    heap_free(&kernel_heap, ptr);
}