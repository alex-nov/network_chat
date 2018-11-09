#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <iostream>

#include <sys/time.h>
#include <sys/types.h>

#include "Socket.h"
#include "ServerClass.h"


using namespace std;

extern void show_user_info(struct user_info *ui, size_t len);


int main(int argc, char *argv[])
{
    string filename(argv[1]);

    size_t count = atoi(argv[2]);


    // Структуры пользовательской информации
    struct user_info *uinfo = new user_info[count];
    for(unsigned int i=0 ; i<count ; i++)
    {
        uinfo[i].num = i;
        uinfo[i].delay = atoi(argv[i*2+3]);
        strcpy(uinfo[i].word, argv[i*2+4]);
        uinfo[i].sock = new Socket(-1);
    }

    //Debug info
    //show_user_info(uinfo, count);

    Server *serv = new Server(filename);

    //Основная петля работы сервера
    serv->run(uinfo, count);

    cout<<"Shutdown server\n";

    // Очищаем память
    for(unsigned int i=0 ; i<count ; i++)
    {
        if(uinfo[i].sock != nullptr)
            delete uinfo[i].sock;
    }
    delete serv;

    return 0;
}

