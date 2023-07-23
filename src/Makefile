CC=g++
CC_OPTS=-Wall -g -std=c++20 -O3 -fopenmp
CC_OBJ=-c
OUTPUT=game
LIBS=-lglfw -lvulkan -ldl -lpthread -lX11 -lXrandr

.PHONY: all clean

all:
	mkdir -p ../build
	cd Main ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)
	cd VideoEngine ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)
	cd UniverseEngine ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)
	cd PhysicalEngine ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)
	cd Utils ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)
	cd Logger ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)
	cd Assets ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)
	$(CC) $(CC_OPTS) ../build/*.o -o ../build/$(OUTPUT) $(LIBS)

clean:
	rm -rf ../build
