CC 		= clang++
CCFLAGS = -lcurl -lncurses -pthread

all: main

main: main.cpp
	$(CC) $(CCFLAGS) main.cpp -o main

clean:
	rm -rf *.o main
