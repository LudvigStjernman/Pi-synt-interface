#include <SFML/Network.hpp>
#include <iostream>
//protokoll: index, värde
/*
1. ton
2. volym
3. oktav
4. vågform
5. lågpass frekvens
6. högpass frekvens
*/
namespace SocketNS{
    sf::UdpSocket socket;
    char data[2] = {};
    unsigned short port = 54000;
    sf::IpAddress addrss = "localhost";
    enum command {
        ton,
        volym,
        vågform,
        lågpassF,
        högpassF
    };
    void init(){
        if(socket.bind(port) != sf::Socket::Done){
            std::cout << "socket failed";
        }

    }

    void send(command cmd, uint8_t value){
        data[0] = cmd;
        data[1] = value;
        if(socket.send(data, 2, addrss, port) != sf::Socket::Done)
            std::cout << "sending failed";
        
    }
    
}