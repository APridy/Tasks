#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define PATH_TO_FILE "./resources/mando_test.mkv"

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

	AVFormatContext *pFormatContext = avformat_alloc_context();
	avformat_open_input(&pFormatContext, PATH_TO_FILE, NULL, NULL);
	printf("Format %s, duration %ld us\n", pFormatContext->iformat->long_name, pFormatContext->duration);

	return 0;
}
