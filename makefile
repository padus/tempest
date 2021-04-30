#
# Project:     C/C++ Makefile
# Author:      Mirco Caramori
# Copyright:   (c) 2020 Mirco Caramori
# Repository:  https://github.com/mircolino/makefile
#
# Description: application builder
#
# Usage:       make run                         run release version build/relese/project
#              make release (or just make)      build release version build/relese/project -> bin/project
#              make debug                       build development version build/debug/project
#              make syntax FILE=./src/foo.cpp   check the syntax of $(FILE)
#              make clean                       clean or reset the building environment
#
# Environment: Linux -> gcc                     apt install build-essential gdb
#              Windows -> gcc                   install mingw-w64 and either run mingw-w64.bat or add mingw/bin to the PATH
#                                               copy mingw/bin/mingw32-make.exe to mingw/bin/make.exe
#              Windows -> msvc                  install VS Build Tools and run VsDevCmd.bat (i.e. VsDevCmd.bat -arch=x64)
#              Windows -> bash commands         using git/usr/bin
#
# Notes:       - only variables between dotted lines should need to be customized
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
#              |-- bin/<os>_<cpu>/
#              |   |
#              |    -- project (from build/release)
#              |
#               -- build/<os>_<cpu>/
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
# Misc
#
define NEWLINE


endef

HASH := \#

#
# Variables
#

#-------------------------------------------------------------------------------------------------------------------------------
PROJECT := tempest
PRECOMP := system

ifeq ($(OS),Windows_NT)
  UNAME := /dev/git/usr/bin/uname
  TR    := /dev/git/usr/bin/tr
  ECHO  := /dev/git/usr/bin/echo
  RM    := /dev/git/usr/bin/rm
  CP    := /dev/git/usr/bin/cp
  MKDIR := /dev/git/usr/bin/mkdir

  OS    := windows
  CC    := msvc
else
  UNAME := uname
  TR    := tr
  ECHO  := echo
  RM    := rm
  CP    := cp
  MKDIR := mkdir

  OS    := $(shell $(UNAME) -s | $(TR) A-Z a-z)
  CC    := gcc
endif
#-------------------------------------------------------------------------------------------------------------------------------

CPU := $(shell $(UNAME) -m | $(TR) A-Z a-z)

HDR_EXT := .hpp
SRC_EXT := .cpp

ifeq ($(CC),msvc)
  PCH_EXT := .pch
else
  PCH_EXT := $(HDR_EXT).gch
endif

ifeq ($(OS),windows)
  OBJ_EXT := .obj
  EXE_EXT := .exe
else
  OBJ_EXT := .o
  EXE_EXT :=
endif

SRC_DIR := src
BIN_DIR := bin/$(OS)_$(CPU)
REL_DIR := build/$(OS)_$(CPU)/release
DBG_DIR := build/$(OS)_$(CPU)/debug
HDR_LST := $(sort $(call rwildcard,$(SRC_DIR),*$(HDR_EXT)))
SRC_LST := $(sort $(call rwildcard,$(SRC_DIR),*$(SRC_EXT)))
DIR_LST := $(patsubst %/,%,$(dir $(SRC_LST)))
REL_LST := $(sort $(REL_DIR) $(patsubst $(SRC_DIR)%,$(REL_DIR)%,$(DIR_LST)))
DBG_LST := $(sort $(DBG_DIR) $(patsubst $(SRC_DIR)%,$(DBG_DIR)%,$(DIR_LST)))

ifeq ($(CC),msvc)
  #
  # cl options:   https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category
  #               https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically
  # link options: https://docs.microsoft.com/en-us/cpp/build/reference/linker-options
  #
  #-----------------------------------------------------------------------------------------------------------------------------
  REL_CFL := -std:c++17 -TP -EHsc -nologo -O2 -I$(SRC_DIR) -DNDEBUG
  REL_LFL := -nologo

  DBG_CFL := -std:c++17 -TP -EHsc -nologo -Zi -Wall -I$(DBG_DIR) -I$(SRC_DIR) 
  DBG_LFL := -nologo -debug
  #-----------------------------------------------------------------------------------------------------------------------------

  REL_CMP  = cl $(REL_CFL) -c -Fo$@ $<
  REL_LNK  = link $(REL_LFL) -out:$@ $^

  DBG_PCH  = $(ECHO) "$(HASH)include <$(PRECOMP)$(HDR_EXT)>" > $(DBG_DIR)/$(PRECOMP)$(SRC_EXT)$(NEWLINE) \
             cl $(DBG_CFL) -Yc$(PRECOMP)$(HDR_EXT) -Fd$(DBG_DIR)/ -Fp$(DBG_DIR)/$(PRECOMP)$(PCH_EXT) -Fo$(DBG_DIR)/$(PRECOMP)$(OBJ_EXT) -c $(DBG_DIR)/$(PRECOMP)$(SRC_EXT)$(NEWLINE) \
             $(RM) -f $(DBG_DIR)/$(PRECOMP)$(SRC_EXT)
  DBG_SYN  = cl $(DBG_CFL) -Yu$(PRECOMP)$(HDR_EXT) -Fd$(DBG_DIR)/ -Fp$(DBG_DIR)/$(PRECOMP)$(PCH_EXT) -Zs $(FILE)
  DBG_CMP  = cl $(DBG_CFL) -Yu$(PRECOMP)$(HDR_EXT) -Fd$(DBG_DIR)/ -Fp$(DBG_DIR)/$(PRECOMP)$(PCH_EXT) -Fo$@ -c $<
  DBG_LNK  = link $(DBG_LFL) -out:$@ $^ $(DBG_DIR)/$(PRECOMP)$(OBJ_EXT)
else
  #
  # gcc/g++ options: https://gcc.gnu.org/onlinedocs/gcc/Invoking-GCC.html
  #
  #-----------------------------------------------------------------------------------------------------------------------------
  REL_CFL := -std=c++17 -pthread -O3 -I$(SRC_DIR) -DNDEBUG
  REL_LFL := -pthread -lcurl

  DBG_CFL := -std=c++17 -pthread -ggdb -I$(DBG_DIR) -I$(SRC_DIR)
  DBG_LFL := -pthread -lcurl
  #-----------------------------------------------------------------------------------------------------------------------------

  REL_CMP  = g++ $(REL_CFL) -c $< -o $@
  REL_LNK  = g++ $(REL_LFL) $^ -o $@

  DBG_PCH  = g++ $(DBG_CFL) -x c++-header $< -o $@
  DBG_SYN  = g++ $(DBG_CFL) -fsyntax-only $(FILE)
  DBG_CMP  = g++ $(DBG_CFL) -c $< -o $@
  DBG_LNK  = g++ $(DBG_LFL) $^ -o $@
endif

#
# Dependencies & Tasks
#
.PHONY: all run release debug syntax clean info

# default build
all: release

# release: runs
run: $(REL_DIR)/$(PROJECT)$(EXE_EXT)
	$<

# release: build
release: $(BIN_DIR)/$(PROJECT)$(EXE_EXT)

# release: copy release exe to bin folder
$(BIN_DIR)/$(PROJECT)$(EXE_EXT): $(REL_DIR)/$(PROJECT)$(EXE_EXT) | $(BIN_DIR)
	$(CP) -f $< $@

# release: link
$(REL_DIR)/$(PROJECT)$(EXE_EXT): $(patsubst $(SRC_DIR)/%$(SRC_EXT),$(REL_DIR)/%$(OBJ_EXT),$(SRC_LST)) | $(REL_DIR)
	$(REL_LNK)

# release: compile
$(REL_DIR)/%$(OBJ_EXT): $(SRC_DIR)/%$(SRC_EXT) $(HDR_LST) | $(REL_LST)
	$(REL_CMP)

# debug: build
debug: $(DBG_DIR)/$(PROJECT)$(EXE_EXT)

# debug: link
$(DBG_DIR)/$(PROJECT)$(EXE_EXT): $(patsubst $(SRC_DIR)/%$(SRC_EXT),$(DBG_DIR)/%$(OBJ_EXT),$(SRC_LST)) | $(DBG_DIR)
	$(DBG_LNK)

# debug: compile
$(DBG_DIR)/%$(OBJ_EXT): $(SRC_DIR)/%$(SRC_EXT) $(HDR_LST) $(DBG_DIR)/$(PRECOMP)$(PCH_EXT) | $(DBG_LST)
	$(DBG_CMP)

# syntax test only
syntax: $(HDR_LST) $(DBG_DIR)/$(PRECOMP)$(PCH_EXT) | $(DBG_DIR)
	$(DBG_SYN)

# debug: precomp
$(DBG_DIR)/$(PRECOMP)$(PCH_EXT): $(SRC_DIR)/$(PRECOMP)$(HDR_EXT) | $(DBG_DIR)
	$(DBG_PCH)

# clean or reset build
clean: | $(REL_DIR) $(DBG_DIR) $(BIN_DIR)
	$(RM) -fr $(REL_DIR)/* $(DBG_DIR)/* $(BIN_DIR)/*

# directory factory
$(REL_LST) $(DBG_LST) $(BIN_DIR):
	$(MKDIR) -p $@

# makefile debug helper
info:
	$(info $(HDR_LST))
	$(info $(SRC_LST))
	$(info $(DIR_LST))
	$(info $(REL_LST))
	$(info $(DBG_LST))
	$(info $(OS))

#
# Recycle Bin
#

# $(warning OS=$(OS))

# EOF
