tests:
	gcc muring.c test.c -o ./test
	gcc muring.c test.c -DMURING_ATOMIC -lpthread -o ./test_atomic
	./test
	./test_atomic
