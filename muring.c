#include "muring.h"
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

#ifdef RB_ATOMIC
#define STORE(p, v, m) atomic_store_explicit(p, v, m)
#define LOAD(p, m)     atomic_load_explicit(p, m)
#else
#define STORE(p, v, _) (*(p) = v)
#define LOAD(p, _)     (*(p))
#endif

void muring_init(muring_buffer* ctx, uint8_t* mem, size_t size)
{
    ctx->mem = mem;
    ctx->r = 0;
    ctx->w = 0;
    ctx->h = size;
    ctx->size = size;
}

int muring_reserve(muring_buffer* ctx, muring_reservation* rs, size_t size)
{
    // There are two possible cases:
    //  1. r <= w -- 'normal' setup with no wraparound
    //  2. r > w  -- 'abnormal' setup with wraparound
    const size_t w = LOAD(&ctx->w, memory_order_relaxed);
    const size_t r = LOAD(&ctx->r, memory_order_acquire);
    if (w >= r) {
#ifndef MURING_ATOMIC
        if (r == w) {
            // we are allowed to change r when we are in sync mode
            // so this allows us to use the whole buffer instead of
            // potentially half of it (worse case).
            ctx->r = 0;
            ctx->w = 0;
        }
#endif
        if (ctx->size - w >= size) {
            rs->buf = ctx->mem + w;
            rs->wrap = 0;
            rs->last = w;
            return 1;
        }
        if (r > size) {
            rs->buf = ctx->mem;
            rs->wrap = 1;
            rs->last = w;
            return 1;
        }
    } else {
        if (r - w > size) {
            rs->buf = ctx->mem + w;
            rs->wrap = 0;
            rs->last = w;
            return 1;
        }
    }
    return 0;
}

void muring_commit(muring_buffer* ctx, muring_reservation* rs, size_t size)
{
    if (size > 0) {
        if (rs->wrap) {
            STORE(&ctx->h, rs->last, memory_order_relaxed);
        }
        STORE(&ctx->w, (rs->buf - ctx->mem) + size, memory_order_release);
    }
}

uint8_t* muring_read(muring_buffer* ctx, size_t* size)
{
    size_t sz = 0;
    size_t r = LOAD(&ctx->r, memory_order_relaxed);
    const size_t w = LOAD(&ctx->w, memory_order_acquire);
retry:
    if (w >= r) {
        sz = w - r;
    } else {
        // Note: here, ctx->r <= ctx->h, since we reach here if at
        // some point we get a wraparound, and h was set to *that*
        // value of w.
        const size_t h = LOAD(&ctx->h, memory_order_relaxed);
        if (r == h) {
            r = 0;
            goto retry;
        }
        sz = h - r;
    }
    *size = sz;
    return (sz > 0) ? (ctx->mem + r) : NULL;
}

void muring_consume(muring_buffer* ctx, uint8_t* ptr, size_t size)
{
    STORE(&ctx->r, (ptr - ctx->mem) + size, memory_order_release);
}
