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
TMP := temp

HPP := $(call rwildcard,$(SRC)/,*.hpp)
CPP := $(call rwildcard,$(SRC)/,*.cpp)
PCH := $(TMP)/system.hpp.gch
OBJ := $(CPP:$(SRC)/%.cpp=$(TMP)/%.o)
LIB := -lstdc++fs -lcurl
EXE := $(BIN)/tempest

CXX_RELEASE := g++ -std=c++17 -pthread -I$(SRC) -DTEMPEST_RELEASE -O3
CXX_DEBUG := g++ -std=c++17 -pthread -I$(TMP) -I$(SRC) -DTEMPEST_DEBUG -ggdb

# Dependencies & Tasks

.PRECIOUS: $(PCH)

all: release

release: $(EXE)

$(EXE): $(CPP) $(HPP)
	$(CXX_RELEASE) $(CPP) $(LIB) -o $@

debug: $(EXE)d

$(EXE)d: $(OBJ)
	$(CXX_DEBUG) $(OBJ) $(LIB) -o $@ 

$(TMP)/%.o: $(SRC)/%.cpp $(HPP) $(PCH)
	$(CXX_DEBUG) -c $< -o $@

$(TMP)/%.hpp.gch: $(SRC)/%.hpp
	$(CXX_DEBUG) -x c++-header $< -o $@
	
clean:
	$(RM) $(BIN)/** $(TMP)/**								

tree:
	mkdir $(BIN) $(TMP)

info:
	$(info $(HPP))
	$(info $(CPP))
	$(info $(PCH))
	$(info $(OBJ))
	$(info $(EXE))
