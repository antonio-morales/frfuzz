NAME = utils

TESTSRC = 

OBJS = filesys.o utils.o error.o process.o x11.o

TARGET = lib$(NAME).a

INCLUDES = -I..

CC = g++

SANITIZER = -fsanitize=address

FLAGS = -g -O0 -fdiagnostics-color=always --std=c++23 $(INCLUDES) $(SANITIZER) -c

LFLAGS = $(SANITIZER)
# -Wl,--verbose

TESTPROG = 

all: $(TARGET)
	
$(TARGET): $(OBJS)
	ar rcs $@ $(OBJS)
	
#utils.o: utils.cc utils.h
#	$(CC) $(FLAGS) utils.cc -o $@
	
#filesys.o: filesys.cc filesys.h
#	$(CC) $(FLAGS) filesys.cc -o $@

$(OBJS): %.o: %.cc %.h
	$(CC) $(FLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TESTPROG)
