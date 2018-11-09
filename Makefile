.PHONY: all clean 
	
all: client server
	
clean:
		rm -rf client server *.o log.txt
Socket.o: Socket.cpp
		g++ -c -Wall -std=c++11 Socket.cpp
client.o: client.cpp
		g++ -c -Wall -std=c++11 client.cpp
ServerClass.o: ServerClass.cpp
		g++ -c -Wall -std=c++11 ServerClass.cpp 	
server.o: server.cpp
		g++ -c -Wall -std=c++11 server.cpp	
client: client.o Socket.o
		g++ -Wall -std=c++11 -o client client.o Socket.o 
server:  Socket.o server.o ServerClass.o
		g++ -Wall -std=c++11 -o server Socket.o server.o ServerClass.o
