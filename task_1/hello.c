#include <stdio.h>
#include "dynlibs/libhello.h"
#include "stlibs/libgoodbye.h"


int main() {

	printf("%s",hello_world);
	printf("%s",goodbye_world);
	return 0;
	
}
