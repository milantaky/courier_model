all: courier.cc
	g++ -g -O2 -o courier courier.cc -lsimlib -lm 
run:
	./courier