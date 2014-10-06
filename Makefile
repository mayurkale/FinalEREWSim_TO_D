
IDIR =-I/usr/local/includeinclude
CC=gcc
CFLAGS=-static -g
LIBS=-lrt -lm -lpthread -lgsl -lgslcblas -lm
DEPS =align_sort.h applications.h apps.h connections.h dallocate.h global.h new_pmu_or_pdc.h parser.h
OBJ =new_pmu_or_pdc.o parser.o connections.o align_sort.o applications.o apps.o dallocate.o 
CFILE =new_pmu_or_pdc.c parser.c connections.c align_sort.c applications.c apps.c dallocate.c
MCFILE =ipdc.c
all:make

$(OBJ):$(DEPS) $(CFILE)
	$(CC) -c $(CFILE)

make: $(OBJ)	
	$(CC) $(CFLAGS) $(MCFILE) $(OBJ) -o iPDC $(LIBS)  

.PHONY: clean

clean:
	rm *.o iPDC 