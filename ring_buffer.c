#include <stddef.h>
#include <stdint.h>
#include "ring_buffer.h"

#define MIN(a, b) (((a) > (b)) ? (b) : (a))

void rb_buffer_init(rb_buffer* rb, uint8_t* mem, size_t size)
{
    rb->mem = mem;
    rb->r = 0; // next index to read from
    rb->w = 0; // next index to write to
    rb->h = size;
    rb->size = size;
}

uint8_t* rb_buffer_reserve(rb_buffer* rb, size_t size)
{
    // There are two possible states:
    //  1) r <= w -- normal setup
    //  2) r >  w -- write overtook read
    if (rb->r <= rb->w) {
        rb->h = rb->size; // safe here: if r <= w, we've skipped past h
        if (rb->r == rb->w) {
            rb->r = 0;
            rb->w = 0;
        }
        if (rb->h - rb->w >= size) {
            return rb->mem + rb->w;
        }
        // otherwise check the left side of rb->r
        if (rb->r > size) {
            return rb->mem;
        }
    } else {
        if (rb->r - rb->w > size) {
            return rb->mem + rb->w;
        }
    }
    return NULL;
}

void rb_buffer_commit(rb_buffer* rb, uint8_t* ptr, size_t size)
{
    if (size == 0) {
        return;
    }
    if (ptr < rb->mem + rb->w) {
        rb->h = rb->w;
    }
    rb->w = (ptr - rb->mem) + size;
}

uint8_t* rb_buffer_read(rb_buffer* rb, size_t* actual_size, size_t max_size)
{
retry:
    ;
    size_t size = 0;
    uint8_t* ptr = rb->mem + rb->r;
    if (rb->r <= rb->w) {
        // Case 1:
        // | r | ... | w |
        size = MIN(rb->w - rb->r, max_size);
        rb->r += size;
    } else {
        // Case 2:
        // ... | w | ... | r | ... | h |
        // Note: we only get here iff at some point w was >= r, and h was
        // set to *that* value of w, so rb->h >= rb->r.
        if (rb->r == rb->h) {
            rb->r = 0;
            goto retry;
        }
        size = MIN(rb->h - rb->r, max_size);
        rb->r += size;
    }
    *actual_size = size;
    return size > 0 ? ptr : NULL;
}

size_t rb_buffer_total(rb_buffer* rb)
{
    if (rb->r <= rb->w) {
        // Case 1:
        // ... | r | ... | w | ...
        return rb->w - rb->r;
    } else {
        // Case 2:
        // ... | w | ... | r | ... | h |
        return rb->h - rb->r + rb->w;
    }
}
