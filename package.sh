#!/bin/bash
set -e

UNAME_S=$(uname -s)
if [[ "$UNAME_S" == MINGW* ]] || [[ "$OS" == "Windows_NT" && -z "$UNAME_S" ]]; then
    PLATFORM="Windows"
    EXE_NAME="NotARhythmGame.exe"
    DIST_DIR="NotARhythmGame-Windows"
    ARCHIVE_NAME="NotARhythmGame-Windows.zip"
else
    PLATFORM="Linux"
    EXE_NAME="NotARhythmGame"
    DIST_DIR="NotARhythmGame-Linux"
    ARCHIVE_NAME="NotARhythmGame-Linux.tar.gz"
fi

make fclean

echo "Building for $PLATFORM..."
if [ "$PLATFORM" == "Windows" ]; then
    make re || echo "Make failed, continuing packaging..."
else
    make -C src static
fi

rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

cp "$EXE_NAME" "$DIST_DIR/"

if [ "$PLATFORM" == "Windows" ]; then
    DLLS=(
        "bass.dll" "glfw3.dll" "libwinpthread-1.dll" "libsndfile-1.dll"
        "libmpg123-0.dll" "libmp3lame-0.dll" "libogg-0.dll" "libFLAC.dll"
        "libopus-0.dll" "libvorbis-0.dll" "libvorbisenc-2.dll"
        "libgcc_s_seh-1.dll" "libstdc++-6.dll"
    )
    for dll in "${DLLS[@]}"; do
        if [ -f "$dll" ]; then
            cp "$dll" "$DIST_DIR/" && echo "$dll copied"
        else
            echo "$dll missing"
        fi
    done
    mkdir -p "$DIST_DIR/assets"
    [ -f "assets/hit.wav" ] && cp "assets/hit.wav" "$DIST_DIR/assets/"
    cat > "$DIST_DIR/run.bat" <<EOF
@echo off
cd /d "%~dp0"
NotARhythmGame.exe %*
pause
EOF
    cat > "$DIST_DIR/README.txt" <<EOF
Not A Rhythm Game
Run with run.bat or double-click the EXE.
EOF
    zip -r "$ARCHIVE_NAME" "$DIST_DIR"
else
    mkdir -p "$DIST_DIR/assets"
    [ -f "assets/hit.wav" ] && cp "assets/hit.wav" "$DIST_DIR/assets/"
    cat > "$DIST_DIR/run.sh" <<EOF
#!/bin/bash
DIR=\$(dirname "\$0")
"\$DIR/$EXE_NAME" "\$@"
EOF
    chmod +x "$DIST_DIR/run.sh"
    cat > "$DIST_DIR/README.txt" <<EOF
Not A Rhythm Game
Run with ./NotARhythmGame or ./run.sh
EOF
    tar czf "$ARCHIVE_NAME" "$DIST_DIR"
fi

echo "Package created: $ARCHIVE_NAME"
