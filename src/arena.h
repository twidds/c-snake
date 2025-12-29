#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h> //malloc, NULL

#define ARENA_SLABSIZE 4096 //Bytes

typedef struct MemorySlab {
  struct MemorySlab* next;
  uintptr_t data[ARENA_SLABSIZE/sizeof(uintptr_t) - 1];
} MemorySlab;

typedef struct Arena {
    struct MemorySlab* head;
    uintptr_t free;
    uintptr_t end;
} Arena;


void arena_init(Arena **arena);
void* arena_alloc(Arena *arena, size_t size);
void arena_destroy(Arena **arena);

#endif