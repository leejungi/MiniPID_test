all: MiniPID.o test.o
	g++ test.o MiniPID.o -o test
test.o: test.cpp
	g++ -c test.cpp
MiniPID.o: MiniPID.cpp MiniPID.h
	g++ -c MiniPID.cpp
