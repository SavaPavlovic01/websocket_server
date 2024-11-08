all:
	g++ src/main.cpp -lssl -lcrypto -std=c++20 -g -o main 