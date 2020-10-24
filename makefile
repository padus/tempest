#
# App:         WeatherFlow Tempest UDP Relay
# Author:      Mirco Caramori
# Copyright:   (c) 2020 Mirco Caramori
# Repository:  https://github.com/mircolino/tempest
#
# Description: application builder
#
# Usage:       make release (or just make)      build release version build/relese/project -> bin/project
#              make run                         run release version build/relese/project 
#              make debug                       build development version build/debug/project
#              make syntax FILE=./src/foo.cpp   check the syntax of $(FILE)
#              make clean                       clean or reset the building environment
#
# Notes:       - on Windows this has been tested with mingw-w64 GNU gcc/g++ and make
#              - filenames and directories cannot have spaces
#              - directories must not end with /
#              - extensions must begin with .
#
# Project:     project/
#              |
#              |-- src/
#              |   |
#              |   |-- precomp.hpp
#              |   |-- *.hpp
#              |    -- *.cpp
#              |
#              |-- bin/
#              |   |
#              |    -- project (from build/release)
#              |
#               -- build/
#                  |
#                  |-- release/
#                  |   |
#                  |   |-- *.o
#                  |    -- project
#                  |
#                   -- debug/
#                      |
#                      |-- precomp.hpp.gch
#                      |-- *.o
#                       -- project
#

#
# Functions
#
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2)$(filter $(subst *,%,$2),$d))

#
# Variables
#
PROJECT := tempest
PRECOMP := system

HDR_EXT := .hpp
SRC_EXT := .cpp
PCH_EXT := .hpp.gch

ifeq ($(OS),Windows_NT)
  OBJ_EXT := .obj
  EXE_EXT := .exe
else
  OBJ_EXT := .o
  EXE_EXT :=
endif

SRC_DIR := ./src
BIN_DIR := ./bin
REL_DIR := ./build/release
DBG_DIR := ./build/debug
HDR_LST := $(sort $(call rwildcard,$(SRC_DIR),*$(HDR_EXT)))
SRC_LST := $(sort $(call rwildcard,$(SRC_DIR),*$(SRC_EXT)))
DIR_LST := $(patsubst %/,%,$(dir $(SRC_LST)))
REL_LST := $(sort $(REL_DIR) $(patsubst $(SRC_DIR)%,$(REL_DIR)%,$(DIR_LST)))
DBG_LST := $(sort $(DBG_DIR) $(patsubst $(SRC_DIR)%,$(DBG_DIR)%,$(DIR_LST)))

REL_OPT := -std=c++17 -pthread -O3
DBG_OPT := -std=c++17 -pthread -ggdb

REL_WRN :=
DBG_WRN :=# -Wpedantic -Wall -Wextra -Weffc++

REL_INC := -I$(SRC_DIR)
DBG_INC := -I$(DBG_DIR) -I$(SRC_DIR)

REL_DEF := -DNDEBUG
DBG_DEF :=

REL_CMP := $(REL_OPT) $(REL_WRN) $(REL_INC) $(REL_DEF)
DBG_CMP := $(DBG_OPT) $(DBG_WRN) $(DBG_INC) $(DBG_DEF)

REL_LIB := -lcurl
DBG_LIB := -lcurl

REL_LNK := $(REL_CMP) $(REL_LIB)
DBG_LNK := $(DBG_CMP) $(DBG_LIB)

#
# Dependencies & Tasks
#
.PHONY: all release run debug syntax clean info

# default build
all: release

# release: build
release: $(BIN_DIR)/$(PROJECT)$(EXE_EXT)

# release: run
run: $(REL_DIR)/$(PROJECT)$(EXE_EXT)
	$<

# release: copy release exe to bin folder
$(BIN_DIR)/$(PROJECT)$(EXE_EXT): $(REL_DIR)/$(PROJECT)$(EXE_EXT) | $(BIN_DIR)
	cp -f $< $@ 

# release: link
$(REL_DIR)/$(PROJECT)$(EXE_EXT): $(patsubst $(SRC_DIR)/%$(SRC_EXT),$(REL_DIR)/%$(OBJ_EXT),$(SRC_LST)) | $(REL_DIR)
	g++ $(REL_LNK) $^ -o $@ 

# release: compile
$(REL_DIR)/%$(OBJ_EXT): $(SRC_DIR)/%$(SRC_EXT) $(HDR_LST) | $(REL_LST)
	g++ $(REL_CMP) -c $< -o $@

# debug: build
debug: $(DBG_DIR)/$(PROJECT)$(EXE_EXT)

# debug: link
$(DBG_DIR)/$(PROJECT)$(EXE_EXT): $(patsubst $(SRC_DIR)/%$(SRC_EXT),$(DBG_DIR)/%$(OBJ_EXT),$(SRC_LST)) | $(DBG_DIR)
	g++ $(DBG_LNK) $^ -o $@ 

# debug: compile
$(DBG_DIR)/%$(OBJ_EXT): $(SRC_DIR)/%$(SRC_EXT) $(HDR_LST) $(DBG_DIR)/$(PRECOMP)$(PCH_EXT) | $(DBG_LST)
	g++ $(DBG_CMP) -c $< -o $@

# debug: precomp
$(DBG_DIR)/$(PRECOMP)$(PCH_EXT): $(SRC_DIR)/$(PRECOMP)$(HDR_EXT) | $(DBG_DIR)
	g++ $(DBG_CMP) -x c++-header $< -o $@

# syntax test only
syntax: 
	g++ $(DBG_CMP) -fsyntax-only $(FILE)

# clean or reset build
clean: | $(REL_DIR) $(DBG_DIR) $(BIN_DIR)
	rm -fr $(REL_DIR)/* $(DBG_DIR)/* $(BIN_DIR)/*

# directory factory
$(REL_LST) $(DBG_LST) $(BIN_DIR):
	mkdir -p $@

# makefile debug helper
info:
	$(info $(HDR_LST))
	$(info $(SRC_LST))
	$(info $(DIR_LST))
	$(info $(REL_LST))
	$(info $(DBG_LST))

# Recycle Bin

# $(warning OS=$(OS))

# EOF
