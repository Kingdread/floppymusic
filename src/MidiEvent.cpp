#include "MidiEvent.hpp"

MidiEvent::MidiEvent()
{}


MidiEvent::~MidiEvent()
{}


std::string MidiEvent::nameForType(EventType t)
{
    switch (t)
    {
        case Event_Note_On:
            return "NOTE ON";
        case Event_Note_Off:
            return "NOTE OFF";
        case Event_Text:
            return "TEXT";
        case Event_Lyrics:
            return "LYRICS";
        case Event_Tempo:
            return "TEMPO";
        case Event_Generic:
            return "GENERIC";
        default:
            return "UNKNOWN";
    }
}
