#include "ring_buffer_atomic.h"
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

#define MIN(a, b)      ((a) < (b) ? (a) : (b))
#define STORE(p, v, m) atomic_store_explicit(p, v, m)
#define LOAD(p, m)     atomic_load_explicit(p, m)

void rb_buffer_init(rb_buffer* rb, uint8_t* mem, size_t size)
{
    rb->mem = mem;
    rb->r = 0;
    rb->w = 0;
    rb->h = size;
    rb->size = size;
}

int rb_buffer_reserve(rb_buffer* rb, rb_reservation* rs, size_t size)
{
    const size_t w = LOAD(&rb->w, memory_order_relaxed);
    const size_t r = LOAD(&rb->r, memory_order_acquire);
    if (w >= r) {
        if (rb->size - w >= size) {
            rs->buf = rb->mem + w;
            rs->wrap = 0;
            rs->last = w;
            return 1;
        }
        if (r > size) {
            rs->buf = rb->mem;
            rs->wrap = 1;
            rs->last = w;
            return 1;
        }
    } else {
        if (r - w > size) {
            rs->buf = rb->mem + w;
            rs->wrap = 0;
            rs->last = w;
            return 1;
        }
    }
    return 0;
}

void rb_buffer_commit(rb_buffer* rb, rb_reservation* rs, size_t size)
{
    if (size > 0) {
        if (rs->wrap) {
            STORE(&rb->h, rs->last, memory_order_relaxed);
        }
        STORE(&rb->w, (rs->buf - rb->mem) + size, memory_order_release);
    }
}

uint8_t* rb_buffer_read(rb_buffer* rb, size_t size)
{
    size_t r = LOAD(&rb->r, memory_order_relaxed);
    const size_t w = LOAD(&rb->w, memory_order_acquire);
retry:
    ;
    uint8_t* ptr = rb->mem + r;
    if (w >= r) {
        if (w - r >= size) {
            return ptr;
        }
    } else {
        const size_t h = LOAD(&rb->h, memory_order_relaxed);
        if (r == h) {
            r = 0;
            goto retry;
        }
        if (h - r >= size) {
            return ptr;
        }
    }
    return NULL;
}

void rb_buffer_consume(rb_buffer* rb, uint8_t* ptr, size_t size)
{
    STORE(
        &rb->r,
        (ptr - rb->mem) + size,
        memory_order_release
    );
}
