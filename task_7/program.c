#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>
#include <unistd.h>

unsigned int g_interval[2] = {0 , 0};

int parse_args(int argc, char **argv) {
	char arg = 0;
	while ((arg = getopt(argc,argv,"b:e:")) != -1) {
		switch (arg) {
			case 'b':
				g_interval[0] = atoi(optarg);
				break;
			case 'e':
				g_interval[1] = atoi(optarg);
				break;
			case '?':
				printf("Invalid argument!\n");
				return -1;
		};
	};
	return 0;

}

int main(int argc, char **argv) {
	if(parse_args(argc, argv)) return -1;	
	printf("%d\n%d\n", g_interval[0], g_interval[1]);
	return 0;
}
