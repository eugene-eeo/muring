#include <assert.h>
#include <string.h> // for memcpy
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "muring.h"

#ifdef MURING_ATOMIC
#include <pthread.h>

void* read_thread(void* vargp)
{
    uint8_t* b;
    size_t sz;
    muring_buffer* ctx = (muring_buffer*) vargp;
    // spin on this
    while ((b = muring_read(ctx, &sz)) == NULL || sz < 5) {
    }
    assert(sz == 6);
    assert(memcmp(b, "abcde", 5) == 0);
    muring_consume(ctx, b, 5);

    // next one should be immediate
    b = muring_read(ctx, &sz);
    assert(b != NULL);
    assert(sz == 1);
    assert(b[0] == 'f');
    muring_consume(ctx, b, 1);

    // spin on this
    while ((b = muring_read(ctx, &sz)) == NULL || sz < 4) {
    }
    assert(sz == 4);
    assert(memcmp(b, "ghij", 4) == 0);
    muring_consume(ctx, b, 4);
}

void* write_thread(void* vargp)
{
    muring_reservation rs;
    muring_buffer* ctx = (muring_buffer*) vargp;
    int ok = muring_reserve(ctx, &rs, 6);
    assert(ok);
    memcpy(rs.buf, "abcdef", 6);
    muring_commit(ctx, &rs, 6);

    // wait for chance to wraparound
    while (!muring_reserve(ctx, &rs, 4)) {
    }

    memcpy(rs.buf, "ghij", 4);
    muring_commit(ctx, &rs, 4);
}

static
void test_threaded()
{
    uint8_t buf[8];
    muring_buffer ctx;
    muring_init(&ctx, buf, sizeof(buf));
    pthread_t r, w;
    pthread_create(&r, NULL, read_thread,  (void*) &ctx);
    pthread_create(&w, NULL, write_thread, (void*) &ctx);
    pthread_join(r, NULL);
    pthread_join(w, NULL);
}
#endif

static
void test_use_full_buffer()
{
    uint8_t  buf[8];
    uint8_t* b;
    size_t   sz;
    int      ok;
    muring_reservation rs;
    muring_buffer ctx;
    muring_init(&ctx, buf, sizeof(buf));

    ok = muring_reserve(&ctx, &rs, 5);
    assert(ok);
    memcpy(rs.buf, "abcde", 5);
    muring_commit(&ctx, &rs, 5);

    ok = muring_reserve(&ctx, &rs, 3);
    assert(ok);
    memcpy(rs.buf, "fgh", 3);
    muring_commit(&ctx, &rs, 3);

    b = muring_read(&ctx, &sz);
    assert(b != NULL);
    assert(sz == 8);
    assert(memcmp(b, "abcdefgh", 8) == 0);
    muring_consume(&ctx, b, 5);

    b = muring_read(&ctx, &sz);
    assert(b != NULL);
    assert(sz == 3);
    assert(memcmp(b, "fgh", 3) == 0);
    muring_consume(&ctx, b, 3);

    b = muring_read(&ctx, &sz);
    assert(b == NULL);
    assert(sz == 0);

    ok = muring_reserve(&ctx, &rs, 5);
    assert(ok);
}

static
void test_wraparound()
{
    uint8_t  buf[8];
    uint8_t* b;
    size_t   sz;
    int      ok;
    muring_reservation rs;
    muring_buffer ctx;
    muring_init(&ctx, buf, sizeof(buf));

    ok = muring_reserve(&ctx, &rs, 6);
    assert(ok);
    memcpy(rs.buf, "abcdef", 6);
    muring_commit(&ctx, &rs, 6);

    ok = muring_reserve(&ctx, &rs, 1);
    assert(ok);
    memcpy(rs.buf, "g", 1);
    muring_commit(&ctx, &rs, 1);

    b = muring_read(&ctx, &sz);
    assert(b != NULL);
    assert(sz == 7);
    assert(memcmp(b, "abcde", 5) == 0);
    muring_consume(&ctx, b, 5);

    ok = muring_reserve(&ctx, &rs, 4);
    assert(ok);
    memcpy(rs.buf, "hijk", 4);
    muring_commit(&ctx, &rs, 4);

    b = muring_read(&ctx, &sz);
    assert(b != NULL);
    assert(sz == 2);
    assert(memcmp(b, "fg", 2) == 0);
    muring_consume(&ctx, b, 2);

    b = muring_read(&ctx, &sz);
    assert(b != NULL);
    assert(sz == 4);
    assert(memcmp(b, "hijk", 4) == 0);
    muring_consume(&ctx, b, 4);
}

int main()
{
    test_use_full_buffer();
    test_wraparound();
#ifdef MURING_ATOMIC
    test_threaded();
#endif
}
