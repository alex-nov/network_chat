#ifndef SOCKETCPP_H
#define SOCKETCPP_H

#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h> 
#include <fstream>


const unsigned short std_port = 7890;
const char std_addr[] = "127.0.0.1";
const int MAXDATASIZE = 1024;
const int MINIBUFSIZE = 80;


class Socket
    {
    public:

        Socket ();
        Socket (int new_handle);
        virtual ~Socket();

        int     getHandle() const;
        void    setHandle(int new_handle);
        std::string getAddress() const;

        bool    Connect( std::string &destination, unsigned short prt );
        int     MakeListened(unsigned short prt);
        int     AcceptConnection();

        bool    isOpen() const;
        void    Close();
        
        virtual bool Send(  void * message, size_t size );
        virtual size_t  Receive( void * message, size_t size );

        Socket& operator=(const Socket &other);

    private:

        void open(struct addrinfo &hints, short int prt);

        bool connected = false, server_socket = false;
        bool binded = false;

        struct addrinfo sock_info;

        int port, handle = -1, dest_handle = 0;
        std::string ip; 
    };

#endif // SOCKETCPP_H  