#ifndef FM_TEMPO_EVENT_HPP
#define FM_TEMPO_EVENT_HPP

#include "../MidiEvent.hpp"

#define MSEC_PER_MIN 60000000.0
#define BPM_TO_MPQN(x) MSEC_PER_MIN / x
#define MPQN_TO_BPM(x) MSEC_PER_MIN / x

class TempoEvent : public MidiEvent
{
    private:
    double m_mpqn;

    public:
    TempoEvent(double mpqn);
    virtual ~TempoEvent();
    virtual EventType type() const;
    double getMpqn() const;
};

#endif
