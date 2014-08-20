#ifndef FM_GENERIC_EVENT_HPP
#define FM_GENERIC_EVENT_HPP

#include "../MidiEvent.hpp"

class GenericEvent : public MidiEvent
{
    public:
    GenericEvent();
    virtual ~GenericEvent();
    virtual EventType type() const;
};

#endif
