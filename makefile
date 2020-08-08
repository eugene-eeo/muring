tests:
	gcc ring_buffer.c test.c -o ./test
	gcc ring_buffer.c test.c -DRB_ATOMIC -lpthread -o ./test_atomic
	./test
	./test_atomic
