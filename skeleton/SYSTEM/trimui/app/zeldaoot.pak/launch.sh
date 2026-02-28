#!/bin/sh
# Zelda Ocarina of Time - RT64ES Launcher

APP_DIR="/mnt/SDCARD/Emus/ZeldaOOT"
ROM_PATH="${ZELDA_ROM_PATH:-/mnt/SDCARD/Roms/N64/zelda_oot.n64}"
SAVE_DIR="/mnt/SDCARD/Saves/N64/ZeldaOOT"
CONFIG_DIR="/mnt/SDCARD/Config/ZeldaOOT"

# Create directories if they don't exist
mkdir -p "$SAVE_DIR"
mkdir -p "$CONFIG_DIR"

# Set environment variables
export ZELDA_ROM_PATH="$ROM_PATH"
export ZELDA_SAVE_DIR="$SAVE_DIR"
export ZELDA_CONFIG_DIR="$CONFIG_DIR"

# Check if ROM exists
if [ ! -f "$ROM_PATH" ]; then
    echo "ERROR: ROM file not found: $ROM_PATH"
    echo "Please place your Zelda OOT ROM at: $ROM_PATH"
    sleep 3
    exit 1
fi

# Change to app directory
cd "$APP_DIR" || exit 1

# Launch the game
echo "Starting Zelda Ocarina of Time..."
echo "ROM: $ROM_PATH"
echo "Save: $SAVE_DIR"

./zeldaoot "$ROM_PATH" || {
    echo "ERROR: Failed to launch zeldaoot"
    sleep 3
    exit 1
}

exit 0
