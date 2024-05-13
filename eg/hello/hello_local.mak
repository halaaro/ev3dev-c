all: hello.o
	arm-linux-gcc hello.o -lev3dev-c -o hello

hello.o: hello.c
	arm-linux-gcc -c hello.c -I../../source/ev3/

clean:
	-rm -f *.o
	-rm -f hello
