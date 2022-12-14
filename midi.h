#include "RtMidi.h" //midi-library
#include <thread> //library för att skapa trådar
#include <functional>
//#include "socket.h"
struct midi
{
    RtMidiIn *midin; //midi-mottagar-objekt
    std::string portnamn = ""; //namn för midi-port
    std::vector<std::thread> listenthreads;
    std::vector<unsigned char> message;
    void listener(std::function<void(uint8_t, uint8_t, uint8_t)>);
    void init(std::function<void(uint8_t, uint8_t, uint8_t)>);
    bool run = true;
    double stamp = 0;
    unsigned int nBytes = 2, i = 0;
    unsigned int portnr = 0; //nummer för port
    void cleanup(); //friar allt minne
    midi(std::function<void(uint8_t, uint8_t, uint8_t)>); //kontruktör
    ~midi();
};
