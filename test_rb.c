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
    rb_buffer_commit(&rb, c, 6);

    c = (char*) rb_buffer_reserve(&rb, 2);
    assert(c != NULL);
    rb_buffer_commit(&rb, c, 2);

    c = (char*) rb_buffer_read(&rb, &sz, rb.size);
    assert(c != 0);
    assert(sz == 8);

    c = (char*) rb_buffer_reserve(&rb, 6);
    assert(c != NULL);
    for (int i = 0; i < 6; i++)
        c[i] = i + 1;
    rb_buffer_commit(&rb, c, 6);
    assert(rb_buffer_total(&rb) == 6);

    // cannot alloc any more
    assert(rb_buffer_reserve(&rb, 3) == NULL);

    printf("%ld\n", rb.r);
    printf("%ld\n", rb.h);
    printf("%ld\n", rb.w);

    c = (char*) rb_buffer_read(&rb, &sz, 5);
    printf("%ld\n", sz);
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
    assert(c == rb.mem);
    c[0] = 7;
    c[1] = 8;
    c[2] = 9;
    c[3] = 10;
    rb_buffer_commit(&rb, c, 4);
    assert(rb_buffer_total(&rb) == 5);

    for (int i = 6; i < 10 + 1; i++) {
        c = (char*)rb_buffer_read(&rb, &sz, 1);
        assert(sz == 1);
        assert(c[0] == i);
    }

    b = rb_buffer_read(&rb, &sz, rb.size);
    assert(b == NULL);
    assert(sz == 0);

    c = (char*) rb_buffer_reserve(&rb, 5);
    assert(c != NULL);
    for (int i = 0; i < 5; i++)
        c[i] = i;
    rb_buffer_commit(&rb, c, 5);

    c = (char*) rb_buffer_read(&rb, &sz, 4);
    assert(c != NULL);
    assert(sz == 4);
    for (int i = 0; i < 4; i++)
        assert(c[i] == i);

    c = (char*) rb_buffer_reserve(&rb, 3);
    assert(c != NULL);
    for (int i = 0; i < 3; i++)
        c[i] = i + 5;
    rb_buffer_commit(&rb, c, 3);
    assert(rb.w == 8);
    assert(rb.h == 8);

    c = (char*) rb_buffer_reserve(&rb, 3);
    assert(c != NULL);
    for (int i = 0; i < 3; i++)
        c[i] = i + 8;
    rb_buffer_commit(&rb, c, 3);
    assert(rb.w == 3);
    assert(rb.h == 8);

    c = (char*) rb_buffer_reserve(&rb, 1);
    assert(c == NULL);

    printf("%ld\n", rb_buffer_total(&rb));

    int i = 4;
    while ((c = (char*) rb_buffer_read(&rb, &sz, 8)) != NULL) {
        assert(sz != 0);
        for (size_t j = 0; j < sz; j++) {
            assert(c[j] == i);
            i++;
        }
    }

    free(buf);
}
