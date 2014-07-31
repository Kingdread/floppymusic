#ifndef FM_MIDI_HPP
#define FM_MIDI_HPP

#include <stdint.h>
#include <string>
#include <vector>

#define MICROSECONDS_PER_MINUTE (60000000.0)
#define MPQN_TO_BPM(MPQN) (MICROSECONDS_PER_MINUTE / MPQN)
#define BPM_TO_MPQN(BPM) (MICROSECONDS_PER_MINUTE / BPM)

#define MIDI_NOTE_OFF (0x8)
#define MIDI_NOTE_ON (0x9)

struct MidiEvent
{
    int delta_time; // for the sake of simplicity we will already convert this
                    // to microseconds, otherwise we'd have to store all the
                    // tempo-meta-events also.
    short event_type;
    int channel; // 4 bits too but this is also used in a merged track which
                 // may have more than 15 channels
    unsigned char param_1;
    unsigned char param_2;
};

struct MidiHeader
{
    short format_type;
    unsigned short track_count;
    uint16_t time_division;
};

struct MidiTrackHeader
{
    uint32_t chunk_size;

    // Defined by meta events:
    uint16_t sequence_number;
    std::string sequence_name;
    unsigned long tempo;
    // leaving out the instrument names, who cares about them anyway?
    uint16_t time_division; // copied from the MidiHeader

};

struct MidiTrack
{
    MidiTrackHeader header;
    std::vector<MidiEvent> events;

    int delta_to_musec(int ticks);
};

#endif
