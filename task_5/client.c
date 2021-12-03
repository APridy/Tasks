#include <stdio.h>
#include <unistd.h>

#define NUM_OF_DATA_TYPES 3

struct mystruct {
	int a;
	int b;
	int c;
};

union data {
	int num;
	char arr[5];
	struct mystruct num3;
};

FILE *fp[NUM_OF_DATA_TYPES];
char *filename[NUM_OF_DATA_TYPES] = {"int.txt", "array.txt", "struct.txt"};

int main(int argc, char **argv) {

	char rez = 0;
	while ((rez = getopt(argc,argv,"f:")) != -1) {
		switch (rez) {
                        case 'f':
				printf("%s\n",optarg);
                                break;
                        case '?':
                                printf("Invalid argument!\n");
                                return 1;
                };
        };

}
