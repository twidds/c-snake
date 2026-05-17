#include "arena.h"
#include <assert.h>

#define ALIGN_SIZE 8

/*  --------------------------------------------------------------------------------------- /
                                Arena Memory Functions
    --------------------------------------------------------------------------------------- */
void arena_init(Arena **arena) {
    Arena* a = malloc(sizeof(Arena));
    a->head = malloc(sizeof(MemorySlab));
    a->head->next = NULL;
    a->free = (uintptr_t)&(a->head->data);
    a->end = (uintptr_t)&a->head->data + sizeof(((MemorySlab*)0)->data);
    *arena = a;
}

void* arena_alloc(Arena* arena, size_t size) {
    size_t size_padded = (size + sizeof(uintptr_t) - 1) & ~(sizeof(uintptr_t)-1);
    assert(size_padded >= size && size_padded % sizeof(uintptr_t) == 0);
    
    if (size_padded > ARENA_SLABSIZE) {
        return NULL; //Can't allocate that big
    }

    if (arena->free + size_padded > arena->end) { //Allocate new slab
        MemorySlab* last = arena->head;
        while (last->next) { last = last->next; }

        last->next = malloc(sizeof(MemorySlab));
        last = last->next;
        last->next = NULL;
        
        arena->free = (uintptr_t)&last->data;
        arena->end = (uintptr_t)&last->data + sizeof(((MemorySlab*)0)->data);
    }

    void* addr = (void*)arena->free;
    arena->free += size_padded;
    assert(arena->free % ALIGN_SIZE == 0);
    return addr;
}

//Deallocates the arena and all of its memory
void arena_destroy(Arena **arena) {
    MemorySlab* slab = (*arena)->head;
    while (slab) {
        MemorySlab* next = slab->next;
        free(slab);
        slab = next;
    }
    free(*arena);
    *arena = NULL;
}