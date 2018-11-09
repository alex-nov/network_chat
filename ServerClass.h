#ifndef SERVERCLASS_H
#define SERVERCLASS_H

#include <string.h>
#include <map>

#include "Socket.h"


struct user_info
{
    int num;
    int delay;
    char word[40];
    Socket *sock;
};

class Server
{
public:
    Server( std::string filename, int l_port = std_port );
    ~Server();

    void run(struct user_info* us_info, size_t us_count);
    void printToScreen(std::string message);
    bool writeToFile(std::string message);

private:

    //Слушающий сокет 
    Socket *sock_to_listen;

    //Лог-файл
    std::ofstream logfile;

    int listen_port, users=0, max_users;

    // Данные для работы select
    fd_set master;
    fd_set read_fds;
    int fdmax;

    std::string format_message(int user, std::string message);
};


#endif // SERVERCLASS_H