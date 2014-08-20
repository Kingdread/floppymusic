#ifndef FM_DRIVECONFIG_HPP
#define FM_DRIVECONFIG_HPP

#include <istream>
#include <vector>

/* This struct stands for a connected drive and contains the pins that
 * this drive is connected to. Each drive needs two pins (three
 * actually, but one is the ground pin which can be the same for every
 * drive), one for the direction and one for the motor pulse.
 */
struct ConnectedDrive
{
    int direction_pin;
    int stepper_pin;
};
typedef std::vector<ConnectedDrive> DriveList;

class DriveConfig
{
    private:
    DriveList m_drives;
    bool m_valid;

    bool read(std::istream &inp);

    public:
    DriveConfig();
    DriveConfig(std::istream &inp);
    DriveList getDrives() const;
    bool isValid() const;
};

#endif
