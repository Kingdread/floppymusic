#ifndef FM_NOTE_OFF_EVENT_HPP
#define FM_NOTE_OFF_EVENT_HPP

#include "../MidiEvent.hpp"

class NoteOffEvent : public MidiEvent
{
    private:
    int m_channel;
    int m_number;

    public:
    NoteOffEvent(int channel, int number);
    virtual ~NoteOffEvent();
    virtual EventType type() const;
    int getChannel() const;
    int getNote() const;

    void setChannel(int c);
};

#endif
