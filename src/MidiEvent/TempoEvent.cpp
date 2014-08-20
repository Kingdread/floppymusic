#include "TempoEvent.hpp"

TempoEvent::TempoEvent(double mpqn) :
    m_mpqn(mpqn)
{}


TempoEvent::~TempoEvent()
{}


EventType TempoEvent::type() const
{
    return Event_Tempo;
}


double TempoEvent::getMpqn() const
{
    return m_mpqn;
}
