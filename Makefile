TARGET    := chip8

CXX       := g++
CXX_FLAGS := -Wall -Wextra
CXX_LINK  := -lncursesw

SRC_DIR   := src
SRC       := $(wildcard $(SRC_DIR)/*.cpp)

INC_DIR   :=
INC       :=

OBJ_DIR   := obj
OBJ       := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEP       := $(OBJ:.o=$(OBJ_DIR)/.d)



all: build $(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $(CXX_FLAGS) $(INC) -c $< -MMD -o $@

$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	@$(CXX) $(CXX_FLAGS) $(CXX_LINK) -o $(TARGET) $^
	@echo "$(TARGET): compiled"

-include $(DEP)

.PHONY: all build clean debug release info

build:
	@mkdir -p $(OBJ_DIR)

debug: CXX_FLAGS += -g3 -fsanitize=address,undefined
debug: all

release: CXX_FLAGS += -O3
release: all

clean:
	-@rm -rf $(OBJ_DIR) $(TARGET)
	@echo "$(TARGET): cleaned"

info:
	@echo "[*] Sources:         ${SRC}         "
	@echo "[*] Objects:         ${OBJECTS}     "
	@echo "[*] Object dir:      ${OBJ_DIR}     "
	@echo "[*] Dependencies:    ${DEPENDENCIES}"
