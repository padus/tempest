# Variables

SRC             := src
BIN             := bin
EXE             := tempest

CXX_DEBUG       := g++ -std=c++17 -I$(SRC) -DTEMPEST_DEBUG -ggdb
CXX_RELEASE     := g++ -std=c++17 -I$(SRC) -DTEMPEST_RELEASE -O2

# Dependencies & Tasks

all:            debug

debug:          $(BIN)/$(EXE)d

release:        $(BIN)/$(EXE)

$(BIN)/$(EXE)d: $(SRC)/**
	              $(CXX_DEBUG) $(SRC)/main.cpp -o $@

$(BIN)/$(EXE):  $(SRC)/**
	              $(CXX_RELEASE) $(SRC)/main.cpp -o $@

clean:
	              $(RM) $(BIN)/**
