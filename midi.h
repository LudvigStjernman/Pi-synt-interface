#include "RtMidi.h"
#include <thread>
struct midi
{
    RtMidiIn *midin;
    std::string portnamn = "";
    unsigned int portnr = 0;
    void midicallback(double, std::vector<unsigned char>*, void*);
    void cleanup();
    midi();
};