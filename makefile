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
EXE := $(BIN)/tempest

CXX_RELEASE := g++ -std=c++17 -I$(SRC) -DTEMPEST_RELEASE -O3
CXX_DEBUG := g++ -std=c++17 -I$(TMP) -I$(SRC) -DTEMPEST_DEBUG -ggdb

# Dependencies & Tasks

.PRECIOUS: $(PCH)

all: release

release: $(EXE)

$(EXE): $(CPP) $(HPP)
	$(CXX_RELEASE) $(CPP) -o $@ 

debug: $(EXE)d

$(EXE)d: $(OBJ)
	$(CXX_DEBUG) $(OBJ) -o $@ 

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
