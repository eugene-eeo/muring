compile:
	gcc -march=native -O3 muring.c test.c -o ./test
	gcc -march=native -O3 muring.c test.c -DMURING_ATOMIC -lpthread -o ./test_atomic

tests: compile
	./test
	./test_atomic
