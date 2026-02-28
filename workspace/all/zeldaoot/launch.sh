#!/bin/sh
# Zelda Ocarina of Time - RT64ES Launcher

APP_DIR="/mnt/SDCARD/Emus/ZeldaOOT"
ROM_PATH="${ZELDA_ROM_PATH:-/mnt/SDCARD/Roms/N64/zelda_oot.n64}"
SAVE_DIR="/mnt/SDCARD/Saves/N64/ZeldaOOT"
CONFIG_DIR="/mnt/SDCARD/Config/ZeldaOOT"
KEYMAP_FILE="$CONFIG_DIR/keymap.json"
MAPPING_CONFIG_FILE="$CONFIG_DIR/mapping_config.json"

# Create directories if they don't exist
mkdir -p "$SAVE_DIR"
mkdir -p "$CONFIG_DIR"

# Copy default keymap if not exists
if [ ! -f "$KEYMAP_FILE" ]; then
    cp "$APP_DIR/keymaps.json" "$KEYMAP_FILE" 2>/dev/null || true
fi

# Copy default mapping config if not exists
if [ ! -f "$MAPPING_CONFIG_FILE" ]; then
    cp "$APP_DIR/mapping_config.json" "$MAPPING_CONFIG_FILE" 2>/dev/null || true
fi

# Load user preferences
CONTROL_SCHEME="${CONTROL_SCHEME:-Default}"
RUMBLE="${RUMBLE:-true}"
SHOW_FPS="${SHOW_FPS:-false}"
SPEED="${SPEED:-100%}"
ENABLE_COMBO_HINTS="${ENABLE_COMBO_HINTS:-true}"

# Set environment variables
export ZELDA_ROM_PATH="$ROM_PATH"
export ZELDA_SAVE_DIR="$SAVE_DIR"
export ZELDA_CONFIG_DIR="$CONFIG_DIR"
export ZELDA_KEYMAP="$KEYMAP_FILE"
export ZELDA_MAPPING_CONFIG="$MAPPING_CONFIG_FILE"
export ZELDA_CONTROL_SCHEME="$CONTROL_SCHEME"
export ZELDA_RUMBLE="$RUMBLE"
export ZELDA_SHOW_FPS="$SHOW_FPS"
export ZELDA_SPEED="$SPEED"
export ZELDA_UI_STYLE="minui"
export ZELDA_ENABLE_COMBO_HINTS="$ENABLE_COMBO_HINTS"

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
echo "Control Scheme: $CONTROL_SCHEME"
echo "Rumble: $RUMBLE"
echo "UI Style: minui"
echo "Combo Hints: $ENABLE_COMBO_HINTS"

./zeldaoot "$ROM_PATH" || {
    echo "ERROR: Failed to launch zeldaoot"
    sleep 3
    exit 1
}

exit 0
