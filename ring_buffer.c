#include "ring_buffer.h"

#define MIN(a, b) (((a) > (b)) ? (b) : (a))

void rb_buffer_init(rb_buffer* rb, void* mem, size_t size)
{
    rb->mem = mem;
    rb->r = 0; // next index to read from
    rb->w = 0; // next index to write to
    rb->h = size;
    rb->size = size;
}

void* rb_buffer_reserve(rb_buffer* rb, size_t size)
{
    // There are two possible states:
    //  1) r <= w -- normal setup
    //  2) r >  w -- write overtook read
    if (rb->r <= rb->w) {
        // safe here: if r <= w, then h is already skipped past
        rb->h = rb->size;
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

void rb_buffer_commit(rb_buffer* rb, void* ptr, size_t size)
{
    void* end = ptr + size;
    if (end == ptr) {
        return;
    }
    if (end < rb->mem + rb->w) {
        // wraparound, need to update hole
        rb->h = rb->w;
    }
    rb->w = end - rb->mem;
}

void* rb_buffer_read(rb_buffer* rb, size_t* actual_size, size_t max_size)
{
    size_t size = 0;
    void* ptr = rb->mem + rb->r;
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
        size = MIN(rb->h - rb->r, max_size);
        rb->r += size;
        if (rb->r == rb->h)
            rb->r = 0;
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
