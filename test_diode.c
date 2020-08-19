#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "diode.h"

#define NTHREADS 8

// Thread tasks
typedef struct {
    muring_diode*        diode;
    muring_diode_entry** entries;
    size_t               sz;
} thread_args;

void* producer(void* argv)
{
    thread_args* args = (thread_args*) argv;
    muring_diode_entry* to_free;
    int ok;
    for (size_t i = 0; i < args->sz; i++) {
        for (;;) {
            ok = muring_diode_send(args->diode,
                                   args->entries[i],
                                   &to_free);
            if (to_free != NULL)
                free(to_free);
            if (ok)
                break;
        }
    }
    return NULL;
}

void* consumer(void* argv)
{
    thread_args* args = (thread_args*) argv;
    // check that we get monotone increasing seq...
    int seen = 0;
    size_t prev;
    size_t i = 0;

    for (;;) {
        muring_diode_entry* e = NULL;
        while (!muring_diode_recv(args->diode, &e)) {
            if (e != NULL)
                free(e);
        }
        // e should not be NULL here!
        assert(e != NULL);
        i++;
        if (!seen) {
            prev = e->seq;
            seen = 1;
            if (muring_diode_should_free(e))
                free(e);
            continue;
        }
        assert(e->seq > prev);
        prev = e->seq;
        if (muring_diode_should_free(e))
            free(e);
        if (prev == (args->sz - 1)) {
            break;
        }
    }

    return NULL;
}

int main()
{
    pthread_t   rthread;
    pthread_t   wthreads[NTHREADS];
    thread_args th_args[NTHREADS + 1];
    // alloc
    muring_diode_entry** buffer  = calloc(256, sizeof(muring_diode_entry*));
    muring_diode_entry** entries = calloc(512, sizeof(muring_diode_entry*));
    for (int i = 0; i < 512; i++)
        entries[i] = (muring_diode_entry*) malloc(sizeof(muring_diode_entry));

    muring_diode diode;
    muring_diode_init(&diode, (uintptr_t *) buffer, 256);

    for (int i = 0; i < NTHREADS; i++) {
        th_args[i].sz = 512 / NTHREADS;
        th_args[i].diode = &diode;
        th_args[i].entries = &entries[i * th_args[i].sz];
        pthread_create(&wthreads[i], NULL, producer, (void*) &th_args[i]);
    }

    th_args[NTHREADS].sz = 512;
    th_args[NTHREADS].diode = &diode;
    pthread_create(&rthread, NULL, consumer, (void*) &th_args[NTHREADS]);

    for (int i = 0; i < NTHREADS; i++)
        pthread_join(wthreads[i], NULL);
    pthread_join(rthread, NULL);

    /* for (int i = 0; i < 512; i++) */
        /* free(entries[i]); */

    free(buffer);
    free(entries);
}
