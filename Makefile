SRCDIR = src
BUILDDIR = build
SRC := $(patsubst %.cpp,$(SRCDIR)/%.cpp,dffread.cpp dffwrite.cpp\
  ps2native.cpp xboxnative.cpp txdread.cpp txdwrite.cpp renderware.cpp)
SRC2 := $(patsubst %.cpp,$(SRCDIR)/%.cpp,\
  dffconv.cpp txdconv.cpp txdex.cpp dumprwtree.cpp)
OBJ := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
OBJ2 := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC2))
DEP := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.d,$(SRC) $(SRC2))
LIB = librwtools.a
BIN = $(patsubst $(BUILDDIR)/%.o,%,$(OBJ2))
CFLAGS = $(INC) -Wall -Wextra -g -O3 -pg
LINK = $(LIB) 

all: $(LIB) bins

bins: $(LIB) $(OBJ2)
	$(foreach bin,$(BIN),$(CXX) -o $(bin) $(BUILDDIR)/$(bin).o $(LINK);)

$(LIB): $(OBJ)
	ar scr $(LIB) $(OBJ)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: $(SRCDIR)/%.cpp
	$(CXX) -MM -MT '$(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$<)' $(CFLAGS) $< > $@

clean:
	rm -f build/* $(LIB) $(BIN)

install: $(BIN)
	cp $(BIN) $(HOME)/bin

-include $(DEP)
