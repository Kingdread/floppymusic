#include "Drive.hpp"
#include "gpio.hpp"
#include <time.h>
#include <unistd.h>

#define SEC_IN_NSEC (1000000000)
#define MAX_STEPS (80)

Drive::Drive()
{
    this->default_init();
}


Drive::Drive(int direction_pin, int step_pin)
{
    this->default_init();
    m_direction_pin = direction_pin;
    m_step_pin = step_pin;
    // Always use INP_GPIO before OUT_GPIO
    INP_GPIO(direction_pin);
    OUT_GPIO(direction_pin);
    INP_GPIO(step_pin);
    OUT_GPIO(step_pin);
}


void Drive::default_init()
{
    m_direction_pin = -1;
    m_step_pin = -1;
    m_steps = 0;
    m_direction = false;
    m_freq = 0.0;
    m_running = false;
}


void *_drive_runner(void *drive)
{
    Drive *d = (Drive*)drive;
    d->loop();
    return NULL;
}


int Drive::start()
{
    if (m_running) return 0;
    m_running = true;
    pthread_create(&m_thread, NULL, _drive_runner, this);
    return 0;
}



Drive::~Drive()
{
    if (m_direction_pin < 0 && m_step_pin < 0) return;
    if (!m_running) return;
    m_running = false;
    pthread_cond_broadcast(&m_cond);
    pthread_join(m_thread, NULL);
}


void Drive::play(double frequency)
{
    bool notify = (m_freq == 0.0);
    if (m_freq == frequency) return;
    m_freq = frequency;
    if (notify)
    {
        pthread_cond_broadcast(&m_cond);
    }
}


void Drive::loop()
{
    unsigned long nsec;
    timespec t;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    while (m_running)
    {
#ifndef NOGPIO
        GPIO_SET = 1 << m_step_pin;
        GPIO_CLR = 1 << m_step_pin;
#endif

        m_steps++;
        if (m_steps == MAX_STEPS)
        {
            m_steps = 0;
            toggle_direction();
        }
        if (m_freq == 0.0)
        {
            pthread_cond_wait(&m_cond, &mutex);
        }
        if (!m_running)
        {
            break;
        }
        nsec = SEC_IN_NSEC / m_freq;
        t.tv_nsec = nsec % SEC_IN_NSEC;
        t.tv_sec = (int) nsec / SEC_IN_NSEC;
        nanosleep(&t, NULL);
    }
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
}


int Drive::reseed()
{
    if (m_running) return -1;
    m_steps = 0;
    m_direction = true;
#ifndef NOGPIO
    GPIO_CLR = 1 << m_direction_pin;
#endif
    for (int i=0; i<MAX_STEPS; ++i)
    {
#ifndef NOGPIO
        GPIO_SET = 1 << m_step_pin;
        GPIO_CLR = 1 << m_step_pin;
#endif
        usleep(2500);
    }
#ifndef NOGPIO
    GPIO_SET = 1 << m_direction_pin;
#endif
    return 0;
}


void Drive::stop()
{
    play(0.0);
}


inline void Drive::toggle_direction()
{
    m_direction = !m_direction;
#ifndef NOGPIO
    if (m_direction)
    {
        GPIO_SET = 1 << m_direction_pin;
    }
    else
    {
        GPIO_CLR = 1 << m_direction_pin;
    }
#endif
}
