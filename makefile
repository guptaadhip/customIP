all: 
		g++ -o main RouteTable.cpp MyIp.cpp Sniffer.cpp Main.cpp -std=c++11 -lpcap

clean:
		rm -r main
