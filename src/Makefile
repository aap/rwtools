OBJDIR = ../obj
_OBJ = dffread.o dffwrite.o ps2native.o xboxnative.o txdread.o txdwrite.o\
renderware.o #ifpread.o colread.o
_OBJPROG = dffconv.o txdconv.o txdex.o dumprwtree.o #ifp.o col.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
OBJPROG = $(patsubst %,$(OBJDIR)/%,$(_OBJPROG))
HEADER = renderware.h
CC = g++
CFLAGS = -Wall -Wextra -g -DDEBUG -O3

dfftools: $(OBJ) $(OBJPROG)
	ar scr ../librwtools.a $(OBJ)

$(OBJDIR)/%.o : %.cpp $(HEADER)
	$(CC) $(CFLAGS) -c -o $@ $<

