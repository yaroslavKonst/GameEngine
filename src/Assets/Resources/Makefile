ENTRIES=\
	Fonts/DroidSans.ttf \
	Images/transparent.png \
	Skybox/skybox.png \
	Models/Archive/field.obj \
	Models/Archive/floor.jpg \
	Models/Ship/ShipFloorComm.obj \
	Models/Ship/ShipFloorComm.png

.PHONY: all

all: ../../../build/package.bin

../../../build/package.bin: $(ENTRIES)
	../../../build/packer ../../../build/package.bin $(ENTRIES)
