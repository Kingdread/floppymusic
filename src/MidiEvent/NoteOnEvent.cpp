#include "NoteOnEvent.hpp"

NoteOnEvent::NoteOnEvent(int channel, int number, int velocity) :
    m_channel(channel), m_number(number), m_velocity(velocity)
{}


NoteOnEvent::~NoteOnEvent()
{}


EventType NoteOnEvent::type() const
{
    return Event_Note_On;
}


int NoteOnEvent::getChannel() const
{
    return m_channel;
}


int NoteOnEvent::getNote() const
{
    return m_number;
}


int NoteOnEvent::getVelocity() const
{
    return m_velocity;
}


void NoteOnEvent::setChannel(int c)
{
    m_channel = c;
}
