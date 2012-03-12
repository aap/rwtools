OBJDIR = ../obj
_OBJ = dffread.o dffwrite.o ps2native.o xboxnative.o txdread.o txdwrite.o\
renderware.o ifpread.o
_OBJPROG = dffconv.o txdconv.o txdex.o ifp.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
OBJPROG = $(patsubst %,$(OBJDIR)/%,$(_OBJPROG))
HEADER = renderware.h
CC = g++
CFLAGS = -Wall -g -DDEBUG

dfftools: $(OBJ) $(OBJPROG)
	ar scr ../librwtools.a $(OBJ)

$(OBJDIR)/%.o : %.cpp $(HEADER)
	$(CC) $(CFLAGS) -c -o $@ $<

