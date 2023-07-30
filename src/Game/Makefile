.PHONY: all

all: \
	../../build/ship.o \
	../../build/player.o \
	../../build/world.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
