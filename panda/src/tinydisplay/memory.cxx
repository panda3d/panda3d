/*
 * Memory allocator for TinyGL
 */
#include "zgl.h"
#include "memoryHook.h"

/* modify these functions so that they suit your needs */

void gl_free(void *p)
{
    PANDA_FREE_ARRAY(p);
}

void *gl_malloc(int size)
{
    return PANDA_MALLOC_ARRAY(size);
}

void *gl_zalloc(int size)
{
    void *buffer = PANDA_MALLOC_ARRAY(size);
    memset(buffer, 0, size);
    return buffer;
}
