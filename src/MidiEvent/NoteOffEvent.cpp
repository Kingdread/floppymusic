#include "NoteOffEvent.hpp"

NoteOffEvent::NoteOffEvent(int channel, int number) :
    m_channel(channel), m_number(number), muted(false)
{}


NoteOffEvent::~NoteOffEvent()
{}


EventType NoteOffEvent::type() const
{
    return Event_Note_Off;
}


int NoteOffEvent::getChannel() const
{
    return m_channel;
}


int NoteOffEvent::getNote() const
{
    return m_number;
}


void NoteOffEvent::setChannel(int c)
{
    m_channel = c;
}
