OBJDIR=obj
_OBJ=dffconv.o txdconv.o txdex.o dumprwtree.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
CC=g++ 
CFLAGS = -Wall -g -O3 -L. -Wl,-Bstatic -lrwtools -Wl,-Bdynamic
build: $(OBJ) librwtools.a
	$(CC) -o dffconv obj/dffconv.o $(CFLAGS)
	$(CC) -o txdconv obj/txdconv.o $(CFLAGS)
	$(CC) -o txdex obj/txdex.o $(CFLAGS)
	$(CC) -o dumprwtree obj/dumprwtree.o $(CFLAGS)

librwtools.a:
	cd src && make

clean:
	rm $(OBJDIR)/*
	rm librwtools.a

install:
	cp dffconv $(HOME)/bin
	cp txdconv $(HOME)/bin
	cp txdex $(HOME)/bin
	cp dumprwtree $(HOME)/bin
