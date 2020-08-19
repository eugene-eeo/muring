#include "diode.h"
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

static
muring_diode_entry* read(atomic_uintptr_t* p)
{
    muring_diode_entry* me = (muring_diode_entry*) atomic_load(p);
    if (me != NULL && atomic_fetch_add(&me->rc, 1) == 0) {
        // already expired
        /* atomic_store(&m); */
        return NULL;
    }
    return me;
}

// return whether we should free the entry
static
int release(muring_diode_entry* me)
{
    return me != NULL && atomic_fetch_add(&me->rc, -1) == 0;
}

void muring_diode_init(muring_diode* md, uintptr_t* mem, size_t size)
{
    md->size = size;
    md->mem = (atomic_uintptr_t *) mem;
    md->r = 0;
    md->w = 0;
}

int muring_diode_send(muring_diode* md, muring_diode_entry* entry, muring_diode_entry** to_free)
{
    for (;;) {
        size_t seq = atomic_fetch_add(&md->w, 1);
        size_t idx = seq % md->size;
        muring_diode_entry* old = read(&md->mem[idx]);

        // don't overwrite newer entries
        if (old != NULL && old->seq > seq) {
            if (release(old)) {
                // check if we should free here
                *to_free = old;
                return 0;
            }
            continue;
        }

        entry->rc = 1;
        entry->seq = seq;
        if (atomic_compare_exchange_weak(
                    &md->mem[idx],
                    (uintptr_t *) &old,
                    (uintptr_t  ) entry)) {
            *to_free = release(old) ? old : NULL;
            return 1;
        }
    }
}

int muring_diode_recv(muring_diode* md, muring_diode_entry** entry)
{
    size_t idx = md->r % md->size;
    muring_diode_entry* me = read(&md->mem[idx]);

    if (me == NULL || me->seq < md->r) {
        *entry = release(me) ? me : NULL;
        return 0;
    }

    if (me->seq > md->r) {
        md->r = me->seq;
    }

    md->r++;
    *entry = me;
    return 1;
}

int muring_diode_should_free(muring_diode_entry* me)
{
    return release(me);
}
