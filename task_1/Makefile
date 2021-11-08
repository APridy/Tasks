all:  
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./libhello.so
	gcc -c -fPIC libhello.c
	gcc -shared -o libhello.so libhello.o
	gcc -L./ -o hello hello.c -lhello

clean:
	rm hello *.o *.so
libs:
	gcc -c -fPIC libhello.c
	gcc -shared -o libhello.so libhello.o
