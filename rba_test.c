#include "ring_buffer_atomic.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

int main()
{
    uint8_t* b;
    uint8_t* block = malloc(8);
    rb_buffer rb;
    rb_buffer_init(&rb, block, 8);

    rb_reservation rs;
    int x = rb_buffer_reserve(&rb, &rs, 5);
    assert(x);
    memcpy(rs.buf, "abcde", 5);
    rb_buffer_commit(&rb, &rs, 5);

    b = rb_buffer_read(&rb, 4);
    assert(b != NULL);
    assert(memcmp(b, "abcd", 4) == 0);
    rb_buffer_consume(&rb, b, 4);

    // check that we can use the full ring buffer
    x = rb_buffer_reserve(&rb, &rs, 3);
    assert(x);
    memcpy(rs.buf, "fgh", 3);
    rb_buffer_commit(&rb, &rs, 3);

    // read the full buffer...
    b = rb_buffer_read(&rb, 4);
    assert(b != NULL);
    assert(memcmp(b, "efgh", 4) == 0);
    rb_buffer_consume(&rb, b, 4);

    // can use buffer afterwards
    assert(rb_buffer_reserve(&rb, &rs, 6));
    assert(rs.buf != NULL);
    memcpy(rs.buf, "six_ty", 6);
    rb_buffer_commit(&rb, &rs, 6);

    b = rb_buffer_read(&rb, 5);
    assert(b != NULL);
    assert(memcmp(b, "six_t", 5) == 0);
    rb_buffer_consume(&rb, b, 5);

    // only have one byte available for reading
    assert(rb_buffer_read(&rb, 2) == NULL);

    // test wraparound
}
