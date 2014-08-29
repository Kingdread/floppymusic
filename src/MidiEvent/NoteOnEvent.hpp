#ifndef FM_NOTE_ON_EVENT_HPP
#define FM_NOTE_ON_EVENT_HPP

#include "../MidiEvent.hpp"

class NoteOnEvent : public MidiEvent
{
    private:
    int m_channel;
    int m_number;
    int m_velocity;

    public:
    NoteOnEvent(int channel, int number, int velocity);
    virtual ~NoteOnEvent();
    virtual EventType type() const;
    int getChannel() const;
    int getNote() const;
    int getVelocity() const;

    void setChannel(int c);
    bool muted;
};

#endif
