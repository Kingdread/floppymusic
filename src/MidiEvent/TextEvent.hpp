#ifndef FM_TEXT_EVENT_HPP
#define FM_TEXT_EVENT_HPP

#include "../MidiEvent.hpp"
#include <string>

class TextEvent : public MidiEvent
{
    private:
    std::string m_text;

    public:
    TextEvent(std::string text);
    virtual ~TextEvent();
    virtual EventType type() const;
    std::string getText() const;
};

#endif
