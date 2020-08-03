## `rb_buffer`

Simple ring-buffer similar in design to a [BipBuffer](https://www.codeproject.com/Articles/3479/The-Bip-Buffer-The-Circular-Buffer-with-a-Twist).
The API is very simple (it is not thread safe):

```c
void   rb_buffer_init   (rb_buffer* rb, void* mem, size_t size);
void*  rb_buffer_reserve(rb_buffer* rb, size_t size);
void   rb_buffer_commit (rb_buffer* rb, void* ptr);
void*  rb_buffer_read   (rb_buffer* rb, size_t* m, size_t n);
size_t rb_buffer_total  (rb_buffer* rb);
```

To write something into the buffer, you first have to
_reserve_ space -- and if it is available in the buffer,
the buffer will give you a contiguous slice of memory.

After using that slice, you then need to _commit_ the
actual amount of data you wrote -- this usage pattern
is ~inspired by~ taken from the original BipBuffer API.

**Note:** you cannot nest `reserve` or `commit` functions;
a reserve inside another reserve may return the same location
in memory!

More detailed examples:
Need to first initialise the ring buffer:

```c
size_t sz = 512;
void* buf = malloc(sz);
rb_buffer rb;
rb_buffer_init(&rb, buf, sz);
```

Write:

```c
// size should be <= 512
void* block = rb_buffer_reserve(&rb, size);
if (block == NULL) {
    // handle error (not enough space)
}
// Use block here. The pointer passed to commit
// should be <= block + size, e.g.:
actual_size = fread(block, 1, size, fp);
rb_buffer_commit(&rb, block + actual_size);
```

Read (up to `max_size` bytes):

```c
size_t sz;
void* block;
block = rb_buffer_read(block, *sz, max_size);
if (block != NULL) {
    // sz <= max_size
    // do your thing
}
```
