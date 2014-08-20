#ifndef FM_MIDI_EVENT_HPP
#define FM_MIDI_EVENT_HPP

#include <string>

enum EventType
{
    Event_Note_On,
    Event_Note_Off,
//    Event_Note_Aftertouch,
//    Event_Controller,
//    Event_Program_Change,
//    Event_Channel_Aftertouch,
//    Event_Pitch_Bend,
//    Event_Sequence,
    Event_Text,
    Event_Lyrics,
    Event_Tempo,
    Event_Generic
};

class MidiEvent
{
    protected:
    MidiEvent();

    public:
    virtual EventType type() const = 0;
    virtual ~MidiEvent() = 0;

    int relative_ticks;
    int absolute_ticks;
    int relative_musec;
    int absolute_musec;

    static std::string nameForType(EventType t);
};

#endif
