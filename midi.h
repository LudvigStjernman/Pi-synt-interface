#include "RtMidi.h" //midi-library
#include <thread> //library för att skapa trådar
struct midi
{
    RtMidiIn *midin; //midi-mottagar-objekt
    std::string portnamn = ""; //namn för midi-port
    unsigned int portnr = 0; //nummer för port
    void midicallback(double, std::vector<unsigned char>*, void*); //funktion som tillkallas för varje not
    void cleanup(); //friar allt minne
    midi(); //kontruktör
};
