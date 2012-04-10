OBJDIR=obj
_OBJ=dffread.o dffwrite.o ps2native.o xboxnative.o txdread.o\
txdwrite.o renderware.o #ifpread.o colread.o
DOBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ) dffconv.o)
TOBJ1 = $(patsubst %,$(OBJDIR)/%,$(_OBJ) txdconv.o)
TOBJ2 = $(patsubst %,$(OBJDIR)/%,$(_OBJ) txdex.o)
#IOBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ) ifp.o)
#COBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ) col.o)
CC=g++
CFLAGS = -Wall -g
DFFBIN=dffconv
TXDBIN=txdconv
TXDEXBIN=txdex
#IFPBIN=ifp
#COLBIN=col

build: $(DOBJ) $(TOBJ1) $(TOBJ2) #$(IOBJ) $(COBJ)
	$(CC) $(CFLAGS) $(DOBJ) -o $(DFFBIN)
	$(CC) $(CFLAGS) $(TOBJ1) -o $(TXDBIN)
	$(CC) $(CFLAGS) $(TOBJ2) -o $(TXDEXBIN)
#	$(CC) $(CFLAGS) $(IOBJ) -o $(IFPBIN)
#	$(CC) $(CFLAGS) $(COBJ) -o $(COLBIN)

$(DOBJ):
$(TOBJ1):
$(TOBJ2):
#$(IOBJ):
#$(COBJ):
	cd src && make

clean:
	rm $(OBJDIR)/*

install:
	cp $(DFFBIN) $(HOME)/bin
	cp $(TXDBIN) $(HOME)/bin
	cp $(TXDEXBIN) $(HOME)/bin
#	cp $(IFPBIN) $(HOME)/bin
