#include "midi.h"
void midi::cleanup()
{
    delete midin;
}
void midi::midicallback(double dtime, std::vector<unsigned char> *message, void *userData){
    std::string str(message->begin(), message->end());
    std::cout << str;
    std::cout << "press";
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
    midin->setCallback(&midicallback, (void*) this);
    portnr = midin->getPortCount();
    std::cout << portnr << '\n';
    try
    {
        portnamn = midin->getPortName(0);
    }
    catch (RtMidiError &error)
    {
        std::cout << "Error\n";
        error.printMessage();
        cleanup();
    }
    printf("Midi klar");
}