#include "GenericEvent.hpp"

GenericEvent::GenericEvent()
{}


GenericEvent::~GenericEvent()
{}


EventType GenericEvent::type() const
{
    return Event_Generic;
}

