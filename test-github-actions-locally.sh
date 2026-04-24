#!/bin/bash

# Test script to simulate GitHub Actions workflow locally
# This mimics the key steps from .github/workflows/compile.yml

set -e

echo "=== Local GitHub Actions Test ==="
echo "Testing M5Stick-NEMO compilation with GitHub Actions dependencies"
echo

# Configuration (matching GitHub Actions)
BOARD_CONFIGS=(
    "M5StickCPlus2:m5stack:esp32:m5stack_stickc_plus2:M5StickCPlus2@1.0.1 IRremote M5Stack-SD-Updater M5Unified@0.2.8"
    "M5StickCPlus:m5stack:esp32:m5stack_stickc_plus:M5StickCPlus@0.1.1 IRremote M5Stack-SD-Updater M5Unified@0.2.8"
    "M5Cardputer:m5stack:esp32:m5stack_cardputer:M5Cardputer@1.1.0 IRremote M5Stack-SD-Updater M5Unified@0.2.8"
    "M5StickC:m5stack:esp32:m5stack_stickc:M5StickC@0.3.0 IRremote M5Stack-SD-Updater M5Unified@0.2.8"
)

# Choose which board to test (default: M5StickCPlus2)
BOARD_TO_TEST=${1:-"M5StickCPlus2"}

echo "Looking for board configuration: $BOARD_TO_TEST"

# Find the board configuration
BOARD_CONFIG=""
for config in "${BOARD_CONFIGS[@]}"; do
    if [[ $config == $BOARD_TO_TEST:* ]]; then
        BOARD_CONFIG=$config
        break
    fi
done

if [ -z "$BOARD_CONFIG" ]; then
    echo "Error: Board $BOARD_TO_TEST not found!"
    echo "Available boards: M5StickCPlus2, M5StickCPlus, M5Cardputer, M5StickC"
    exit 1
fi

# Parse configuration
IFS=':' read -r BOARD_NAME FQBN_PART1 FQBN_PART2 FQBN_PART3 LIBRARIES <<< "$BOARD_CONFIG"
FQBN="$FQBN_PART1:$FQBN_PART2:$FQBN_PART3"
echo "Board: $BOARD_NAME"
echo "FQBN: $FQBN"  
echo "Libraries: $LIBRARIES"
echo

# Step 1: Update Arduino CLI core index
echo "=== Step 1: Update Arduino CLI core index ==="
arduino-cli core update-index --additional-urls "https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json"

# Step 2: Install/Update M5Stack ESP32 core 3.2.2
echo
echo "=== Step 2: Install M5Stack ESP32 core 3.2.2 ==="
arduino-cli core install m5stack:esp32@3.2.2 --additional-urls "https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json" || echo "Core already installed"

# Step 3: Install required libraries  
echo
echo "=== Step 3: Install libraries ==="
echo "Installing: $LIBRARIES"
arduino-cli lib install $LIBRARIES --log-level warn --verbose

# Step 4: Compile the sketch
echo
echo "=== Step 4: Compile sketch ==="
echo "Compiling for: $FQBN"

# Set version and language (simulating GitHub Actions environment)
VERSION="local-test"
LOCALE="en-us"
LANGUAGE=$(echo "LANGUAGE_${LOCALE//-/_}" | tr '[:lower:]' '[:upper:]')

# Set board-specific flags
case $BOARD_NAME in
    "M5StickCPlus2") EXTRA_FLAGS="-DSTICK_C_PLUS2" ;;
    "M5StickCPlus")  EXTRA_FLAGS="-DSTICK_C_PLUS" ;;
    "M5Cardputer")   EXTRA_FLAGS="-DCARDPUTER" ;;
    "M5StickC")      EXTRA_FLAGS="-DSTICK_C" ;;
    *) EXTRA_FLAGS="" ;;
esac

EXTRA_FLAGS="$EXTRA_FLAGS -DNEMO_VERSION=\"$VERSION\" -D$LANGUAGE"

echo "Extra flags: $EXTRA_FLAGS"
echo

# Run compilation
arduino-cli compile --fqbn "$FQBN" -e \
                    --build-property build.partitions=huge_app \
                    --build-property upload.maximum_size=3145728 \
                    --build-property "compiler.cpp.extra_flags=$EXTRA_FLAGS" \
                    ./m5stick-nemo.ino

echo
echo "=== Compilation completed successfully! ==="
echo "Binary location: build/$FQBN/"

# Show build results
if [ -f "build/${FQBN//:/.}/m5stick-nemo.ino.bin" ]; then
    echo "Firmware binary: build/${FQBN//:/.}/m5stick-nemo.ino.bin"
    ls -la "build/${FQBN//:/.}"/m5stick-nemo.ino.*
fi