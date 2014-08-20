#include "MidiTrack.hpp"
#include "MidiEvents.hpp"
#include <cstring>
#include <iostream>

static const char MIDI_TRACK_HEADER_ID[] = {'M', 'T', 'r', 'k', 0};


#define IS_META(event) (event.event_type == 0xF && event.channel == 0xF)
#define IS_SYSEX(event) (event.event_type == 0xF && \
        (event.channel == 0x0 || event.channel == 0x7))


/* Extracts a variable length value from a char array, starting at *inp
 * and copying the relevant bytes to *buffer. Reads at most max bytes
 * and returns the number of bytes used.
 */
static int read_varlen(unsigned char buffer[], unsigned char const *inp, int max)
{
    int i = 0;
    do
    {
        if (i >= max)
        {
            return -i;
        }
        buffer[i] = inp[i] & 0x7F;
        ++i;
    } while (inp[i-1] & 0x80);
    return i;
}


/* Converts a variable length value (stored in *buffer) that has length
 * num. Returns the value.
 */
static int varlen_to_int(unsigned char const buffer[], int num)
{
    int res = 0;
    for (int i = 0; i < num; ++i)
    {
        // since the first bit is unused we only have to shift by 7 instead of
        // 8 here:
        // 0x81 0x00 (varlen) = 0x80 (normal int)
        res |= buffer[i] << ((num - i - 1) * 7);
    }
    return res;
}


MidiTrack::MidiTrack()
{}


MidiTrack::~MidiTrack()
{
    for (EventList::iterator event = m_events.begin();
            event != m_events.end(); ++event)
    {
        delete *event;
        *event = 0;
    }
}


void MidiTrack::insert(MidiEvent *event)
{
    m_events.push_back(event);
}


/* Read a single track from the given input stream. Advances the input
 * stream by the bytes read.
 *
 * Returns either a pointer to a MidiTrack or NULL if errors occured.
 */
MidiTrack* MidiTrack::read_track(int t_nr, std::istream &inp)
{
    unsigned char buffer[4] = {0, 0, 0, 0};
    char *sbuffer = reinterpret_cast<char*>(buffer);
    MidiTrack* track = new MidiTrack;
    if (!track)
    {
        std::cerr << "Can't allocate enough space for MidiTrack" << std::endl;
        return 0;
    }
    inp.read(sbuffer, 4);
    if (std::memcmp(buffer, MIDI_TRACK_HEADER_ID, 4))
    {
        std::cerr << "MIDI: Invalid midi track " << t_nr
            << ", invalid starting bytes" << std::endl;
        delete track;
        return 0;
    }

    inp.read(sbuffer, 4);
    track->m_chunk_size = buffer[0] << 24
        | buffer[1] << 16
        | buffer[2] << 8
        | buffer[3];

    // Position in the track (in bytes since the track start)
    int i = 0;
    // Number of bytes consumed by read_varlen
    int rv_read;
    int last_event = 0;
    int delta_time = 0;
    int event_type = 0;
    int channel = 0;
    int ticks = 0;

    // To make it easier (and faster) we will just read the remaining
    // bytes of the file into memory.
    unsigned char *file_content = new unsigned char[track->m_chunk_size];
    if (file_content == NULL)
    {
        std::cerr << "MIDI: FILE_CONTENT ALLOCATION FAILED, "
            "WHERE IS YOUR GOD NOW" << std::endl;
        delete track;
        return 0;
    }
    char *sfile_content = reinterpret_cast<char*>(file_content);
    inp.read(sfile_content, track->m_chunk_size);
    if (inp.gcount() != (int)track->m_chunk_size)
    {
        std::cerr << "MIDI: Couldn't read all " << track->m_chunk_size
            << " bytes, maybe the header is corrupted?" << std::endl;
        goto fail;
    }

    while (i < track->m_chunk_size)
    {
        MidiEvent* event = 0;
        rv_read = read_varlen(buffer, file_content + i, 4);
        if (rv_read < 0)
        {
            std::cerr << "MIDI: Varlength data too much in track " << t_nr
                << std::endl;
            goto fail;
        }
        i += rv_read;
        delta_time = varlen_to_int(buffer, rv_read);
        ticks += delta_time;

        event_type = (file_content[i] & 0xF0) >> 4;
        channel = file_content[i] & 0x0F;
        ++i;

category:        
        if (event_type == 0x8)
        {
            event = new NoteOffEvent(channel, file_content[i]);
            i += 2;
        }
        else if (event_type == 0x9)
        {
            if (file_content[i+1] == 0)
            {
                // NOTE ON with velocity of 0 should be treated as NOTE OFF
                event = new NoteOffEvent(channel, file_content[i]);
            }
            else
            {
                event = new NoteOnEvent(channel, file_content[i], file_content[i+1]);
            }
            i += 2;
        }
        else if (event_type == 0xC || event_type == 0xD)
        {
            event = new GenericEvent();
            ++i;
        }
        else if (!(event_type & 0x8))
        {
            // Reuse last event
            event_type = last_event >> 8;
            channel = last_event & 0xF;
            --i;
            goto category;
        }
        else if (event_type == 0xF && channel == 0xF)
        {
            // Meta event
            unsigned int meta_type = file_content[i]; ++i;
            unsigned int meta_length = file_content[i]; ++i;
            switch (meta_type)
            {
                case 0x01:
                    // Text event
                    event = new TextEvent(std::string(
                                sfile_content + i, meta_length));
                    break;
                case 0x05:
                    // Lyrics
                    event = new LyricsEvent(std::string(
                                sfile_content + i, meta_length));
                    break;
                case 0x2F:
                    // End of track:
                    goto end;
                case 0x51:
                    // Set tempo
                    event = new TempoEvent(file_content[i] << 16 |
                            file_content[i+1] << 8 |
                            file_content[i]);
                    break;
                default:
                    event = new GenericEvent();
                    break;
            }
            i += meta_length;
        }
        else if (event_type == 0xF && (channel == 0x0 || channel == 0x7))
        {
            // SysEx event
            rv_read = read_varlen(buffer, file_content + i, 4);
            if (rv_read < 0)
            {
                std::cerr << "Invalid SysEx event in track " << t_nr
                    << std::endl;
                goto fail;
            }
            i += rv_read;
            i += varlen_to_int(buffer, rv_read);
            event = new GenericEvent();
        }
        else
        {
            event = new GenericEvent();
            i += 2;
        }

        last_event = (event_type << 8) | channel;
        if (!event)
        {
            std::cerr << "Event allocation failed" << std::endl;
            goto fail;
        }
        event->relative_ticks = delta_time;
        event->absolute_ticks = ticks;
        track->m_events.push_back(event);

    }

end:
    delete[] file_content;
    return track;

fail:
    delete[] file_content;
    delete track;
    return 0;
}


/* Takes the microseconds per quarter note, the time division of the
 * MIDI header and the number of ticks and returns... the ticks
 * converted to real time microseconds!
 *
 * Only works with time divisions of ond type yet (though I haven't
 * found a MIDI yet that didn't use this type)
 */
static int calc_musec(double mpqn, int time_div, long ticks)
{
    if ((time_div & 0x8000) == 0)
    {
        // time division is in ticks per quarter note, header.tempo contains
        // the microseconds per quarter note
        return (double)mpqn * ticks / time_div;
    }
    return -1;
}


/* Advances the given iterator to the next tempo change event
 */
static void find_tempo_event(EventList::const_iterator &tit,
        EventList::const_iterator end)
{
    ++tit;
    while (tit != end && (*tit)->type() != Event_Tempo)
    {
        ++tit;
    }
}


/* Calculate absolute_musec and relative_musec for (this) using
 * timeline as the track containing the tempo change events
 */
void MidiTrack::calc_realtimes(int time_div, MidiTrack const *timeline)
{
    long abs_musec = 0;
    static TempoEvent default_tempo = TempoEvent(BPM_TO_MPQN(120));
    TempoEvent const *tempo = &default_tempo;
    EventList::const_iterator timeline_it = timeline->m_events.begin();
    EventList::const_iterator timeline_end = timeline->m_events.end();

    EventList::iterator event = m_events.begin();
    EventList::iterator end = m_events.end();
    
    while (event != end)
    {
        find_tempo_event(timeline_it, timeline_end);
        while (event != end &&
                // either there is no further tempo change
                (timeline_it == timeline_end ||
                // or the next tempo change is still in the future
                 (*event)->absolute_ticks < (*timeline_it)->absolute_ticks))
        {
            (*event)->relative_musec = calc_musec(tempo->getMpqn(), time_div, (*event)->relative_ticks);
            //std::cerr << (*event)->relative_musec << std::endl;
            abs_musec += (*event)->relative_musec;
            (*event)->absolute_musec = abs_musec;
            ++event;
        }
        if (timeline_it != timeline_end)
        {
            tempo = dynamic_cast<TempoEvent*>(*timeline_it);
        }
    }
}


EventList::iterator MidiTrack::begin()
{
    return m_events.begin();
}


EventList::iterator MidiTrack::end()
{
    return m_events.end();
}


MidiEvent* MidiTrack::at(int n)
{
    return m_events[n];
}


int MidiTrack::size() const
{
    return m_events.size();
}
