#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>
#include <unistd.h>

unsigned int g_interval[2] = {0 , 0};

int parse_args(int argc, char **argv) {
	if (argc > 3) return -1;
	if (argc > 1) {
		if(!(g_interval[0] = atoi(argv[1]))) return -1;
	}
	if (argc > 2) {
		if(!(g_interval[1] = atoi(argv[2]))) return -1;
	}
	return 0;
}

int main(int argc, char **argv) {
	if (parse_args(argc, argv)) {
		printf("Invalid arguments! Try ./program <interval_start> <interval_end>\n");
		return -1;
	}
	printf("%d\n%d\n", g_interval[0], g_interval[1]);
	return 0;
}
