#include <unistd.h>
#include <stdlib.h>
//Best Fit malloc/free

void *ts_malloc_lock(size_t size);
//void *ts_malloc_lock_(size_t size);
void ts_free_lock(void *ptr);
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);


void *heap_start = 0;
void *heap_end =0;
// void *heap_start = 0;
// void *heap_end =0;

void ts_malloc_lock_init(){
  heap_start = heap_end = sbrk(0);
}

void ts_malloc_nolock_init(){
  heap_start = heap_end = sbrk(0);
}


struct mem_meta{
  int is_available;
  size_t real_size;
  size_t size;
  struct  mem_meta * prev;
  struct  mem_meta * next;
  unsigned long thread_id;
};
