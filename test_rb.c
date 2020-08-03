#include "ring_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main()
{
    void* buf = malloc(8);
    if (buf == NULL) {
        printf("buf == NULL\n");
        exit(1);
    }

    rb_buffer rb;
    rb_buffer_init(&rb, buf, 8);

    size_t sz;
    void* b;

    // Check that rb->r is reset
    char* c = (char*) rb_buffer_reserve(&rb, 6);
    assert(c != NULL);
    rb_buffer_commit(&rb, 6);

    c = (char*) rb_buffer_reserve(&rb, 2);
    assert(c != NULL);
    rb_buffer_commit(&rb, 2);

    c = (char*) rb_buffer_read(&rb, &sz, rb.size);
    assert(c != 0);
    assert(sz == 8);
    printf("%p\n", rb.mem);
    printf("%p\n", rb.r);
    printf("%p\n", rb.w);

    c = (char*) rb_buffer_reserve(&rb, 6);
    assert(c != NULL);
    c[0] = 1;
    c[1] = 2;
    c[2] = 3;
    c[3] = 4;
    c[4] = 5;
    c[5] = 6;
    rb_buffer_commit(&rb, 6);
    assert(rb_buffer_total(&rb) == 6);

    c = (char*) rb_buffer_read(&rb, &sz, 5);
    assert(sz == 5);
    assert(c != NULL);
    assert(c[0] == 1);
    assert(c[1] == 2);
    assert(c[2] == 3);
    assert(c[3] == 4);
    assert(c[4] == 5);
    assert(rb_buffer_total(&rb) == 1);

    c = (char*) rb_buffer_reserve(&rb, 4);
    assert(c != NULL);
    c[0] = 7;
    c[1] = 8;
    c[2] = 9;
    c[3] = 10;
    rb_buffer_commit(&rb, 4);
    assert(rb_buffer_total(&rb) == 5);

    for (int i = 6; i < 10 + 1; i++) {
        c = (char*)rb_buffer_read(&rb, &sz, 1);
        assert(sz == 1);
        assert(c[0] == i);
    }

    b = rb_buffer_read(&rb, &sz, rb.size);
    assert(b == NULL);
    assert(sz == 0);
}
