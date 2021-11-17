CC = gcc
TARGET = hello
SOURCE = hello.c 
LIBARCHIVE = lib.a
LIBSOURCES = libhello.c libgoodbye.c
LIBSOURCESDIR = ./lib/
LIBOBJECTS = $(patsubst %.c,%.o,$(LIBSOURCES))

all: $(TARGET)

clean:
	rm $(TARGET) $(LIBOBJECTS) $(LIBARCHIVE)

$(TARGET): libs
	$(CC) -o $(TARGET) $(SOURCE) $(LIBARCHIVE)

libs: $(LIBARCHIVE)

$(LIBARCHIVE): $(LIBOBJECTS)
	ar ruv $@ $^
	ranlib $@

%.o : $(LIBSOURCESDIR)%.c
	$(CC) -c $^
