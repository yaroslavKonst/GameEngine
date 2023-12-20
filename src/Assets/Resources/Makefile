ENTRIES=\
	Fonts/DroidSans.ttf \
	Locale/en \
	Images/transparent.png \
	Images/Cross.png \
	Images/Grass.png \
	Images/White.png \
	Images/Circle.png \
	Skybox/skybox.png \
	Skybox/skybox_night.png \
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
	Models/Ship/ShipDataCableHub.png \
	Models/Ship/MainBlocks/Wall.obj \
	Models/Ship/MainBlocks/Wall.png \
	Models/Ship/MainBlocks/FlightControl.obj \
	Models/Ship/MainBlocks/FlightControl.png \
	Models/Ship/MainBlocks/StaticThruster.obj \
	Models/Ship/MainBlocks/StaticThruster.png \
	Models/Ship/MainBlocks/StaticThrusterCollision.obj \
	Models/Ship/MainBlocks/DynamicThrusterBase.obj \
	Models/Ship/MainBlocks/DynamicThrusterBase.png \
	Models/Ship/MainBlocks/DynamicThruster.obj \
	Models/Ship/MainBlocks/DynamicThruster.png \
	Models/Shuttle/Base.obj \
	Models/Shuttle/Base.png \
	Models/Shuttle/Roof.obj \
	Models/Shuttle/Roof.png \
	Models/Shuttle/Thruster.obj \
	Models/Shuttle/Thruster.png \
	Models/Shuttle/ThrusterExh.obj \
	Models/Shuttle/ThrusterExh.png \
	Models/Shuttle/Wing.obj \
	Models/Shuttle/Wing.png

.PHONY: all

all: ../../../build/resources.bin

../../../build/resources.bin: ../../../build/packer $(ENTRIES)
	../../../build/packer $@ $(ENTRIES)
