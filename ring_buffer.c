#include "ring_buffer.h"

#define MIN(a, b) (((a) > (b)) ? (b) : (a))

void rb_buffer_init(rb_buffer* rb, void* mem, size_t size)
{
    rb->mem = mem;
    rb->r = mem;
    rb->w = mem;
    rb->h = mem + size;
    rb->s = 0;
    rb->size = size;
}

void* rb_buffer_reserve(rb_buffer* rb, size_t size)
{
    // There are two possible states:
    //  1) r <= w -- normal setup
    //  2) r >  w -- write overtook read
    rb->s = size;
    if (rb->r <= rb->w) {
        if ((size_t)(rb->h - rb->w) >= size) {
            return rb->w;
        }
        // otherwise check the left side of rb->r
        if ((size_t)(rb->r - rb->mem) > size) {
            return rb->mem;
        }
    } else {
        if ((size_t)(rb->r - rb->w) > size) {
            return rb->w;
        }
    }
    // not enough space
    rb->s = 0;
    return NULL;
}

void rb_buffer_commit(rb_buffer* rb, size_t size)
{
    if (rb->r <= rb->w) {
        if ((size_t)(rb->h - rb->w) >= rb->s) {
            // normal setup with r behind w
            rb->w += size;
        } else {
            // gap
            rb->h = rb->w;
            rb->w = rb->mem + size;
        }
    } else {
        rb->w += size;
    }
    rb->s = 0;
}

void* rb_buffer_read(rb_buffer* rb, size_t* actual_size, size_t max_size)
{
    size_t size = 0;
    void* ptr = rb->r;
    if (rb->w >= rb->r) {
        // Case 1:
        // | r | ... | w |
        size = MIN((size_t)(rb->w - rb->r), max_size);
        rb->r += size;
        if (size > 0 && rb->r == rb->w) {
            rb->r = rb->mem;
            rb->w = rb->mem;
        }
    } else {
        // Case 2:
        // ... | w | ... | r | ... | h |
        // (Note: we only get here iff at some point
        //  w was >= r, and h was set to *that* value
        //  of w, so rb->h should be >= rb->r.)
        size = MIN((size_t)(rb->h - rb->r), max_size);
        rb->r += size;
        if (size > 0 && rb->r == rb->h) {
            // if rb->r reaches 'end', skip ahead
            rb->r = rb->mem;
            rb->h = rb->mem + rb->size;
        }
        // Don't need to check if rb->r == rb->w.
        // if rb->w == rb->r == rb->mem, it is handled.
        // Otherwise rb->w < rb->r. (Case 2)
    }
    *actual_size = size;
    return size > 0 ? ptr : NULL;
}

size_t rb_buffer_total(rb_buffer* rb)
{
    if (rb->w >= rb->r) {
        // Case 1:
        // | r | ... | w |
        return (size_t)(rb->w - rb->r);
    } else {
        // Case 2:
        // ... | w | ... | r | ... | h |
        return (size_t)(rb->h - rb->r)
             + (size_t)(rb->w - rb->mem);
    }
}
