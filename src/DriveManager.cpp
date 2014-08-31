#include "DriveManager.hpp"
#include "gpio.hpp"
#include <unistd.h>
#define MAX_STEPS 80
#define RESOLUTION 7200
#define SEC_IN_NSEC (1000000000)
DriveManager::DriveManager() : m_running(false)
{}


DriveManager::DriveManager(DriveList drives) : m_running(false)
{
    for (DriveList::iterator drv = drives.begin();
            drv != drives.end(); ++drv)
    {
        Drive d = {
            drv->direction_pin,
            drv->stepper_pin,
            0, -1, 0, true};
        m_drives.push_back(d);
    }
}


DriveManager::~DriveManager()
{
    if (!m_running) return;
    m_running = false;
    pthread_join(m_thread, NULL);
}


static void *_drive_jumper(void *drive)
{
    reinterpret_cast<DriveManager*>(drive)->loop();
    return NULL;
}


void DriveManager::setup()
{
    if (m_running) return;
    for (Drives::iterator d = m_drives.begin();
            d != m_drives.end(); ++d)
    {
        // Always use INP before OUT
        INP_GPIO(d->direction_pin);
        INP_GPIO(d->stepper_pin);
        OUT_GPIO(d->direction_pin);
        OUT_GPIO(d->stepper_pin);
        
        // "reseed" the drive
#ifndef NOGPIO
        GPIO_CLR = 1 << d->direction_pin;
#endif
        for (int i=0; i<MAX_STEPS; ++i)
        {
#ifndef NOGPIO
            GPIO_SET = 1 << d->stepper_pin;
            GPIO_CLR = 1 << d->stepper_pin;
#endif
            usleep(2500);
        }
#ifndef NOGPIO
        GPIO_SET = 1 << d->direction_pin;
#endif
    }
    pthread_mutex_init(&m_mutex, NULL);
    pthread_create(&m_thread, NULL, _drive_jumper, this);
    m_running = true;
}


void DriveManager::loop()
{
    timespec t;
    unsigned long nsec = SEC_IN_NSEC / RESOLUTION;
    t.tv_sec = nsec / SEC_IN_NSEC;
    t.tv_nsec = nsec % SEC_IN_NSEC;
    while (m_running)
    {
        pthread_mutex_lock(&m_mutex);
        for (Drives::iterator d = m_drives.begin();
                d != m_drives.end(); ++d)
        {
            if (d->maxticks == -1) continue;
            ++d->ticks;
            if (d->ticks >= d->maxticks)
            {
                // should do a step
                // need to reverse direction first?
                ++d->steps;
                if (d->steps > MAX_STEPS)
                {
                    d->direction = !d->direction;
#ifndef NOGPIO
                    if (d->direction)
                    {
                        GPIO_SET = 1 << d->direction_pin;
                    }
                    else 
                    {
                        GPIO_CLR = 1 << d->direction_pin;
                    }
#endif
                    d->steps = 0;
                }
                // now send a pulse
#ifndef NOGPIO
                GPIO_SET = 1 << d->stepper_pin;
                GPIO_CLR = 1 << d->stepper_pin;
#endif
                d->ticks = 0;
            }
        }
        pthread_mutex_unlock(&m_mutex);
        nanosleep(&t, NULL);
    }
}


void DriveManager::play(int drive, double frequency)
{
    if (frequency == 0)
    {
        this->stop(drive);
        return;
    }
    pthread_mutex_lock(&m_mutex);
    Drive& d = m_drives[drive];
    d.ticks = 0;
    d.maxticks = RESOLUTION / frequency;
    pthread_mutex_unlock(&m_mutex);
}


void DriveManager::stop(int drive)
{
    m_drives[drive].maxticks = -1;
}
