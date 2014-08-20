#ifndef FM_MIDI_TRACK_HPP
#define FM_MIDI_TRACK_HPP

#include "MidiEvent.hpp"
#include <istream>
#include <vector>

typedef std::vector<MidiEvent*> EventList;

class MidiTrack
{
    private:
    EventList m_events;
    int m_chunk_size;

    MidiTrack(MidiTrack const &other);
    MidiTrack& operator=(MidiTrack const &other);

    public:
    MidiTrack();
    ~MidiTrack();

    void insert(MidiEvent *event);
    void calc_realtimes(int time_div, MidiTrack const *timeline);

    static MidiTrack* read_track(int t_nr, std::istream &inp);

    EventList::iterator begin();
    EventList::iterator end();
    MidiEvent* at(int index);
    int size() const;
};

#endif
