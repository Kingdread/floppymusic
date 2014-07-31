#include "Arguments.hpp"
#include "Drive.hpp"
#include "DriveConfig.hpp"
#include "MidiReader.hpp"
#include "gpio.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <map>
#include <unistd.h>
#include <vector>


// C C# D D# E F F# G G# A A# H
static double frequencies[] = {261.626, 277.183, 293.665, 311.127, 329.628,
                               349.228, 369.994, 391.995, 415.305, 440.000,
                               466.164, 493.883};

typedef std::vector<Drive*> vDrive;
int main(int argc, char **argv)
{
    std::cout << "[floppymusic 0.1]" << std::endl;
    parse_args(argc, argv);

    std::cout << "Reading drive configuration " << arguments.cfg_path
        << std::endl;   
    std::ifstream dc_file(arguments.cfg_path.c_str());
    if (!dc_file.good())
    {
        std::cerr << "Can't open " << arguments.cfg_path << ": "
            << std::strerror(errno) << std::endl;
        return 1;
    }

    DriveConfig drive_cfg(dc_file);
    if (!drive_cfg.isValid())
    {
        std::cerr << "Invalid drive configuration. Aborting." << std::endl;
        return 1;
    }

    std::cout << "Setting up GPIO" << std::endl;
    setup_io();

    std::cout << "Setting up drives" << std::endl;
    DriveList drive_list = drive_cfg.getDrives();
    vDrive drives;
    for (DriveList::iterator it = drive_list.begin(); it != drive_list.end();
            ++it)
    {
        Drive *d = new Drive(it->direction_pin, it->stepper_pin);
        drives.push_back(d);
        d->reseed();
        d->start();
    }
    int dcount = drives.size();

    std::cout << "Reading MIDI file" << std::endl;
    std::ifstream midi_input(arguments.midi_path.c_str());
    if (!midi_input.good())
    {
        std::cerr << "Error reading '" << arguments.midi_path << "': "
            << std::strerror(errno) << std::endl;
        return 1;
    }
    MidiReader midi(midi_input);
    if (!midi.isValid())
    {
        std::cerr << "Invalid MIDI File. Aborting." << std::endl;
        return 1;
    }

    std::cout << "Merging " << (int)midi.m_header.track_count << " tracks"
        << std::endl;
    MidiTrack track = midi.mergedTracks(arguments.mute_tracks);
    std::cout << "Ready, steady, go!" << std::endl;
    std::map<int, int> channel_map;
    std::map<int, int>::iterator drive_index;
    int pool_free = 0;

    /* Play loop */
    for (std::vector<MidiEvent>::iterator event = track.events.begin();
            event != track.events.end(); ++event)
    {
        // Praise usleep
        if (event->delta_time)
        {
            usleep(event->delta_time);
        }
        if (event->event_type == MIDI_NOTE_OFF)
        {
            // Stop playing and release the drive back to the pool
            drive_index = channel_map.find(event->channel);
            if (drive_index != channel_map.end())
            {
                drives[drive_index->second]->stop();
                pool_free ^= 1 << drive_index->second;
            }
        }
        else if (event->event_type == MIDI_NOTE_ON)
        {
            // See if a drive is free
            for (int check = 0; check < dcount; ++check)
            {
                if (!(pool_free & (1 << check)))
                {
                    // Device is free
                    channel_map[event->channel] = check;
                    drives[check]->play(
                        frequencies[event->param_1 % 12] / arguments.drop_factor);
                    pool_free |= 1 << check;
                    break; // stop searching for a drive
                }
            }
        }
    }

    std::cout << "Cleaning up" << std::endl;
    for (vDrive::iterator drv = drives.begin(); drv != drives.end(); ++drv)
    {
        delete *drv;
        *drv = NULL;
    }
    std::cout << "Bye bye!" << std::endl;
}

/*
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " filename" << std::endl;
        return 1;
    }
    std::ifstream cf(argv[1]);
    DriveConfig drvs(cf);
    if (!drvs.isValid())
    {
        return 1;
    }
    DriveList dl = drvs.getDrives();
    for (DriveList::iterator it = dl.begin(); it != dl.end(); ++it)
    {
        std::cout << it->direction_pin << " " << it->stepper_pin << std::endl;
    }
    return 0;
}

int main(int argc, char **argv)
{
    std::ifstream input_file("test.mid");
    MidiReader midi(input_file);
    std::cerr << "Midi file has " << (int)midi.m_header.track_count << " Tracks" << std::endl;
    int chan;
    std::sscanf(argv[1], "%i", &chan);
    //MidiTrack track = midi.m_tracks[chan];
    MidiTrack track = midi.mergedTracks();
    int len = 0;
    bool sleep = true;
    for (std::vector<MidiEvent>::iterator it = track.events.begin(); it != track.events.end(); ++it)
    {
        std::cerr << (int)it->event_type << ":" << (int)it->channel << " ";
        if (sleep)
        {
            usleep(it->delta_time);
        }
        else
        {
            len += it->delta_time;
        }
        if (it->channel == 0)
        {
            if (it->event_type == 8 || (it->event_type == 0xF && it->channel == 0xF && it->param_1 == 0x2F))
            {
                char buf[64];
                std::snprintf(buf, 63, "beep -f %i -l %i", (int)(frequencies[it->param_1 % 12] * std::pow(2, it->param_1 / 12 - 7)), (int)len/1000);
                std::system(buf);
                sleep = true;
            }
            else if (it->event_type == 9)
            {
                len = 0;
                sleep = false;
            }
        }
    }
}

int main(int argc, char **argv) {
    double freq;
    int spin;
    int dpin;
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " dir-pin step-pin freq" << std::endl;
        return EXIT_FAILURE;
    }
    std::sscanf(argv[1], "%i", &dpin);
    std::sscanf(argv[2], "%i", &spin);
    std::sscanf(argv[3], "%lf", &freq);

    setup_io();

    Drive d(pin, spin);
    d.reseed();
    d.start();
    d.play(freq);
    std::cin.get();
    d.play(0);
}
*/
