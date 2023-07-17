.PHONY: all

all: \
	../../build/window.o \
	../../build/VkInstanceHandler.o \
	../../build/video.o \
	../../build/MemoryManager.o \
	../../build/CommandPool.o \
	../../build/swapchain.o \
	../../build/PhysicalDeviceSupport.o

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
