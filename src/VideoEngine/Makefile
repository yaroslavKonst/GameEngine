.PHONY: all shaders gui

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
	../../build/drawable.o \
	../../build/InputControl.o \
	../../build/TextureHandler.o \
	../../build/TextHandler.o \
	gui

shaders:
	cd shaders ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)

gui:
	cd GUI ; make CC=$(CC) CC_OPTS="$(CC_OPTS)" CC_OBJ=$(CC_OBJ)

../../build/%.o: %.cpp %.h
	$(CC) $(CC_OPTS) $(CC_OBJ) -o $@ $<
