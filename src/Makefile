CC=g++
CC_OPTS=-Wall -g
CC_OBJ=-c
OUTPUT=game
LIBS=-lglfw -lvulkan -ldl -lpthread -lX11 -lXrandr

.PHONY: all clean

all:
	mkdir -p ../build
	cd Main ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)
	cd VideoEngine ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)
	$(CC) $(CC_OPTS) ../build/*.o -o ../build/$(OUTPUT) $(LIBS)

clean:
	rm -rf ../build
