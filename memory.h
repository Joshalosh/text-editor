#if !defined(MEMORY_H)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define Assert(expression) if(!(expression)) {*(int *)0 = 0;}

struct Memory_Arena {
    u8 *start;
    u8 *current;
    u8 *end;
};

static Memory_Arena ArenaInit(size_t size) {
    Memory_Arena result;
    result.start   = (u8 *)malloc(size);
    result.current = result.start;
    result.end     = result.current + size;
    return result;
}

static void *ArenaAlloc(Memory_Arena *arena, size_t size) {
    void *result = NULL;
    Assert(arena->current + size <= arena->end);
    result          = arena->current;
    arena->current += size;
    return result;
}

#define ZeroStruct(instance) ZeroSize(&instance, sizeof(instance))
static void ZeroSize(void *ptr, size_t size) {
    u8 *byte = (u8 *)ptr;

    while (size--) {
        *byte++ = 0;
    }
}

static void ArenaFree(Memory_Arena *arena) {
    free(arena->start);
}

#define MEMORY_H
#endif
