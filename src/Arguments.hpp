#ifndef FM_ARGUMENTS_HPP
#define FM_ARGUMENTS_HPP

#include <set>
#include <string>

struct Arguments
{
    float drop_factor;
    std::string cfg_path;
    std::string midi_path;
    std::set<int> mute_tracks;
    bool lyrics;
};

extern Arguments arguments;

void parse_args(int argc, char **argv);
#endif
