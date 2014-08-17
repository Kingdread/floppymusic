#include "MidiReader.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>

using std::memcmp;
using std::memcpy;
using std::vector;

static const char MIDI_HEADER_ID[] = {'M', 'T', 'h', 'd'};
static const char MIDI_TRACK_HEADER_ID[] = {'M', 'T', 'r', 'k'};

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


/* Constructs a MidiReader from the given input stream. This can be an
 * opened file like MidiReader(std::ifstream("DeathMarch.mid"));
 */
MidiReader::MidiReader(std::istream &inp)
{
    m_valid = this->read(inp);
}


/* Read the midi from the given input stream. This is called by the
 * constructor and is a private method.
 */
bool MidiReader::read(std::istream &inp)
{
    unsigned char buffer[4];
    char *sbuffer = reinterpret_cast<char*>(buffer);
    // Check if this is a valid midi file
    inp.read(sbuffer, 4);
    if (memcmp(buffer, MIDI_HEADER_ID, 4))
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
    m_header.format_type = buffer[0] << 8 | buffer[1];
    switch (m_header.format_type)
    {
        case 0:
        case 1:
        case 2:
            break;
        default:
            std::cerr << "MIDI: Invalid format type ("
                << m_header.format_type << ")" << std::endl;
            return false;
    }

    // Read the number of tracks
    inp.read(sbuffer, 2);
    m_header.track_count = buffer[0] << 8 | buffer[1];

    // Read the time division
    inp.read(sbuffer, 2);
    m_header.time_division = buffer[0] << 8 | buffer[1];

    // Header completed, read the tracks
    for (int t_nr = 0; t_nr < m_header.track_count; ++t_nr)
    {
        if (!this->read_track(t_nr, inp))
        {
            return false;
        }
    }

    return true;
}


bool MidiReader::read_track(int t_nr, std::istream &inp)
{
    unsigned char buffer[4] = {0, 0, 0, 0};
    char *sbuffer = reinterpret_cast<char*>(buffer);
    MidiTrack track;
    track.header.tempo = 0;
    track.header.time_division = m_header.time_division;
    inp.read(sbuffer, 4);
    if (memcmp(buffer, MIDI_TRACK_HEADER_ID, 4))
    {
        std::cerr << "MIDI: Invalid midi track " << t_nr
            << ", invalid starting bytes" << std::endl;
        return false;
    }

    inp.read(sbuffer, 4);
    track.header.chunk_size = buffer[0] << 24
        | buffer[1] << 16
        | buffer[2] << 8
        | buffer[3];

    // To make it easier (and faster) we will just read the remaining
    // bytes of the file into memory.
    unsigned char *file_content = new unsigned char[track.header.chunk_size];
    if (file_content == NULL)
    {
        std::cerr << "MIDI: FILE_CONTENT ALLOCATION FAILED, "
            "WHERE IS YOUR GOD NOW" << std::endl;
        return false;
    }
    char *sfile_content = reinterpret_cast<char*>(file_content);
    inp.read(sfile_content, track.header.chunk_size);

    // Position in the track (in bytes since the track start)
    unsigned int i = 0;
    // Number of bytes consumed by read_varlen
    int rv_read;
    MidiEvent last_event = {0, 0, 0, 0, 0};
    // We don't store meta events, yet they can have a delta time. If
    // we ignore that, the whole timing will get messed up!
    int meta_delta = 0;

    while (i < track.header.chunk_size)
    {
        MidiEvent event;
        rv_read = read_varlen(buffer, file_content + i, 4);
        if (rv_read < 0)
        {
            std::cerr << "MIDI: Varlength data too much in track " << t_nr
                << std::endl;
            return false;
        }
        i += rv_read;
        // Storing the delta_time as microseconds since that will be
        // easier to work with later on. We don't have to store the
        // tempo change events either.
        event.delta_time = track.delta_to_musec(varlen_to_int(buffer, rv_read));

        event.event_type = (file_content[i] & 0xF0) >> 4;
        event.channel = file_content[i] & 0x0F;
        ++i;

        // magic meta event:
        if (IS_META(event))
        {
            meta_delta += event.delta_time;
            unsigned int meta_type = file_content[i];
            ++i;
            unsigned int meta_length = file_content[i];
            ++i;
            switch (meta_type)
            {
                case 0x00:
                    // Sequence number
                    track.header.sequence_number = file_content[i] << 8
                        | file_content[i+1];
                    break;
                case 0x03:
                    // Sequence/Track name
                    track.header.sequence_name =
                        std::string(sfile_content + i, meta_length);
                    break;
                case 0x2F:
                    // End of track
                    event.param_1 = 0x2F;
                    track.events.push_back(event);
                    goto end; // goto is evil yadda yadda
                case 0x51:
                    // set tempo
                    track.header.tempo = file_content[i] << 16
                        | file_content[i+1] << 8
                        | file_content[i+2];

            }
            i += meta_length;
            continue;
        }
        else if (IS_SYSEX(event))
        {
            // skip sysex events
            meta_delta += event.delta_time;
            rv_read = read_varlen(buffer, file_content + i, 4);
            if (rv_read < 0)
            {
                std::cerr << "Invalid SysEx event in track " << t_nr
                    << std::endl;
                return false;
            }
            i += rv_read;
            i += varlen_to_int(buffer, rv_read);
            continue;
        }
        else if (!(event.event_type & 0x8))
        {
            // Seems like some midi files omit the status/event type
            // byte if it's the same as last event. This is purely
            // based on observations and I haven't read this anywhere
            // in a midi file format description.
            event.event_type = last_event.event_type;
            event.channel = last_event.channel;
            // Compensate the "wrong" byte
            --i;
        }
        
        // normal event
        event.delta_time += meta_delta;
        meta_delta = 0;
        event.param_1 = file_content[i]; ++i;
        if (event.event_type != 0xC && event.event_type != 0xD)
        {
            // Program change and Channel aftertouch events don't have the
            // second data byte set, so this would mess up with the whole file
            // layout. So only read the param if the event is NOT a PC/CA
            // event.
            event.param_2 = file_content[i]; ++i;
        }
        else
        {
            event.param_2 = 0;
        }
        // Save last_event before changing the event's type, as otherwise
        // the next event could be a NOTE_OFF too when it's really a NOTE_ON
        last_event = event;
        if (event.event_type == MIDI_NOTE_ON && event.param_2 == 0)
        {
            // NOTE ON with velocity of 0 should be treated as NOTE OFF
            event.event_type = MIDI_NOTE_OFF;
        }
        track.events.push_back(event);
    }

end:
    m_tracks.push_back(track);
    delete[] file_content;
    return true;
}


bool MidiReader::isValid()
{
    return m_valid;
}


struct _Entry
{
    vector<MidiEvent>::iterator iter;
    vector<MidiEvent>::iterator end;
    int index;
};


/* Returns a track that contains all other tracks merged into one. Each
 * channel stays seperated but might have a different ID after the
 * merge, like
 *   Track 1 Channel 1 -> Channel 1
 *   Track 1 Channel 2 -> Channel 2
 *   Track 2 Channel 1 -> Channel 3
 * This modifies the events in the origianl tracks, so they will be
 * useless. That's why this function delets all tracks and marks the
 * midi as invalid.
 *
 * The muted parameter is a set of muted channels. Event on those
 * tracks/channels will be ignored. Every entry is built like
 *   (track_nr << 4) + channel
 */
MidiTrack MidiReader::mergedTracks(std::set<int> muted)
{
    MidiTrack result;
    vector<_Entry> revents;
    typedef vector<_Entry>::iterator viter;
    int num = 0;
    for (vector<MidiTrack>::iterator it = m_tracks.begin();
            it != m_tracks.end(); ++it)
    {
        _Entry t;
        t.iter = it->events.begin();
        t.end = it->events.end();
        t.index = num;
        revents.push_back(t);
        ++num;
    }
    
    std::map<int,int> chanmap;
    int nextchan = 0;
    bool exhausted;
    int min_delta;
    vector<MidiEvent>::iterator *min_iter = NULL;
    vector<MidiEvent>::iterator *min_end;
    MidiEvent min_event;
    unsigned int min_index;
    for (;;)
    {
        // Check if all events from all tracks have been processed
        exhausted = true;
        for (viter it = revents.begin(); it != revents.end(); ++it)
        {
            if (it->iter != it->end)
            {
                exhausted = false;
                // Don't need to check the remaining as it is enough to
                // have one that isn't exhausted yet.
                break;
            }
        }
        if (exhausted)
        {
            break;
        }

        // Find the next event from all tracks
        min_delta = -1;
        for (viter it = revents.begin(); it != revents.end(); ++it)
        {
            if (it->iter == it->end) continue;
            if (min_delta == -1 || it->iter->delta_time < min_delta)
            {
                min_delta = it->iter->delta_time;
                min_iter = &it->iter;
                min_end = &it->end;
                min_index = it->index;
            }
        }
        if (min_delta == -1)
        {
            std::cerr << "Oops, couldn't determine next event. "
                "Something went wrong" << std::endl;
            break;
        }
        min_event = **min_iter;
        min_event.channel |= min_index << 4;
        if (muted.find(min_event.channel) == muted.end())
        {
            // Track/Channel is not muted :)
            if (chanmap.find(min_event.channel) == chanmap.end())
            {
                chanmap[min_event.channel] = nextchan;
                min_event.channel = nextchan;
                ++nextchan;
            }
            else
            {
                min_event.channel = chanmap[min_event.channel];
            }
            result.events.push_back(min_event);
            // Adjust the delta time of all the other events
            for (viter it = revents.begin(); it != revents.end(); ++it)
            {
                if (it->iter == it->end) continue;
                if (&it->iter == min_iter) continue;
                it->iter->delta_time -= min_delta;
            }
            ++(*min_iter);
        }
        else
        {
            // We have to adjust the delta-time of the next event on this
            // track!
            ++(*min_iter);
            if (*min_iter != *min_end)
            {
                (*min_iter)->delta_time += min_delta;
            }
        }
    }
    m_tracks.clear();
    m_valid = false;
    return result;
}

