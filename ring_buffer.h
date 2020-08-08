#ifndef RB_RING_BUFFER_ATOMIC_H
#define RB_RING_BUFFER_ATOMIC_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

typedef struct {
    uint8_t*      mem;
#ifdef RB_ATOMIC
    atomic_size_t r;
    atomic_size_t w;
    atomic_size_t h; // hole index
#else
    size_t r;
    size_t w;
    size_t h;
#endif
    size_t size;
} rb_buffer;

typedef struct {
    uint8_t* buf;
    uint8_t  wrap;
    size_t   last;
} rb_reservation;

void     rb_buffer_init   (rb_buffer* rb, uint8_t* mem, size_t size);
int      rb_buffer_reserve(rb_buffer* rb, rb_reservation* rs, size_t size);
void     rb_buffer_commit (rb_buffer* rb, rb_reservation* rs, size_t size);
uint8_t* rb_buffer_read   (rb_buffer* rb, size_t* actual_size);
void     rb_buffer_consume(rb_buffer* rb, uint8_t* ptr, size_t size);
#endif
