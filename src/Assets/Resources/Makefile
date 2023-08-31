ENTRIES=\
	Fonts/DroidSans.ttf \
	Images/transparent.png \
	Skybox/skybox.png \
	Models/Archive/field.obj \
	Models/Archive/floor.jpg \
	Models/Ship/ShipFloor.obj \
	Models/Ship/ShipFloor.png \
	Models/Ship/ShipFloorComm.obj \
	Models/Ship/ShipFloorComm.png \
	Models/Ship/ShipFloorCommDelim.obj \
	Models/Ship/ShipFloorCommDelim.png \
	Models/Ship/ShipFloorCollision.obj \
	Models/Ship/ShipPowerCableHub.obj \
	Models/Ship/ShipPowerCableHub.png

.PHONY: all

all: ../../../build/package.bin

../../../build/package.bin: $(ENTRIES)
	../../../build/packer ../../../build/package.bin $(ENTRIES)
