all:
	g++ *.cpp -g -o hengster `pkg-config --cflags --libs opencv`