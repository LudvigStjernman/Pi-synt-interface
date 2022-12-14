#include "midi.h"
void midi::cleanup()
{
    delete midin;
}
void midi::listener(std::function<void(uint8_t, uint8_t, uint8_t)> callback){
    std::cout << "run\n";
    while ( run ) {
        message.clear();
        stamp = midin->getMessage(&message);
        nBytes = message.size();
        //callbackfun(message[0], message[1], message[2]);
        if(message.size())
            callback(message[0], message[1], message[2]);
        // Sleep for 10 milliseconds ... platform-dependent.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

}
void midi::init(std::function<void(uint8_t, uint8_t, uint8_t)> callback){
    int port = 0;
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
    int nr = 0;
    try
    {
        for(int i = 0; i < portnr; i++){
            portnamn = midin->getPortName(i);
            if(!portnamn.find("Through") != std::string::npos)
                nr = i;
        }
        std::cout << "Portnamn: " << portnamn << '\n';
    }
    catch (RtMidiError &error)
    {
        std::cout << "Error\n";
        error.printMessage();
        cleanup();
    }
    midin->openPort(nr);
    printf("Midi klar\n");
    listener(callback);
}

midi::midi(std::function<void(uint8_t, uint8_t, uint8_t)> callback)
{
    listenthreads.push_back(std::thread(&midi::init, this, callback));
}

midi::~midi(){
    run = false;
    for(auto &t : listenthreads)
        t.join();
}