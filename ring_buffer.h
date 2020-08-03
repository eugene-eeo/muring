#ifndef RB_RING_BUFFER_H
#define RB_RING_BUFFER_H

#include <stddef.h>

typedef struct {
    void*   mem;
    void*   r;
    void*   w;
    void*   h; // hole pointer
    size_t  s; // reservation size
    size_t  size;
} rb_buffer;

void   rb_buffer_init   (rb_buffer* rb, void* mem, size_t size);
void*  rb_buffer_reserve(rb_buffer* rb, size_t size);
void   rb_buffer_commit (rb_buffer* rb, size_t size);
void*  rb_buffer_read   (rb_buffer* rb, size_t* m, size_t n);
size_t rb_buffer_total  (rb_buffer* rb);

#endif
