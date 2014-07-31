#ifndef FM_DRIVE_HPP
#define FM_DRIVE_HPP

#include <pthread.h>

class Drive {
    private:
    int m_direction_pin;
    int m_step_pin;
    int m_steps;
    bool m_direction;
    double m_freq;
    bool m_running;
    pthread_t m_thread;
    pthread_cond_t m_cond;

    void toggle_direction();
    void default_init();

    // Disallow copy
    Drive(const Drive&);
    Drive& operator=(const Drive&);

    public:
    Drive();
    Drive(int direction_pin, int step_pin);

    ~Drive();

    void loop();
    void play(double frequency);
    int reseed();
    int start();
    void stop();
};

#endif
