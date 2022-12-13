#include "midi.h"
void midi::cleanup()
{
    delete midin;
}
void midi::listener(){
    std::cout << "run\n";
    while ( run ) {
        message.clear();
        stamp = midin->getMessage(&message);
        nBytes = message.size();
        for (i = 0; i < nBytes; i++)
            std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
        if (nBytes > 0)
            std::cout << "stamp = " << stamp << std::endl;
        // Sleep for 10 milliseconds ... platform-dependent.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

}
void midi::init(){
    try
    {
        midin = new RtMidiIn();
    }
    catch (RtMidiError &error)
    {
        error.printMessage();
        exit(EXIT_FAILURE);
    }
    portnr = midin->getPortCount();
    std::cout << "Portnr: " <<portnr << '\n';
    try
    {
        portnamn = midin->getPortName(0);
        std::cout << "Portnamn: " << portnamn << '\n';
    }
    catch (RtMidiError &error)
    {
        std::cout << "Error\n";
        error.printMessage();
        cleanup();
    }
    midin->openPort(1);
    printf("Midi klar\n");
    listener();
}

midi::midi()
{
    listenthreads.push_back(std::thread(&midi::init, this));
}

midi::~midi(){
    run = false;
    for(auto &t : listenthreads)
        t.join();
}