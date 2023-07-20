.PHONY: all shaders

all: \
	shaders \
	../../build/window.o \
	../../build/VkInstanceHandler.o \
	../../build/video.o \
	../../build/MemoryManager.o \
	../../build/CommandPool.o \
	../../build/swapchain.o \
	../../build/PhysicalDeviceSupport.o \
	../../build/MemorySystem.o \
	../../build/ImageHelper.o \
	../../build/pipeline.o \
	../../build/model.o \
	../../build/ModelDescriptor.o \
	../../build/BufferHelper.o \
	../../build/rectangle.o \
	../../build/drawable.o

shaders:
	cd shaders ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
