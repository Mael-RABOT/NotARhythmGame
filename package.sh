#!/bin/bash

# Package script for Not A Rhythm Game
# This creates a self-contained distribution

set -e

echo "Building Not A Rhythm Game distribution..."

# Build the static binary
echo "Building static binary..."
make -C src static

# Create distribution directory
DIST_DIR="NotARhythmGame-Dist"
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Copy the binary
cp NotARhythmGame "$DIST_DIR/"

# Copy required libraries (if not statically linked)
if [ -f "libraries/libbass.so" ]; then
    mkdir -p "$DIST_DIR/libraries"
    cp libraries/libbass.so "$DIST_DIR/libraries/"
fi

# Copy assets
mkdir -p "$DIST_DIR/assets"
cp assets/hit.wav "$DIST_DIR/assets/"

# Create a launcher script
cat > "$DIST_DIR/run.sh" << 'EOF'
#!/bin/bash
# Launcher script for Not A Rhythm Game

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set library path to include local libraries
export LD_LIBRARY_PATH="$SCRIPT_DIR/libraries:$LD_LIBRARY_PATH"

# Run the game
cd "$SCRIPT_DIR"
./NotARhythmGame "$@"
EOF

chmod +x "$DIST_DIR/run.sh"

# Create README
cat > "$DIST_DIR/README.txt" << 'EOF'
Not A Rhythm Game - Distribution Package

This is a self-contained distribution of Not A Rhythm Game.

To run the game:
1. Make sure you have execute permissions: chmod +x run.sh
2. Run: ./run.sh

Or run directly: ./NotARhythmGame

Requirements:
- Linux with X11 support
- OpenGL compatible graphics card

The game will create a recent_charts.txt file to remember your recently played charts.

For support or issues, please refer to the original project repository.
EOF

# Create archive
echo "Creating distribution archive..."
tar -czf "NotARhythmGame-Linux.tar.gz" "$DIST_DIR"

echo "Distribution created successfully!"
echo "Files:"
echo "  - NotARhythmGame-Linux.tar.gz (distribution archive)"
echo "  - $DIST_DIR/ (unpacked distribution)"
echo ""
echo "To distribute:"
echo "  1. Send the .tar.gz file to users"
echo "  2. Users extract: tar -xzf NotARhythmGame-Linux.tar.gz"
echo "  3. Users run: cd NotARhythmGame-Dist && ./run.sh"