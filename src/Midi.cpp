#include "Midi.hpp"

#include <iostream>

int MidiTrack::delta_to_musec(int ticks)
{
    if (header.tempo == 0)
    {
        header.tempo = BPM_TO_MPQN(120); // default tempo
    }
    if ((header.time_division & 0x8000) == 0)
    {
        // time division is in ticks per quarter note, header.tempo contains
        // the microseconds per quarter note
        return (double)header.tempo * ticks / header.time_division;
    }
    return -1;
}
