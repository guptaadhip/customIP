all: 
		g++ -o main RouteTable.cpp MyIp.cpp NetworkHandler.cpp CustomOspf.cpp PacketEngine.cpp Sniffer.cpp Main.cpp -std=c++11 -lpcap -pthread

clean:
		rm -r main
