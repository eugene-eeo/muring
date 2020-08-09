#include <assert.h>
#include <string.h> // for memcpy
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "rb_buffer.h"

#ifdef RB_ATOMIC
#include <pthread.h>

void* read_thread(void* vargp)
{
    uint8_t* b;
    size_t sz;
    rb_buffer* rb = (rb_buffer*) vargp;
    // spin on this
    while ((b = rb_buffer_read(rb, &sz)) == NULL || sz < 5) {
    }
    assert(sz == 6);
    assert(memcmp(b, "abcde", 5) == 0);
    rb_buffer_consume(rb, b, 5);

    // next one should be immediate
    b = rb_buffer_read(rb, &sz);
    assert(b != NULL);
    assert(sz == 1);
    assert(b[0] == 'f');
    rb_buffer_consume(rb, b, 1);

    // spin on this
    while ((b = rb_buffer_read(rb, &sz)) == NULL || sz < 4) {
    }
    assert(sz == 4);
    assert(memcmp(b, "ghij", 4) == 0);
    rb_buffer_consume(rb, b, 4);
}

void* write_thread(void* vargp)
{
    rb_reservation rs;
    rb_buffer* rb = (rb_buffer*) vargp;
    int ok = rb_buffer_reserve(rb, &rs, 6);
    assert(ok);
    memcpy(rs.buf, "abcdef", 6);
    rb_buffer_commit(rb, &rs, 6);

    // wait for chance to wraparound
    while (!rb_buffer_reserve(rb, &rs, 4)) {
    }

    memcpy(rs.buf, "ghij", 4);
    rb_buffer_commit(rb, &rs, 4);
}

static
void test_threaded()
{
    uint8_t buf[8];
    rb_buffer rb;
    rb_buffer_init(&rb, buf, sizeof(buf));
    pthread_t r, w;
    pthread_create(&r, NULL, read_thread,  (void*) &rb);
    pthread_create(&w, NULL, write_thread, (void*) &rb);
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
    rb_reservation rs;
    rb_buffer rb;
    rb_buffer_init(&rb, buf, sizeof(buf));

    ok = rb_buffer_reserve(&rb, &rs, 5);
    assert(ok);
    memcpy(rs.buf, "abcde", 5);
    rb_buffer_commit(&rb, &rs, 5);

    ok = rb_buffer_reserve(&rb, &rs, 3);
    assert(ok);
    memcpy(rs.buf, "fgh", 3);
    rb_buffer_commit(&rb, &rs, 3);

    b = rb_buffer_read(&rb, &sz);
    assert(b != NULL);
    assert(sz == 8);
    assert(memcmp(b, "abcdefgh", 8) == 0);
    rb_buffer_consume(&rb, b, 5);

    b = rb_buffer_read(&rb, &sz);
    assert(b != NULL);
    assert(sz == 3);
    assert(memcmp(b, "fgh", 3) == 0);
    rb_buffer_consume(&rb, b, 3);

    b = rb_buffer_read(&rb, &sz);
    assert(b == NULL);
    assert(sz == 0);

    ok = rb_buffer_reserve(&rb, &rs, 5);
    assert(ok);
}

static
void test_wraparound()
{
    uint8_t  buf[8];
    uint8_t* b;
    size_t   sz;
    int      ok;
    rb_reservation rs;
    rb_buffer      rb;
    rb_buffer_init(&rb, buf, sizeof(buf));

    ok = rb_buffer_reserve(&rb, &rs, 6);
    assert(ok);
    memcpy(rs.buf, "abcdef", 6);
    rb_buffer_commit(&rb, &rs, 6);

    ok = rb_buffer_reserve(&rb, &rs, 1);
    assert(ok);
    memcpy(rs.buf, "g", 1);
    rb_buffer_commit(&rb, &rs, 1);

    b = rb_buffer_read(&rb, &sz);
    assert(b != NULL);
    assert(sz == 7);
    assert(memcmp(b, "abcde", 5) == 0);
    rb_buffer_consume(&rb, b, 5);

    ok = rb_buffer_reserve(&rb, &rs, 4);
    assert(ok);
    memcpy(rs.buf, "hijk", 4);
    rb_buffer_commit(&rb, &rs, 4);

    b = rb_buffer_read(&rb, &sz);
    assert(b != NULL);
    assert(sz == 2);
    assert(memcmp(b, "fg", 2) == 0);
    rb_buffer_consume(&rb, b, 2);

    b = rb_buffer_read(&rb, &sz);
    assert(b != NULL);
    assert(sz == 4);
    assert(memcmp(b, "hijk", 4) == 0);
    rb_buffer_consume(&rb, b, 4);
}

int main()
{
    test_use_full_buffer();
    test_wraparound();
#ifdef RB_ATOMIC
    test_threaded();
#endif
}
