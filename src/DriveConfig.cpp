#include "DriveConfig.hpp"
#include <iostream>
#include <string>
#include <sstream>

#define COMMENT_CHAR '#'
#define WHITESPACE " \t"

DriveConfig::DriveConfig()
{};


DriveConfig::DriveConfig(std::istream &inp)
{
    m_valid = this->read(inp);
}


/* Helper method to split a string by a delim.
 */
std::vector<std::string> split(std::string const &s, std::string const &delim)
{
    std::vector<std::string> result;
    size_t pos, prev, start, len;
    prev = std::string::npos;
    do
    {
        pos = s.find(delim, prev == std::string::npos ? 0 : prev + 1);
        // + 1 because we don't want to include the delimeter
        start = prev == std::string::npos ? 0 : prev + 1;
        len = pos - start;
        result.push_back(s.substr(start, len));
        prev = pos;
    }
    while (pos != std::string::npos);
    return result;
}


/* Helper method to convert a string to an int.
 */
int str_to_int(std::string const &s)
{
    int result;
    std::stringstream ss(s);
    ss >> result;
    return result;
}


/* Trimming strings, removing unneeded whitespace at the beginning and
 * the end
 */
void trim(std::string &what)
{
    if (what.size() == 0) return;
    // We need to find the first whitespace that isn't followed by a
    // non-whitespace character
    size_t ftm = what.find_first_of(WHITESPACE,
            what.find_last_not_of(WHITESPACE) + 1);
    if (ftm != std::string::npos)
    {
        what.erase(ftm);
    }
    // And the first non-whitespace character so we can remove anything
    // that comes before it
    what.erase(0, what.find_first_not_of(WHITESPACE));
}


#define PINMASK(x) (1 << x)
/* Read and parse the given drive config from the given input stream.
 * Returns if the file was valid.
 * This is a private method and only called by the constructor that
 * sets m_valid accordingly.
 */
bool DriveConfig::read(std::istream &inp)
{
    std::string line;
    int lineno = 0;
    unsigned int pins = 0;
    std::vector<std::string> splitted;
    ConnectedDrive cdrive;
    size_t pos;
    while (inp.good())
    {
        std::getline(inp, line);
        ++lineno;
        // Strip out the everything after the comment char
        pos = line.find(COMMENT_CHAR);
        line = line.substr(0, pos);
        trim(line);
        if (line.length() == 0) // full line was a comment or whitespace
        {
            continue;
        }
        splitted = split(line, " ");
        if (splitted.size() != 3)
        {
            std::cerr << "DriveConfig: Invalid line '" << line << "' ("
                << lineno << ")" << std::endl;
            return false;
        }
        if (splitted[0] != "drive")
        {
            std::cerr << "DriveConfig: Invalid command " << splitted[0]
                << std::endl;
            return false;
        }
        cdrive.direction_pin = str_to_int(splitted[1]);
        cdrive.stepper_pin = str_to_int(splitted[2]);
        if ((pins & PINMASK(cdrive.direction_pin))
                || (pins & PINMASK(cdrive.stepper_pin)))
        {
            std::cerr << "Pin already in use (line " << lineno << ")"
                << std::endl;
            return false;
        }
        pins |= (PINMASK(cdrive.direction_pin) | PINMASK(cdrive.stepper_pin));
        m_drives.push_back(cdrive);
    }
    return true;
}
#undef PINMASK


// Reference to avoid a copy
DriveList& DriveConfig::getDrives()
{
    return m_drives;
}


bool DriveConfig::isValid()
{
    return m_valid;
}
