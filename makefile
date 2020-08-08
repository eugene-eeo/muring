tests:
	gcc ring_buffer_atomic.c test.c -o ./test
	gcc ring_buffer_atomic.c test.c -DRB_ATOMIC -o ./test_atomic
	valgrind -s ./test
	valgrind -s ./test_atomic
