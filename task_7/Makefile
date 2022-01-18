CC = gcc
PROGRAM_TARGET = program
PROGRAM_SOURCE = $(PROGRAM_TARGET).c
OUTPUT = output.mkv

all:
	$(CC) -o $(PROGRAM_TARGET) $(PROGRAM_SOURCE) -lavformat -lavcodec -lavutil -lz

clean:
	rm $(PROGRAM_TARGET) $(OUTPUT)

