## `rb_buffer`

Simple FIFO ring buffer that allocates contiguous slices
of memory, similar to a [BipBuffer](https://www.codeproject.com/Articles/3479/The-Bip-Buffer-The-Circular-Buffer-with-a-Twist),
and inspired by [this article](https://andrea.lattuada.me/blog/2019/the-design-and-implementation-of-a-lock-free-ring-buffer-with-contiguous-reservations.html).
The API is very simple (not thread-safe):

    void     rb_buffer_init   (rb_buffer* rb, uint8_t* mem, size_t size);
    uint8_t* rb_buffer_reserve(rb_buffer* rb, size_t size);
    void     rb_buffer_commit (rb_buffer* rb, uint8_t* ptr, size_t size);
    uint8_t* rb_buffer_read   (rb_buffer* rb, size_t* actual_size, size_t max_size);
    size_t   rb_buffer_total  (rb_buffer* rb);

To write a into the buffer, you first need to `reserve`
some amount of space; this gives you a pointer that you
can write to. You can advertise to IO functions (e.g.
`read`) that you have that much space for use.

After writing, you then need to `commit` the actual amount
of data you wrote. A `commit` of size 0 is a no-op.
**Note:** you cannot nest `reserve` and `commit` calls;
essentially only the _last_ `reserve` has any real effect.

Example usage:

    // First initialise the buffer
    uint8_t* buffer = malloc(size);
    rb_buffer rb;
    rb_buffer_init(&rb, buffer, size);

    // Get a contiguous slice of memory
    uint8_t* slice = rb_buffer_reserve(&rb, 100);
    if (slice == NULL) {
        // Handle error (not enough memory)
    }
    // Use slice here: the committed size should
    // be <= the allocated size, e.g.:
    size_t n = fread(slice, 1, 100, fp);
    rb_buffer_commit(&rb, slice, n);

    // Get buffer unread size
    size_t total = rb_buffer_total(&rb);
    assert(total == n);

    // Read max of 100 bytes from buffer (FIFO)
    size_t sz;
    uint8_t* slice = rb_buffer_read(&rb, *sz, 100);
    if (slice != NULL) {
        // do your thing
    }
