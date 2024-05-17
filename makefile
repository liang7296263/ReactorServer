all:client server

client:client.cpp
	g++ -g -o client client.cpp 
server:server.cpp inetAddress.cpp ServerSocket.cpp ServerEpoll.cpp Channel.cpp \
Eventloop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp EchoServer.cpp ThreadPool.cpp Timestamp.cpp
	g++ -g -o server server.cpp inetAddress.cpp ServerSocket.cpp ServerEpoll.cpp \
	Channel.cpp Eventloop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp EchoServer.cpp ThreadPool.cpp \
	Timestamp.cpp -lpthread

clean:
	rm -f client server
