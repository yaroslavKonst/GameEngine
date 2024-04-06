ENTRIES=\
	Fonts/DroidSans.ttf \
	Locale/en \
	Menu/ButtonBackground.png \
	World/Player.png \
	World/EmptyCell.png \
	World/WallCell.png \
	World/FinishCell.png \
	World/Levels/Level.txt

$(BUILD_DIR)/resources.bin: $(BUILD_DIR)/packer $(ENTRIES)
	$(BUILD_DIR)/packer $@ $(ENTRIES)
