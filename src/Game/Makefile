.PHONY: all

all: \
	../../build/ship.o \
	../../build/player.o \
	../../build/world.o \
	../../build/BaseGrid.o \
	../../build/MainGrid.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
