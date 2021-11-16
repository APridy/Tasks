all:
	gcc -c ./lib/*.c #compile libraries
	ar ruv lib.a *.o #archive .o files
	ranlib lib.a #create map file
	gcc -o hello hello.c lib.a #compile program

clean:
	rm hello *.o *.a

libs:
	gcc -c ./lib/*.c
	ar ruv lib.a *.o
	ranlib lib.a
