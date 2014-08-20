#include "TextEvent.hpp"

TextEvent::TextEvent(std::string text) :
    m_text(text)
{}


TextEvent::~TextEvent()
{}


EventType TextEvent::type() const
{
    return Event_Text;
}


std::string TextEvent::getText() const
{
    return m_text;
}
