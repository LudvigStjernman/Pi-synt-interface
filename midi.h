#include "RtMidi.h" //midi-library
#include <thread> //library för att skapa trådar
struct midi
{
    RtMidiIn *midin; //midi-mottagar-objekt
    std::string portnamn = ""; //namn för midi-port
    std::vector<std::thread> listenthreads;
    std::vector<unsigned char> message;
    void listener();
    void init();
    bool run = true;
    double stamp = 0;
    unsigned int nBytes = 2, i = 0;
    unsigned int portnr = 0; //nummer för port
    void cleanup(); //friar allt minne
    midi(); //kontruktör
    ~midi();
};
