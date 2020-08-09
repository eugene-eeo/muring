#ifndef MURING_H
#define MURING_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

typedef struct {
    uint8_t*      mem;
#ifdef MURING_ATOMIC
    atomic_size_t r;
    atomic_size_t w;
    atomic_size_t h; // hole index
#else
    size_t r;
    size_t w;
    size_t h;
#endif
    size_t size;
} muring_buffer;

typedef struct {
    uint8_t* buf;
    uint8_t  wrap;
    size_t   last;
} muring_reservation;

void     muring_init   (muring_buffer* ctx, uint8_t* mem, size_t size);
int      muring_reserve(muring_buffer* ctx, muring_reservation* rs, size_t size);
void     muring_commit (muring_buffer* ctx, muring_reservation* rs, size_t size);
uint8_t* muring_read   (muring_buffer* ctx, size_t* size);
void     muring_consume(muring_buffer* ctx, uint8_t* ptr, size_t size);
#endif
