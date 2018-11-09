#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <signal.h> 
#include <stdio.h>
#include <string.h>

#include "Socket.h"

using namespace std;

// обработка ctrl+C для выхода 
volatile sig_atomic_t flag = 0;
void app_exit(int sig)
{ 
	flag = 1; 
}

int main(int argc, char *argv[])
{
	char buff[MAXDATASIZE], c_time[MINIBUFSIZE], word[MINIBUFSIZE];
	std::string serv_addr;
	int timeout = 0, pport=0;
	size_t readed = 0;

	Socket *cl_socket = new Socket();

	//Если порт не указан - используем стандартный
	if(argc >= 2)
	{
		pport = atoi(argv[1]);

		if( argc >= 3)
			serv_addr = std::string(argv[2]);
		else
			serv_addr = std::string(std_addr);
	}
	else
	{
		serv_addr = std::string(std_addr);
		pport = std_port;
	}

	if( !cl_socket->Connect(serv_addr, pport) )
	{
		std::cout<<"Cannot connect to server\n";
		delete cl_socket;
		return 1;
	}
	else
	{
		std::cout << "Connected to server " << serv_addr << ":" << pport << std::endl;
	}

	// Получаем значение задержки и слово

	readed = cl_socket->Receive( buff, MAXDATASIZE );

	unsigned int i=0;
	for ( i = 0; i < readed; ++i)	
	{
		if(buff[i] == ' ')
		{
			break;
		}
	}
	strncpy(c_time, buff, i);
	timeout = atoi( static_cast<char*>(c_time));
	strcpy(word, buff + i +1);

	std::cout << "timeout: " << timeout << "| word: " << word << std::endl << std::endl;

	// Регистрируем сигнал выхода 
	signal(SIGINT, app_exit);

	while(1)
	{
		// Условие выхода из цикла
		if(flag)
		{
			flag = 0;
			cl_socket->Close();
			break;
		}

		// Отрабатываем ожидание(в милисекундах)
		sleep(timeout);
		// Отправляем слово на сервер
		std::cout << "Sending to server: " << word << std::endl;
		cl_socket->Send( word, MINIBUFSIZE );		
	}

	delete cl_socket;

	return 0;
}


