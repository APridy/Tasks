CC = gcc
TARGET = hello
SOURCE = hello.c 
STLIB = stlib.a
DYNLIB = dyn
DYNLIB_SRC = libhello.c 
STLIB_SRC = libgoodbye.c
DYNLIB_SRCDIR = ./dynlibs/
STLIB_SRCDIR = ./stlibs/
DYNLIB_OBJ = $(patsubst %.c,%.o,$(DYNLIB_SRC))
STLIB_OBJ = $(patsubst %.c,%.o,$(STLIB_SRC))

all: $(TARGET)

clean:
	rm $(TARGET) lib$(DYNLIB).so $(STLIB) $(DYNLIB_OBJ) $(STLIB_OBJ)

$(TARGET): libs
	$(CC) -L./ $(SOURCE) $(STLIB) -l$(DYNLIB) -o $(TARGET)

libs: $(STLIB) $(DYNLIB)

$(STLIB): $(STLIB_OBJ)
	ar ruv $@ $^
	ranlib $@
	
$(DYNLIB): $(DYNLIB_OBJ) 
	$(CC) -shared -o lib$(DYNLIB).so $(DYNLIB_OBJ)

%.o : $(STLIB_SRCDIR)%.c
	$(CC) -c $^

%.o : $(DYNLIB_SRCDIR)%.c
	$(CC) -c -fPIC $^
