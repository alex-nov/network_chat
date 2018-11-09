#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>  
#include <cstring>
#include <iostream>

//#include "Address.h"
#include "Socket.h"

using namespace std;

// получить sockaddr, IPv4 или IPv6: максимальная длина принимаемых за раз данных
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
        
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//-------------------------------------------------------------

void Socket::open(struct addrinfo &hints,short int prt)
{
    int rv=0;
    struct addrinfo *servinfo, *p;

    char port_buf[MINIBUFSIZE];
    memset(&port_buf, 0, MINIBUFSIZE);
    sprintf(port_buf,"%d",prt);

    if( ip.empty() )
    {
        rv = getaddrinfo(NULL , port_buf, &hints, &servinfo);
    }
    else
    {
        rv = getaddrinfo(ip.c_str() , port_buf, &hints, &servinfo);
    }    

    if( rv != 0 ) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv) );
        handle = -1;

        return;
    }

    for( p = servinfo; p != NULL; p = p->ai_next ) 
    {
        handle = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (handle == -1 ) 
        {
            perror("socket: can't create socket");
            continue;
        }

        break;
    }
    
    sock_info.ai_family    = p->ai_family; 
    sock_info.ai_socktype  = p->ai_socktype; 
    sock_info.ai_protocol  = p->ai_protocol;
    sock_info.ai_addr      = p->ai_addr;
    sock_info.ai_addrlen   = p->ai_addrlen;

    freeaddrinfo(servinfo);
}

Socket::Socket( ) 
{

}

Socket::Socket(int new_handle)
{
    this->setHandle(new_handle);       
}


Socket::~Socket()
{
    this->Close();
}


string Socket::getAddress() const
{
    return this->ip;
}

int Socket::getHandle() const
{
    return this->handle;
}

void Socket::setHandle(int new_handle)
{
    this->handle = new_handle;
    if (new_handle > 0)
    {
        connected = true;
    }
    else
    {
        connected = false;
    }
}

bool Socket::isOpen() const
{
    return connected;
}


void Socket::Close()
{
    connected = false;
    binded = false;
    memset( &sock_info, 0, sizeof(sock_info) );

    close(handle);
    handle = -1;
}


bool Socket::Send( void * message, size_t size )
{
    if(connected)
    {
        return send(handle, message, size, 0) > 0;
    }

    return false;
}
        

size_t Socket::Receive( void * message, size_t size )
{
    int bytes_read=0;
    if(connected)
    {
        bytes_read = recv( handle, message, size, 0 );
    }
    return bytes_read;
}


bool Socket::Connect( std::string &destination , unsigned short prt )
{
    int rv;
    struct addrinfo hints;

    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    ip = destination;
    if( !connected )
    {
        this->open(hints, prt);    
    
        if(handle < 0)
        {
            return connected = false;
        }

        rv = connect(handle, sock_info.ai_addr, sock_info.ai_addrlen);
        if ( rv == -1 ) 
        {
            close(handle);        
            perror("client: connect");

            return connected = false;
        }

        return connected = true;
    }
    std::cout<<"socket: Alredy connected\n"<<std::endl;
    return connected;
}

int Socket::MakeListened(unsigned short prt)
{
    int yes=1;
    struct addrinfo hints;

    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    //Если сокет не создан - создаем
    if(handle < 0)
    {
        this->open(hints, prt);

        if(handle < 0)
            return handle;
    }

    //cout<<"handle to listen "<<handle<<"\n";
  
    // Биндим сокет на нужный порт(если уже это не делали)
    if (!binded)
    {
        if ( setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 ) 
        {
            perror("setsockopt");
            return -1;
        }

        if (bind(handle, sock_info.ai_addr, sock_info.ai_addrlen) == -1) 
        {
            close(handle);
            perror("server: bind");
            return -1;
        }

        //cout<<"binded\n";
        binded = true;
    }    
   
    // Слушаем наш сокет
    if (listen(handle, 5) == -1) 
    {
        perror("listen error");
        return -1;
    }

    return this->handle;     
}

int Socket::AcceptConnection()
{
    // Принимаем запрос и иницируем создание нового сокета
    socklen_t sin_size;
    struct sockaddr_storage their_addr;
    int new_fd=-1;

    char s[MINIBUFSIZE];

    sin_size = sizeof (their_addr);
    new_fd = accept(handle, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) 
    {
        perror("accept");

        return -1;
    }
    else
    {
        // Получаем адрес отправителя
        inet_ntop(  their_addr.ss_family,
                    get_in_addr((struct sockaddr *)&their_addr),
                    s, sizeof(s) );   

    }
    
    // Оповещаем о том, что у нас новое подключение
    printf("socket: got connection from %s\n", s);
    
    return new_fd;

}