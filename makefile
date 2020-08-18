compile:
	gcc -O3 muring.c test.c -o ./test
	gcc -O3 muring.c test.c -DMURING_ATOMIC -lpthread -o ./test_atomic
	# gcc -O3 diode.c  test_diode.c -lpthread -g -o ./test_diode

tests: compile
	./test
	./test_atomic
	# ./test_diode
