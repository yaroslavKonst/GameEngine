export CXX = g++
export CXX_OPTS = -Wall -g -std=c++20 -O3
export CXX_OBJ = -c
export AR = ar rcs
export LIBS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXrandr -lfreetype -lportaudio

.PHONY: all clean

all:
	cd Engine ; make
	cd Tools ; make
	cd Game ; make
	cd Resources ; make

clean:
	rm -rf $(BUILD_DIR)
