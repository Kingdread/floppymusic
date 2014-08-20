#ifndef FM_MIDI_FILE_HPP
#define FM_MIDI_FILE_HPP

#include "MidiTrack.hpp"
#include <istream>
#include <map>
#include <set>

typedef std::vector<MidiTrack*> TrackList;

class MidiFile
{
    private:
    TrackList m_tracks;
    int m_format_type;
    int m_track_count;
    int m_time_division;

    public:
    MidiFile();
    ~MidiFile();
    bool read(std::istream &inp);

    MidiTrack* getTrack(int n);
    int getTrackCount() const;
    int getFormatType() const;

    EventList mergedTracks(std::set<int>);
};

#endif
