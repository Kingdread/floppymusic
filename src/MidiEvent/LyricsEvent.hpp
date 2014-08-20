#ifndef FM_LYRICS_EVENT_HPP
#define FM_LYRICS_EVENT_HPP

#include "../MidiEvent.hpp"
#include <string>

class LyricsEvent : public MidiEvent
{
    private:
    std::string m_text;

    public:
    LyricsEvent(std::string text);
    virtual ~LyricsEvent();
    virtual EventType type() const;
    std::string getText() const;
};

#endif
