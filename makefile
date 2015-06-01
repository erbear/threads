CC 		= clang++
CCFLAGS = -lcurl -lncurses -pthread

all: main

main: main.cpp
	$(CC) $(CCFLAGS) main.cpp -o main

test: all
	./main urls.txt 10

clean:
	rm -rf *.o main
