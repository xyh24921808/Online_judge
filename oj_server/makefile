oj_server:oj_server.cpp
	g++ oj_server.cpp -o oj_server -lctemplate -ljsoncpp -pthread -lcryptopp -lssl -lcrypto 

debug:debug.cpp
	g++ debug.cpp -o debug -pthread -lcryptopp -lssl -lcrypto 

.PHONY:clean
clean:
	rm -rf oj_server

