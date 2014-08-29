#include "Arguments.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

Arguments arguments = {1, "drives.cfg", "", std::set<int>(), false};

static int help = 0;

static option long_opts[] = {
    // Arguments
    {"dropfactor", required_argument, 0, 'd'},
    {"configpath", required_argument, 0, 'c'},
    {"mute",       required_argument, 0, 'm'},
    // Flags
    {"help",       no_argument,       &help, 1},
    {"lyrics",     no_argument,       0, 'l'},

    {0, 0, 0, 0}
};


static void print_usage()
{
    std::cout << "Usage: floppymusic [-c PATH] [-d FACTOR] [-m MUTE] [-l] MIDIFILE" << std::endl;
}


static void print_help()
{
    std::cout <<
        "-c PATH, --configpath    Sets the path of the drive configuration file\n"
        "\n"
        "-d FACTOR, --dropfactor  Sets the 'drop factor'. A drop factor of 0\n"
        "                         uses the frequencies as they are. A factor\n"
        "                         greater than 0 drops all notes by n oct-\n"
        "                         aves, while a negative integer makes every\n"
        "                         note higher.\n"
        "\n"
        "-l, --lyrics             Print lyrics (if available)\n"
        "\n"
        "-m MUTE, --mute          Mutes channels. The format is\n"
        "                         track:channel,track:channel,... If only\n"
        "                         track is given then every channel on the\n"
        "                         track will be muted.\n"
        "\n"
        "MIDIFILE                 The MIDI file that should be played."
        << std::endl;
}


static void parse_muted(std::string &param)
{
    int track, channel;
    if (std::sscanf(param.c_str(), "%i:%i", &track, &channel) == 1)
    {
        // Channel not given
        for (int c = 0; c < 16; ++c)
        {
            arguments.mute_tracks.insert((track << 4) | c);
        }
    }
    else
    {
        arguments.mute_tracks.insert((track << 4) | channel);
    }
}


void parse_args(int argc, char **argv)
{
    int option_index = 0;
    int c;
    bool invalid = false;
    while ((c = getopt_long(argc, argv, "c:d:hlm:", long_opts, &option_index)) != -1)
    {
        switch (c)
        {
            case 0:
                // setting a flag
                break;
            case 'c':
                // Config file path
                arguments.cfg_path = std::string(optarg);
                break;
            case 'd':
                // Dropfactor
                {
                    std::stringstream ss(optarg);
                    double arg;
                    ss >> arg;
                    arguments.drop_factor = std::pow(2, arg);
                }
                break;
            case 'h':
                // Help message
                help = 1;
                break;
            case 'l':
                // Lyrics
                arguments.lyrics = true;
                break;
            case 'm':
                // Mute channels
                {
                    std::stringstream ss(optarg);
                    std::string param;
                    while (!ss.eof())
                    {
                        std::getline(ss, param, ',');
                        parse_muted(param);
                    }
                }
                break;
            case '?':
                // getopt will print a message, just remember to exit later
                invalid = true;
                break;
            default:
                std::exit(1);
        }
    }
    
    if (invalid)
    {
        std::exit(1);
    }

    if (help)
    {
        print_usage();
        print_help();
        std::exit(0);
    }

    if (optind != argc - 1)
    {
        print_usage();
        std::exit(1);
    }

    arguments.midi_path = argv[optind];
}
