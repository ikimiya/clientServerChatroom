server: server.o
	g++ server.cpp -o server -lws2_32

server.o: server.cpp
	g++ -c server.cpp 

client: client.o
	g++ client.cpp -o client -lws2_32

client.o: client.cpp
	g++ -c client.cpp 

clean:
	rm -f *.o
	rm server
	rm client

runserver: server
	./server 

runclient: client
	./client