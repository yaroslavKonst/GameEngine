.PHONY: all

all: \
	../../build/square.o \
	../../build/ExternModel.o \
	../../build/animation.o \
	../../build/ScriptHandler.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
