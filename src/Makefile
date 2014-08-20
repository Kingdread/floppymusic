CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(addprefix ../obj/,$(CPP_FILES:.cpp=.o))

all: $(OBJ_FILES)

../obj/%.o: %.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<
