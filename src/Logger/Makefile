.PHONY: all

all: \
	../../build/logger.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
