#
# App:         WeatherFlow Tempest UDP Relay 
# Author:      Mirco Caramori
# Copyright:   (c) 2020 Mirco Caramori
# Repository:  https://github.com/mircolino/tempest
#
# Description: application builder
#
# Usage:       make release (or just make)      build distribuition bin/tempest
#              make debug                       build development version bin/tempestd
#              make clean                       clean building environment
#              make tree                        build building directy tree structure  
#

# Functions

rwildcard = $(wildcard $1$2)$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# Variables

SRC := src
BIN := bin
BLD := build

HPP := $(call rwildcard,$(SRC)/,*.hpp)
CPP := $(call rwildcard,$(SRC)/,*.cpp)
PCH := $(BLD)/system.hpp.gch
OBJ := $(CPP:$(SRC)/%.cpp=$(BLD)/%.o)
LIB := -lstdc++fs -lcurl
EXE := $(BLD)/tempest

WRN := -Wpedantic -Wall -Wextra -Weffc++

CXX_RELEASE := g++ -std=c++17 -pthread -I$(SRC) -DNDEBUG -DTEMPEST_RELEASE -O3
CXX_DEBUG := g++ -std=c++17 -pthread -I$(BLD) -I$(SRC) -DTEMPEST_DEBUG -ggdb

# Dependencies & Tasks

.PRECIOUS: $(PCH)

all: release

release: $(EXE)

$(EXE): $(CPP) $(HPP)
	$(CXX_RELEASE) $(CPP) $(LIB) -o $@
	cp -f $(EXE) $(BIN)/

debug: $(EXE)d

$(EXE)d: $(OBJ)
	$(CXX_DEBUG) $(OBJ) $(LIB) -o $@ 

$(BLD)/%.o: $(SRC)/%.cpp $(HPP) $(PCH)
	$(CXX_DEBUG) -c $< -o $@

$(BLD)/%.hpp.gch: $(SRC)/%.hpp
	$(CXX_DEBUG) -x c++-header $< -o $@
	
clean:
	rm -fr $(BLD)/*

tree:
	mkdir -p $(BLD)

info:
	$(info $(HPP))
	$(info $(CPP))
	$(info $(PCH))
	$(info $(OBJ))
	$(info $(EXE))
