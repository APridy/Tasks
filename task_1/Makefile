CC = gcc

TARGET = hello

SOURCE = hello.c 

LIBARCHIVE = lib.a

LIBSOURCES = ./lib/*.c

LIBOBJECTS = ./*.o

all: $(TARGET)

clean:
	rm $(TARGET) $(LIBOBJECTS) $(LIBARCHIVE)

$(TARGET): libs
	$(CC) -o $(TARGET) $(SOURCE) $(LIBARCHIVE)

libs: $(LIBARCHIVE)

$(LIBARCHIVE): $(LIBOBJECTS)
	ar ruv $@ $^
	ranlib $@

$(LIBOBJECTS) : $(LIBSOURCES)
	$(CC) -c $^
