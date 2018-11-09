#include <string>
#include <unistd.h>     
#include <iostream>
#include <string.h>
#include <sstream>
#include <chrono>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <map>

#include "Socket.h"
#include "ServerClass.h"

// обработка ctrl+C для выхода 
volatile sig_atomic_t flag = 0;
void app_exit(int sig)
{ 
    flag = 1; 
}

void show_user_info(struct user_info *ui, size_t len)
{
    unsigned int i=0;
    for( i=0; i<len; ++i)
    {
        std::cout <<  ui[i].num << std::endl;
        std::cout <<  ui[i].delay << std::endl;
        std::cout <<  ui[i].word << std::endl;
        std::cout <<  ui[i].sock->getHandle() << std::endl << std::endl;
    }
}

// ---------------------------------

Server::Server(std::string filename, int l_port) : listen_port(l_port)
{
    //Сокет который будем слушать
    sock_to_listen = new Socket();

    //Открываем лог-файл
    logfile.open(filename.c_str(), std::ios::app);
}

Server::~Server()
{
    // Удаляем слуающий сокет и закрываем логфайл.
    if(sock_to_listen != nullptr)
    {
        delete sock_to_listen;
    }

    logfile.close();  
}

//основная петля работы сервера
void Server::run(struct user_info* us_info, size_t us_count)
{
    int new_handle=-1, nbytes=0, i=0;
    char buf[MAXDATASIZE];

    max_users = us_count;

    struct timeval tv;
    tv.tv_sec = 10;

    // Устанавливаем минимальное время ожидания
    for(i=0; i<max_users; ++i)
    {
    	if(us_info[i].delay < tv.tv_sec)
    		tv.tv_sec = us_info[i].delay;
    }    
    
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    sock_to_listen->MakeListened( listen_port );
    fdmax = sock_to_listen->getHandle();
    FD_SET( fdmax , &master);

    signal(SIGINT, app_exit);

    while(1)
	{
		if(flag)
		{
			flag = 0;

			break;
		}

		FD_ZERO(&read_fds);
        read_fds = master;

        // Ищем первый свободный слот для подключения
        for(users=0; users<max_users; ++users)
        {
        	if(-1 == us_info[users].sock->getHandle())
        		break;
        }

        // Ждем события на чтение
        if( select(fdmax+1, &read_fds, NULL, NULL, &tv) == -1 ) 
        {
            perror("select");
            return;
        }

        for( i = 0; i <= fdmax; i++) 
        {
        	// Есть информация на чтение
            if (FD_ISSET(i, &read_fds))  
            {
            	// Новое подключение
                if (i == sock_to_listen->getHandle()) 
                {
                	// Есть слоты для подключения
                	if(users < max_users) 
                	{	
                    	new_handle = sock_to_listen->AcceptConnection();
                    
                    	if (new_handle == -1) 
                    	{
                        	perror("accept");
                    	}
                    	else
                    	{
                    		char tmp[MINIBUFSIZE];
                    		// Сохраняем данные нового пользователя
                    		us_info[users].sock->setHandle(new_handle);

                    		//Debug info:
                    		//show_user_info(us_info, max_users);

                        	FD_SET(new_handle, &master); //Теперь слушаем и его
                        	if(new_handle > fdmax)
                        	{
                        		fdmax = new_handle;
                        	}

                        	// Отправляем новому пользователю задержку и слово
                        	sprintf(tmp,"%d %s", us_info[users].delay, us_info[users].word);
                        	std::cout<<"send to user " << users << ": " << tmp << std::endl;
                        	us_info[users].sock->Send( tmp,  MINIBUFSIZE );
						}
                    }
                } // Новое подключение
                else
                {
                	// Получаем слово от пользователя	

                	// Находим какому пользователю соответствует дескриптор
                	int user_n = 0;
                	for(user_n=0; user_n<max_users; ++user_n)
                	{
                		if(i == us_info[user_n].sock->getHandle())
                			break;
                	}

                	// Собественно получаем само слово
                	nbytes = us_info[user_n].sock->Receive( buf, MAXDATASIZE );
                    
                    if ( nbytes <= 0) 
                    {
                    	
                        // ошибка или соединение закрыто клиентом
                        if (nbytes == 0) 
                        {
                            // соединение закрыто
                            std::cout << "selectserver: socket: " << i << " hung up\n";
                        } 
                        else 
                        {
                            perror("recv");
                        }

                        // Закрываем сокет и перестаем за ним следить
                        FD_CLR(us_info[user_n].sock->getHandle(), &master);

                        us_info[user_n].sock->setHandle(-1);

                        // Проверяем что больший дескриптор всё ее больший
                        fdmax = sock_to_listen->getHandle();
                        for(int j=0; j<max_users; ++j)
                        {
                        	if(us_info[j].sock->getHandle() > fdmax)
                        	{
                        		fdmax = us_info[j].sock->getHandle();
                        	}
                        }
                        // удалить из главного массива
                    }
                    else
                    {
                    	// Пишем в лог и выводим на экран для отладки
                        this->writeToFile(   this->format_message(user_n, std::string(buf)) );
                        this->printToScreen( this->format_message(user_n, std::string(buf)) );
                    }
                }

            } // Информация на чтение
        } // прохот от i до fdmax 

    } // Основной цикл
}


std::string Server::format_message(int user, std::string message)
{
    char buffer[MINIBUFSIZE];

    // Получаем текущее время и приводим к нужному формату
    time_t seconds = time(NULL);
    tm* timeinfo = localtime(&seconds);
    char format[] = "%H:%M:%S; ";
    strftime(buffer, MINIBUFSIZE, format, timeinfo);
   
 	// Создаем сообщение нужного формата  
    std::ostringstream ouput_string ;        
    ouput_string << buffer << user << ": " << message << std::endl;

    return ouput_string.str();

}

bool Server::writeToFile(std::string message)
{
    if (logfile.is_open())
    {
        logfile << message ;
        return true;
    }

    return false;
}

void Server::printToScreen(std::string message)
{
    std::cout << message ;
}