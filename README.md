## `rb_buffer`

Simple ring-buffer similar in design to a [BipBuffer](https://www.codeproject.com/Articles/3479/The-Bip-Buffer-The-Circular-Buffer-with-a-Twist),
and inspired by [this article](https://andrea.lattuada.me/blog/2019/the-design-and-implementation-of-a-lock-free-ring-buffer-with-contiguous-reservations.html).
The API is very simple (not thread safe):

```c
void     rb_buffer_init   (rb_buffer* rb, uint8_t* mem, size_t size);
uint8_t* rb_buffer_reserve(rb_buffer* rb, size_t size);
void     rb_buffer_commit (rb_buffer* rb, uint8_t* ptr, size_t size);
uint8_t* rb_buffer_read   (rb_buffer* rb, size_t* actual_size, size_t max_size);
size_t   rb_buffer_total  (rb_buffer* rb);
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

### Examples

Need to first initialise the ring buffer:

```c
size_t sz = 512;
uint8_t* buf = malloc(sz);
rb_buffer rb;
rb_buffer_init(&rb, buf, sz);
```

Write:
**Note:** if `commit` is not called, then the buffer acts
as if nothing ever happened. You do not have to `commit`
if the piece of memory isn't used successfully.

```c
// size should be <= 512
uint8_t* block = rb_buffer_reserve(&rb, size);
if (block == NULL) {
    // handle error (not enough space)
}
// Use block here, e.g.:
actual_size = fread(block, 1, size, fp);
rb_buffer_commit(&rb, block, actual_size);
```

Read (up to `max_size` bytes):

```c
size_t sz;
uint8_t* block;
block = rb_buffer_read(&rb, *sz, max_size);
if (block != NULL) {
    // sz <= max_size
    // do your thing
}
```

To empty the buffer:

```c
size_t sz;
uint8_t* block;
while ((block = rb_buffer_read(&rb, *sz, rb.size)) != NULL) {
    // do your thing here
}
```
