NAME = grmELF

DESC = GRMFuzz ELF/DWARF parsing library

SOURCE	= elf.cc dwarf.cc

TESTSRC = tests/test1.cc tests/test2.cc

OBJS = ${SOURCE:.cc=.o} ${TESTSRC:.cc=.o}

TARGET	= lib$(NAME).a

INCLUDES = -I..

CC	 = g++

SANITIZER = -fsanitize=address

FLAGS	 = -g -w -fdiagnostics-color=always --std=c++23 $(INCLUDES) $(SANITIZER) -c

LFLAGS	 = -L. -lgrmELF -L../grmUtils -lgrmUtils -lmagic -lcrypto $(SANITIZER)
# -Wl,--verbose

TESTPROG = tests/test1 tests/test2

all: $(TESTPROG)
	
tests/test1: tests/test1.o libgrmELF.a
	$(CC) -o $@ tests/test1.o $(LFLAGS)
	
tests/test2: tests/test2.o libgrmELF.a
	$(CC) -o $@ tests/test2.o $(LFLAGS)
	
tests/test1.o: tests/test1.cc
	$(CC) $(FLAGS) tests/test1.cc -o $@
	
tests/test2.o: tests/test2.cc
	$(CC) $(FLAGS) tests/test2.cc -o $@
	
libgrmELF.a: elf.o dwarf.o
	ar rcs $@ elf.o dwarf.o
	
elf.o: elf.cc elf.h
	$(CC) $(FLAGS) elf.cc -o $@
	
dwarf.o: dwarf.cc dwarf.h
	$(CC) $(FLAGS) dwarf.cc -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TESTPROG)
