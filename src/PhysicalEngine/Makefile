.PHONY: all

all: \
	../../build/CollisionEngine.o
#	../../build/PhysicalEngine.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
