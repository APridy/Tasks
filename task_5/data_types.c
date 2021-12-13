#define QUEUE_KEY 252525
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

struct message {
	long mtype;
	uint8_t msg[sizeof(union data)];
};

char *g_data_type[NUM_OF_DATA_TYPES] = {
	"Integer",
	"Char array",
	"Structure with 3 integers"
};

char *g_filename[NUM_OF_DATA_TYPES] = {
	"int.txt", 
	"array.txt", 
	"struct.txt"
};
