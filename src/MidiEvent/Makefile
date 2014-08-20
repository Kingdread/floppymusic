CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(addprefix ../../obj/MidiEvent_,$(CPP_FILES:.cpp=.o))

all: $(OBJ_FILES)

../../obj/MidiEvent_%.o: %.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<
