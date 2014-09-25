all: 
		g++ -o main RouteTable.cpp Main.cpp -std=c++11

clean:
		rm -r main
