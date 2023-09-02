ENTRIES=\
	Fonts/DroidSans.ttf \
	Images/transparent.png \
	Images/Cross.png \
	Skybox/skybox.png \
	Models/Archive/field.obj \
	Models/Archive/floor.jpg \
	Models/Player/Collision.obj \
	Models/Ship/ShipFloor.obj \
	Models/Ship/ShipFloor.png \
	Models/Ship/ShipFloorComm.obj \
	Models/Ship/ShipFloorComm.png \
	Models/Ship/ShipFloorCommDelim.obj \
	Models/Ship/ShipFloorCommDelim.png \
	Models/Ship/ShipFloorCollision.obj \
	Models/Ship/ShipPowerCable.obj \
	Models/Ship/ShipPowerCable.png \
	Models/Ship/ShipPowerCableHub.obj \
	Models/Ship/ShipPowerCableHub.png \
	Models/Ship/ShipDataCable.obj \
	Models/Ship/ShipDataCable.png \
	Models/Ship/ShipDataCableHub.obj \
	Models/Ship/ShipDataCableHub.png

.PHONY: all

all: ../../../build/package.bin

../../../build/package.bin: $(ENTRIES)
	../../../build/packer ../../../build/package.bin $(ENTRIES)
