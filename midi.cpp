#include "midi.h"
void midi::cleanup()
{
    delete midin;
}
midi::midicallback(double dtime, std::vector<unsigned char> *message, void *userData){
    
}
midi::midi()
{
    try
    {
        midin = new RtMidiIn();
    }
    catch (RtMidiError &error)
    {
        error.printMessage();
        exit(EXIT_FAILURE);
    }
    midin->setCallback((RtMidiIn::RtMidiCallback)&midicallback);
    portnr = midin->getPortCount();
    try
    {
        portnamn = midin->getPortName(0);
    }
    catch (RtMidiError &error)
    {
        error.printMessage();
        cleanup();
    }
    printf("Midi klar");
}
midi::~midi()
{
    cleanup();
}