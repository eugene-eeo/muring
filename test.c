#include <assert.h>
#include <string.h> // for memcpy
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "ring_buffer.h"

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
}
