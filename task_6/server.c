#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

int g_thread_mode = 0, g_process_mode = 0;

const struct option long_options[] = {
	{ "pthreads", no_argument, &g_thread_mode, 1 },
	{ "process", no_argument, &g_process_mode, 1 },
	{ NULL, 0, NULL, 0}
};

int parse_args(int argc, char **argv) {
	char arg = 0;
	while ((arg = getopt_long_only(argc, argv, "", long_options, NULL)) != -1) {
		if(arg == '?') return -1;
	};
	return 0;
}

int main(int argc, char **argv) {
	if(parse_args(argc,argv)) return -1;
	printf("Parsing succesful!\n");
	if(g_thread_mode) printf("Thread\n");
	if(g_process_mode) printf("Process\n");
	return 0;
}
