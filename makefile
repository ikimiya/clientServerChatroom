server: server.o
	g++ server.cpp -o server -lws2_32

server.o: server.cpp
	g++ -c server.cpp 

cilent: cilent.o
	g++ cilent.cpp -o cilent -lws2_32

cilent.o: cilent.cpp
	g++ -c cilent.cpp 

clean:
	rm -f *.o

runserver: server
	./server 

runcilent: cilent
	./cilent