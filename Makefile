CC=g++

CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS := -pthread
CC_FLAGS := -g -Wno-unused-parameter -Wall -Wextra

all: floppymusic

clean:
	rm -v obj/*.o
	rm -v floppymusic

floppymusic: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o $@ $^

obj/%.o: src/%.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<
