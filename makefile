all: 
		g++ -o main RouteTable.cpp MyIp.cpp NetworkHandler.cpp CustomOspf.cpp PacketEngine.cpp Sniffer.cpp Main.cpp -std=c++11 -lpcap -pthread -ggdb

clean:
		rm -r main
