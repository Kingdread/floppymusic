#ifndef FM_MIDIREADER_HPP
#define FM_MIDIREADER_HPP

#include "Midi.hpp"
#include <istream>
#include <set>
#include <vector>

class MidiReader
{
    private:
    bool m_valid;
    bool read(std::istream &inp);
    bool read_track(int t_nr, std::istream &inp);

    public:
    MidiHeader m_header;
    std::vector<MidiTrack> m_tracks;

    MidiReader(std::istream &inp);

    bool isValid();
    MidiTrack mergedTracks(std::set<int> muted = std::set<int>());
};

#endif
