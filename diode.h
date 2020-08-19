#ifndef _MURING_DIODE_H
#define _MURING_DIODE_H

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

typedef struct {
    size_t        seq;
    atomic_size_t rc; // refcount
} muring_diode_entry;

typedef struct {
    size_t            size;
    atomic_uintptr_t* mem;
    atomic_size_t     r;
    atomic_size_t     w;
} muring_diode;

void muring_diode_init(muring_diode* md, uintptr_t* mem, size_t size);
int  muring_diode_send(muring_diode* md, muring_diode_entry*  entry, muring_diode_entry** to_free);
int  muring_diode_recv(muring_diode* md, muring_diode_entry** entry);
int  muring_diode_should_free(muring_diode_entry* me);
#endif
