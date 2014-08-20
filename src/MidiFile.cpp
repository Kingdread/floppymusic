#include "MidiFile.hpp"
#include "MidiEvents.hpp"
#include <cstring>
#include <iostream>

static const char MIDI_HEADER_ID[] = {'M', 'T', 'h', 'd', 0};

MidiFile::MidiFile()
{}


MidiFile::~MidiFile()
{
    for (TrackList::iterator track = m_tracks.begin();
            track != m_tracks.end(); ++track)
    {
        delete *track;
        *track = 0;
    }
}


/* Read the midi from the given input stream. Returns a boolean
 * indicating if the file has been succesfully read.
 */
bool MidiFile::read(std::istream &inp)
{
    unsigned char buffer[4];
    char *sbuffer = reinterpret_cast<char*>(buffer);
    // Check if this is a valid midi file
    inp.read(sbuffer, 4);
    if (std::memcmp(buffer, MIDI_HEADER_ID, 4))
    {
        std::cerr << "MIDI: Invalid midi file, wrong header id (expected "
            << MIDI_HEADER_ID << ")" << std::endl;
        return false;
    }

    // Read the chunk size
    unsigned int chunk_size;
    inp.read(sbuffer, 4); // 4 bytes for the integer 6, seriously?
    chunk_size = buffer[0] << 24
        | buffer[1] << 16
        | buffer[2] << 8
        | buffer[3];
    if (chunk_size != 6)
    {
        std::cerr << "MIDI: Invalid midi file, wrong header size ("
            << chunk_size << " instead of 6)" << std::endl;
        return false;
    }

    // Read the format type
    inp.read(sbuffer, 2);
    m_format_type = buffer[0] << 8 | buffer[1];
    switch (m_format_type)
    {
        case 0:
        case 1:
        case 2:
            break;
        default:
            std::cerr << "MIDI: Invalid format type ("
                << m_format_type << ")" << std::endl;
            return false;
    }

    // Read the number of tracks
    inp.read(sbuffer, 2);
    m_track_count = buffer[0] << 8 | buffer[1];

    // Read the time division
    inp.read(sbuffer, 2);
    m_time_division = buffer[0] << 8 | buffer[1];

    // Header completed, read the tracks
    MidiTrack *track;
    for (int t_nr = 0; t_nr < m_track_count; ++t_nr)
    {
        track = MidiTrack::read_track(t_nr, inp);
        if (!track)
        {
            return false;
        }
        m_tracks.push_back(track);
    }
    for (TrackList::iterator tr = m_tracks.begin();
            tr != m_tracks.end(); ++tr)
    {
        // Tempo information should be stored on the first track and
        // (au contraire to what I thought) applied to every track.
        (*tr)->calc_realtimes(m_time_division, m_tracks[0]);
    }

    return true;
}


MidiTrack* MidiFile::getTrack(int n)
{
    return m_tracks[n];
}


int MidiFile::getTrackCount() const
{
    return m_track_count;
}


int MidiFile::getFormatType() const
{
    return m_format_type;
}


struct _track
{
    MidiTrack *t;
    int pos;
    int size;
    int index;
};

/* Merge all tracks into one big track. This will not copy any event
 * and only return a list of pointers to the events in the right order.
 * Note though that this will modify the relative_* times!
 *
 * The tracks will not be modified but may not be used later for
 * playback since the times are messed up (but you could reconstruct
 * them using the absolute_* times)
 *
 * The parameter muted is a set of muted track/channel combinations in
 * the format ttttcccc, that is the last 4 bits are the channel and the
 * remaining bits are the track:
 *      (track_nr << 4) | channel_nr
 *
 * TODO: muted not implemented since the update, will fix later.
 */
EventList MidiFile::mergedTracks(std::set<int> muted)
{
    EventList result;
    typedef std::vector<_track> trackv;
    trackv tracks;
    int tindex = 0;
    for (TrackList::iterator t = m_tracks.begin();
            t != m_tracks.end(); ++t)
    {
        _track tentry;
        tentry.t = *t;
        tentry.pos = 0;
        tentry.size = (*t)->size();
        tentry.index = tindex;
        tracks.push_back(tentry);
        ++tindex;
    }
    bool exhausted;
    MidiEvent* min_event;
    _track* min_track;
    int combination;
    std::map<int, int> chanmap;
    int nextchan = 0;
    for (;;)
    {
        exhausted = true;
        min_event = 0;
        min_track = 0;
        for (trackv::iterator t = tracks.begin();
                t != tracks.end(); ++t)
        {
            if (t->pos < t->size)
            {
                exhausted = false;
                
                if (min_event == 0 || t->t->at(t->pos)->absolute_musec < min_event->absolute_musec)
                {
                    min_event = t->t->at(t->pos);
                    min_track = &(*t);
                }
            }
        }
        if (exhausted) break;
        ++min_track->pos;

        if (min_event->type() == Event_Note_On)
        {
            NoteOnEvent *e = dynamic_cast<NoteOnEvent*>(min_event);
            combination = min_track->index << 4 | e->getChannel();
            if (chanmap.find(combination) == chanmap.end())
            {
                chanmap[combination] = nextchan;
                ++nextchan;
            }
            e->setChannel(chanmap[combination]);
        }
        else if (min_event->type() == Event_Note_Off)
        {
            NoteOffEvent *e = dynamic_cast<NoteOffEvent*>(min_event);
            combination = min_track->index << 4 | e->getChannel();
            if (chanmap.find(combination) == chanmap.end())
            {
                chanmap[combination] = nextchan;
                ++nextchan;
            }
            e->setChannel(chanmap[combination]);
        }
        result.push_back(min_event);
    }
    int dticks;
    double dmusec;
    for (size_t i = 0; i < result.size(); ++i)
    {
        if (i == 0)
        {
            dticks = result[i]->absolute_ticks;
            dmusec = result[i]->absolute_musec;
        }
        else
        {
            dticks = result[i]->absolute_ticks - result[i-1]->absolute_ticks;
            dmusec = result[i]->absolute_musec - result[i-1]->absolute_musec;
        }
        result[i]->relative_ticks = dticks;
        result[i]->relative_musec = dmusec;
    }
    return result;
}
