## `muring`

Simple FIFO ring buffer that allocates contiguous slices
of memory, similar to a [BipBuffer](https://www.codeproject.com/Articles/3479/The-Bip-Buffer-The-Circular-Buffer-with-a-Twist),
and inspired by [this article](https://andrea.lattuada.me/blog/2019/the-design-and-implementation-of-a-lock-free-ring-buffer-with-contiguous-reservations.html).
The API is very simple:

    void     muring_init   (muring_buffer* ctx, uint8_t* mem, size_t size);
    int      muring_reserve(muring_buffer* ctx, muring_reservation* rs, size_t size);
    void     muring_commit (muring_buffer* ctx, muring_reservation* rs, size_t size);
    uint8_t* muring_read   (muring_buffer* ctx, size_t* size);
    void     muring_consume(muring_buffer* ctx, uint8_t* ptr, size_t size);

To write a into the buffer, you first need to `reserve`
some amount of space; this gives you a pointer that you
can write to. You can advertise to IO functions (e.g.
`read`) that you have that much space for use.

After writing, you then need to `commit` the actual amount
of data you wrote. A `commit` of size 0 is a no-op.
**Note:** you cannot nest `reserve` and `commit` calls;
essentially only the _last_ `reserve` has any real effect.

When reading from the buffer, first call `read` to get a
contiguous slice of memory. Then you should call `consume`
to inform the buffer of your reading progress.

There are two modes:

1. Non-thread safe (default)
2. Lock-free mode (single producer and single consumer,
   needs to be compiled with `-DMURING_ATOMIC`)

       $ gcc ... muring.c -DMURING_ATOMIC

   In lock-free mode, you should allocate ~(2n + 1) bytes
   for the buffer, where n is the size of the largest chunk
   you want to allocate.

Example usage:

    // First initialise the buffer
    uint8_t* buffer = malloc(size);
    muring_buffer ctx;
    muring_init(&ctx, buffer, size);

    // Get a contiguous slice of memory
    rb_reservation rs;
    if (!muring_reserve(&ctx, &rs, 100)) {
        // Handle error (not enough memory)
    }
    // Use rs.buf here: the committed size should
    // be <= the allocated size, e.g.:
    size_t n = fread(rs.buf, 1, 100, fp);
    muring_commit(&ctx, &rs, n);

    // Get buffer unread size
    size_t total = muring_total(&ctx);
    assert(total == n);

    // Read from buffer (FIFO)
    size_t sz;
    uint8_t* slice = muring_read(&ctx, &sz);
    if (slice == NULL) {
        // Handle error
    }
    // sz contains size of slice
    // inform buffer of read progress
    muring_consume(&ctx, slice, n);

Spin waiting example:

    // Wait until we can write 100 bytes into the buffer
    while (!muring_reserve(&ctx, &rs, size)) {
    }
    // use rs.buf here

    // Wait until we can read 100 bytes from buffer
    size_t sz;
    uint8_t* b;
    while ((b = muring_read(&ctx, &sz)) == NULL || sz < size) {
    }
    // use b here
    muring_consume(...);
