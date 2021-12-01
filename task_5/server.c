#include <stdio.h>
#include <unistd.h>


int main(int argc, char **argv) {
	
	char rez = 0;
	while ((rez = getopt(argc,argv,"Di:c:s:")) != -1) {
		switch (rez) {
			case 'D': 
				printf("Found argument \"D\".\n"); 
				break;
			case 'i': 
				printf("Found argument \"i = %s\".\n",optarg); 
				break;
			case 'c': 
				printf("Found argument \"c = %s\".\n",optarg); 
				break;
			case 's': 
				printf("Found argument \"s = %s\".\n",optarg); 
				break;
			//case '?': printf("Invalid argument!\n");
			//	break;
		};
	};

	return 0;

}
