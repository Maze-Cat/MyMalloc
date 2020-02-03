#include "my_malloc.h"
#include <stdio.h>
#include <pthread.h>
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
#define SIZE_MAX 100000000
void *ts_malloc_nolock(size_t size){
    if(!heap_start) ts_malloc_lock_init();
    
    size += sizeof(struct mem_meta); 
    size_t min_size=SIZE_MAX;
    void *memory_location = NULL;
    void *best_mem = NULL;
    void *last_mem = NULL;
    void *current_mem = heap_start;
    pthread_t tid = pthread_self();
    //add lock
    while(current_mem != heap_end){
        struct mem_meta * mm = current_mem;
        if (mm->thread_id==tid){ 
	    last_mem = current_mem;
            if(mm->is_available){
                if(mm->size == size){
                    mm->is_available = 0;
	            memory_location = current_mem;
                    best_mem = NULL;
	            break;
                }else if (mm->size>size){
                    if (mm->size<min_size){
                        min_size = mm->size;
                        best_mem = current_mem;
                    }
                }            
            }
        }
        current_mem += mm->size;
    }

    if(best_mem!=NULL)
    {
        struct mem_meta *mm = best_mem;
        mm->is_available=0;
        memory_location = best_mem;
    }

    if(!memory_location){
        pthread_mutex_lock(&lock);
        sbrk(size);
        memory_location = heap_end;
        struct mem_meta *mm = memory_location;
	if (last_mem){
            struct mem_meta *last_mm = last_mem;
	    last_mm->next = mm;
	    mm->prev = last_mm;
	}else{
	  mm->prev = NULL;
	}
	mm->next = NULL;
        mm->is_available = 0;
        mm->size = size;
	mm->thread_id = tid;
        heap_end += size;
        pthread_mutex_unlock(&lock);
    }
    return memory_location + sizeof(struct mem_meta);
}

void ts_free_nolock(void *ptr){
    if(!ptr) return;
    struct mem_meta *mm = ptr - sizeof(struct mem_meta);
    mm->is_available = 1;
    void* location =mm;
    if (mm->prev!=NULL && mm->prev->is_available==1){
        int size =mm->prev->size;
	int length = mm-mm->prev;
        if (size==length){
	    mm->prev->size+=mm->size;
            mm->prev->next=mm->next;
            if (mm->next!=NULL)
                mm->next->prev = mm->prev;
            mm = mm->prev;
        }
    }
    if (mm->next!=NULL && mm->next->is_available==1 ){
        int size =mm->size;
	int length = mm->next-mm;
        if (size==length){
            mm->size += mm->next->size;
            mm->next = mm->next->next;
            if (mm->prev!=NULL)
                mm->prev->next=mm;
 
       	}
    }
}

void *ts_malloc_lock(size_t size){
    if(!heap_start) ts_malloc_lock_init();
    size += sizeof(struct mem_meta); 
    size_t min_size=SIZE_MAX;
    void *memory_location = NULL;
    void *temp_mem =NULL;
    void *last_mem = NULL;
    void *current_mem = heap_start;
    //add lock
    pthread_mutex_lock(&lock);

    while(current_mem != heap_end){
        struct mem_meta * mm = current_mem;
	last_mem = current_mem;
            if(mm->is_available){
                if(mm->size == size){
                mm->is_available = 0;
	        memory_location = current_mem;
                temp_mem = NULL;
	        break;
                }else if (mm->size>size){
                    if (mm->size<min_size){
                        min_size = mm->size;
                        temp_mem = current_mem;
                     }
                }            
            }
       current_mem += mm->size;
    }
    if(temp_mem!=NULL)
    {
        struct mem_meta *mm = temp_mem;
        mm->is_available=0;
        memory_location = temp_mem;
    }
    if(!memory_location){
        sbrk(size);
        memory_location = heap_end;
        struct mem_meta *mm = memory_location;
	if (last_mem){
            struct mem_meta *last_mm = last_mem;
	    last_mm->next = mm;
	    mm->prev = last_mm;
	}else{
	  mm->prev = NULL;
	}
	mm->next = NULL;
        mm->is_available = 0;
        mm->size = size;
        heap_end += size;
    }
    pthread_mutex_unlock(&lock);
    return memory_location + sizeof(struct mem_meta);
}

void ts_free_lock(void *ptr){
  if(!ptr) return;
  struct mem_meta *mm = ptr - sizeof(struct mem_meta);
  mm->is_available = 1;
  if (mm->prev!=NULL && mm->prev->is_available==1){
    mm->prev->size+=mm->size;
    mm->prev->next=mm->next;
    if (mm->next!=NULL)
        mm->next->prev = mm->prev;
    mm = mm->prev;
  }
  if (mm->next!=NULL && mm->next->is_available==1){
    mm->size += mm->next->size;
    mm->next = mm->next->next;
    if (mm->next!=NULL)
        mm->next->prev=mm;
  }
}

