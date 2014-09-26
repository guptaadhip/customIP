all: 
		g++ -o main RouteTable.cpp Sniffer.cpp Main.cpp -std=c++11 -lpcap

clean:
		rm -r main
