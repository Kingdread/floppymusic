#include "LyricsEvent.hpp"

LyricsEvent::LyricsEvent(std::string text) :
    m_text(text)
{}


LyricsEvent::~LyricsEvent()
{}


EventType LyricsEvent::type() const
{
    return Event_Lyrics;
}


std::string LyricsEvent::getText() const
{
    return m_text;
}
