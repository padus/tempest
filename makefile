# Variables

CXX            := g++
CXX_FLAGS      := -std=c++17 -ggdb

BIN            := bin
SRC            := src
INC            := include

LIB            :=
LIB_FLAGS      :=

EXE            := tempest

# Tasks

all:           $(BIN)/$(EXE)

run:           clean all
	             clear
	             ./$(BIN)/$(EXE)

$(BIN)/$(EXE): $(SRC)/*.cpp
	             $(CXX) $(CXX_FLAGS) -I$(INC) $^ -o $@ $(LIB_FLAGS) $(LIB)

clean:
	             $(RM) $(BIN)/*
