.PHONY: all

all: \
	../../build/window.o \
	../../build/VkInstanceHandler.o \
	../../build/video.o

../../build/window.o: window.cpp window.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o ../../build/window.o window.cpp

../../build/VkInstanceHandler.o: VkInstanceHandler.cpp VkInstanceHandler.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o ../../build/VkInstanceHandler.o VkInstanceHandler.cpp

../../build/video.o: video.cpp video.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o ../../build/video.o video.cpp
