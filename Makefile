SRCDIR = src
BUILDDIR = build
BINDIR = bin
INCDIR = include
LIBDIR = lib
SRC := $(patsubst %.cpp,$(SRCDIR)/%.cpp,dffread.cpp dffwrite.cpp\
  ps2native.cpp xboxnative.cpp oglnative.cpp\
  txdread.cpp txdwrite.cpp renderware.cpp)
SRC2 := $(patsubst %.cpp,$(SRCDIR)/%.cpp,\
  dffconv.cpp txdconv.cpp txdex.cpp dumprwtree.cpp)
OBJ := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
OBJ2 := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC2))
DEP := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.d,$(SRC) $(SRC2))
LIB = $(LIBDIR)/librwtools.a
BIN = $(patsubst $(BUILDDIR)/%.o,%,$(OBJ2))
CFLAGS = -I$(INCDIR) -Wall -Wextra -g -O3 -pg -DDEBUG
LINK = $(LIB) 

all: $(LIB) bins

bins: $(LIB) $(OBJ2)
	$(foreach bin,$(BIN),$(CXX) -o $(BINDIR)/$(bin) $(BUILDDIR)/$(bin).o $(LINK);)

$(LIB): $(OBJ)
	ar scr $(LIB) $(OBJ)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: $(SRCDIR)/%.cpp
	$(CXX) -MM -MT '$(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$<)' $(CFLAGS) $< > $@

clean:
	rm -f build/* lib/* bin/*

install: $(patsubst %,$(BINDIR)/%,$(BIN))
	cp $(patsubst %,$(BINDIR)/%,$(BIN)) $(HOME)/bin

-include $(DEP)
