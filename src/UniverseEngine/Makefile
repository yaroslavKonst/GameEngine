.PHONY: all

all: \
	../../build/universe.o \
	../../build/actor.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
