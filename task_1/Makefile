all: hello.c ./stlibs/*.c ./stlibs/*.h ./dynlibs/*.c ./dynlibs/*.h 
	gcc -c ./stlibs/*.c #compile static libraries
	ar ruv stlib.a *.o #archive .o files
	ranlib stlib.a #create map file
	rm *.o
	gcc -c -fPIC ./dynlibs/*.c #compile dynamic libraries
	gcc -shared -o libdyn.so *.o #merge them into shared library
	gcc -L./ hello.c stlib.a -ldyn -o hello #compile program
	rm *.o

clean:
	rm hello *.so *.a

libs: ./stlibs/*.c ./stlibs/*.h ./dynlibs/*.c ./dynlibs/*.h
	gcc -c ./stlibs/*.c #compile static libraries
	ar ruv stlib.a *.o #archive .o files
	ranlib stlib.a #create map file
	rm *.o
	gcc -c -fPIC ./dynlibs/*.c #compile dynamic libraries
	gcc -shared -o libdyn.so *.o #merge them into shared library
	rm *.o
