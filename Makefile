BUILD_DIR ?= build
CMAKE ?= cmake

.PHONY: all gd simplex clean

all:
	$(CMAKE) -S . -B $(BUILD_DIR)
	$(CMAKE) --build $(BUILD_DIR)

gd:
	$(CMAKE) -S . -B $(BUILD_DIR)
	$(CMAKE) --build $(BUILD_DIR) --target gd1d gd2d

simplex:
	$(CMAKE) -S . -B $(BUILD_DIR)
	$(CMAKE) --build $(BUILD_DIR) --target simplex_cli

clean:
	rm -rf $(BUILD_DIR)
