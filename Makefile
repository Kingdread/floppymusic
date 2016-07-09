CC=g++

LD_FLAGS := -pthread
CC_FLAGS := -O3 -Wno-unused-parameter -Wall -Wextra

ifeq ($(MODEL),PI2)
CC_FLAGS += -DPI_NEW_MODEL
endif

export CC
export LD_FLAGS
export CC_FLAGS

all: floppymusic

clean:
	rm -v obj/*.o
	rm -v floppymusic

floppymusic: verinfo sources events
	$(CC) $(LD_FLAGS) -o $@ $(wildcard obj/*.o)


events:
	make -C src/MidiEvent

sources:
	@mkdir -p obj/
	make -C src

verinfo: .git/HEAD .git/index
	git describe --always --dirty --abbrev=8 | awk 'BEGIN {print "#ifndef FM_VERSION"} {print "#define FM_VERSION \""$$0"\""} END {print "#endif"}' > src/version.hpp
