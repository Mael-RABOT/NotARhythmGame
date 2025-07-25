#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
# Mac OS X:
#   brew install glfw
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
# Windows (MSYS2/MinGW):
#   pacman -S mingw-w64-x86_64-glfw mingw-w64-x86_64-libsndfile
#
#CXX = g++
#CXX = clang++
EXE = ../NotARhythmGame
IMGUI_DIR = ../imgui
SOURCES = main.cpp App.cpp Editor.cpp SoundManager.cpp NodeManager.cpp AudioAnalyzer.cpp Player.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)
CXXFLAGS = -std=c++17 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I../include
ifeq ($(UNAME_S), Darwin)
	CXXFLAGS += `pkg-config glfw3 --cflags-only-I` `pkg-config sndfile --cflags-only-I`
endif

CXXFLAGS += -g -Wall -Wformat -Wno-reorder
LIBS =

##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------
## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS += -DIMGUI_IMPL_OPENGL_ES2
# GL_LIBS = -lGLESv2

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------
ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	GL_LIBS = -lGL
	LIBS += $(GL_LIBS) `pkg-config --static --libs glfw3` -L../libraries -lbass
	LIBS += -lsndfile -lm -lpthread
	LIBS += -Wl,-rpath,'$$ORIGIN/libraries'
	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #MAC OS X
	ECHO_MESSAGE = "Mac OS X"
	GL_LIBS = -framework OpenGL
	LIBS += $(GL_LIBS) `pkg-config --static --libs glfw3` -L../libraries -L/opt/homebrew/lib -lbass
	LIBS += -lsndfile -lm -lpthread
	LIBS += -Wl,-rpath,@loader_path/libraries
	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifneq (,$(findstring MINGW,$(UNAME_S))) #WINDOWS (MSYS2/MinGW)
	ECHO_MESSAGE = "MinGW"
	EXE = ../NotARhythmGame.exe
	GL_LIBS = -lopengl32
	LIBS += $(GL_LIBS) -lglfw3 -L../libraries -lbass -lsndfile
	LIBS += -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -luuid -ladvapi32 -lshell32
	LIBS += -static-libgcc -static-libstdc++
	CXXFLAGS += -DWIN32_LEAN_AND_MEAN -D_USE_MATH_DEFINES
	CFLAGS = $(CXXFLAGS)
endif

# Alternative Windows detection (if the above doesn't work)
ifeq ($(OS), Windows_NT)
	ifeq ($(UNAME_S),)
		ECHO_MESSAGE = "Windows"
		EXE = ../NotARhythmGame.exe
		GL_LIBS = -lopengl32
		LIBS += $(GL_LIBS) -lglfw3 -L../libraries -lbass -lsndfile
		LIBS += -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -luuid -ladvapi32 -lshell32
		CXXFLAGS += -DWIN32_LEAN_AND_MEAN -D_USE_MATH_DEFINES
		CFLAGS = $(CXXFLAGS)
	endif
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)
ifneq (,$(findstring MINGW,$(UNAME_S)))
	@echo "Copying required DLLs..."
	@cp -f ../libraries/bass.dll .. 2>/dev/null || echo "bass.dll not found in ../libraries/"
	@cp -f /mingw64/bin/glfw3.dll .. 2>/dev/null || echo "glfw3.dll not found"
	@cp -f /mingw64/bin/libwinpthread-1.dll .. 2>/dev/null || echo "libwinpthread-1.dll not found"
	@cp -f /mingw64/bin/libsndfile-1.dll .. 2>/dev/null || echo "libsndfile-1.dll not found"
	@cp -f /mingw64/bin/libmpg123-0.dll .. 2>/dev/null || echo "libmpg123-0.dll not found"
	@cp -f /mingw64/bin/libmp3lame-0.dll .. 2>/dev/null || echo "libmp3lame-0.dll not found"
	@cp -f /mingw64/bin/libogg-0.dll .. 2>/dev/null || echo "libogg-0.dll not found"
	@cp -f /mingw64/bin/libFLAC.dll .. 2>/dev/null || echo "libFLAC.dll not found"
	@cp -f /mingw64/bin/libopus-0.dll .. 2>/dev/null || echo "libopus-0.dll not found"
	@cp -f /mingw64/bin/libvorbis-0.dll .. 2>/dev/null || echo "libvorbis-0.dll not found"
	@cp -f /mingw64/bin/libvorbisenc-2.dll .. 2>/dev/null || echo "libvorbisenc-2.dll not found"
	@cp -f /mingw64/bin/libgcc_s_seh-1.dll .. 2>/dev/null || echo "libgcc_s_seh-1.dll not found"
	@cp -f /mingw64/bin/libstdc++-6.dll .. 2>/dev/null || echo "libstdc++-6.dll not found"
	@echo "DLL copying complete"
endif

ifeq ($(OS), Windows_NT)
	ifeq ($(UNAME_S),)
		@echo "Copying required DLLs..."
		@cp -f ../libraries/bass.dll .. 2>/dev/null || echo "bass.dll not found in ../libraries/"
		@cp -f /mingw64/bin/glfw3.dll .. 2>/dev/null || echo "glfw3.dll not found"
		@cp -f /mingw64/bin/libwinpthread-1.dll .. 2>/dev/null || echo "libwinpthread-1.dll not found"
		@cp -f /mingw64/bin/libsndfile-1.dll .. 2>/dev/null || echo "libsndfile-1.dll not found"
		@cp -f /mingw64/bin/libmpg123-0.dll .. 2>/dev/null || echo "libmpg123-0.dll not found"
		@cp -f /mingw64/bin/libmp3lame-0.dll .. 2>/dev/null || echo "libmp3lame-0.dll not found"
		@cp -f /mingw64/bin/libogg-0.dll .. 2>/dev/null || echo "libogg-0.dll not found"
		@cp -f /mingw64/bin/libFLAC.dll .. 2>/dev/null || echo "libFLAC.dll not found"
		@cp -f /mingw64/bin/libopus-0.dll .. 2>/dev/null || echo "libopus-0.dll not found"
		@cp -f /mingw64/bin/libvorbis-0.dll .. 2>/dev/null || echo "libvorbis-0.dll not found"
		@cp -f /mingw64/bin/libvorbisenc-2.dll .. 2>/dev/null || echo "libvorbisenc-2.dll not found"
		@cp -f /mingw64/bin/libgcc_s_seh-1.dll .. 2>/dev/null || echo "libgcc_s_seh-1.dll not found"
		@cp -f /mingw64/bin/libstdc++-6.dll .. 2>/dev/null || echo "libstdc++-6.dll not found"
		@echo "DLL copying complete"
	endif
endif


clean:
	rm -f $(EXE) $(OBJS)
	rm -f ../*.dll

##---------------------------------------------------------------------
## COPY DLLS (Windows only)
##---------------------------------------------------------------------
copy-dlls:
ifneq (,$(findstring MINGW,$(UNAME_S)))
	@echo "Copying required DLLs..."
	@cp -f ../libraries/bass.dll .. 2>/dev/null || echo "bass.dll not found in ../libraries/"
	@cp -f /mingw64/bin/glfw3.dll .. 2>/dev/null || echo "glfw3.dll not found"
	@cp -f /mingw64/bin/libwinpthread-1.dll .. 2>/dev/null || echo "libwinpthread-1.dll not found"
	@cp -f /mingw64/bin/libsndfile-1.dll .. 2>/dev/null || echo "libsndfile-1.dll not found"
	@cp -f /mingw64/bin/libmpg123-0.dll .. 2>/dev/null || echo "libmpg123-0.dll not found"
	@cp -f /mingw64/bin/libmp3lame-0.dll .. 2>/dev/null || echo "libmp3lame-0.dll not found"
	@cp -f /mingw64/bin/libogg-0.dll .. 2>/dev/null || echo "libogg-0.dll not found"
	@cp -f /mingw64/bin/libFLAC.dll .. 2>/dev/null || echo "libFLAC.dll not found"
	@cp -f /mingw64/bin/libopus-0.dll .. 2>/dev/null || echo "libopus-0.dll not found"
	@cp -f /mingw64/bin/libvorbis-0.dll .. 2>/dev/null || echo "libvorbis-0.dll not found"
	@cp -f /mingw64/bin/libvorbisenc-2.dll .. 2>/dev/null || echo "libvorbisenc-2.dll not found"
	@cp -f /mingw64/bin/libgcc_s_seh-1.dll .. 2>/dev/null || echo "libgcc_s_seh-1.dll not found"
	@cp -f /mingw64/bin/libstdc++-6.dll .. 2>/dev/null || echo "libstdc++-6.dll not found"
	@echo "DLL copying complete"
endif
ifeq ($(OS), Windows_NT)
	ifeq ($(UNAME_S),)
	@echo "Copying required DLLs..."
	@cp -f ../libraries/bass.dll .. 2>/dev/null || echo "bass.dll not found in ../libraries/"
	@cp -f /mingw64/bin/glfw3.dll .. 2>/dev/null || echo "glfw3.dll not found"
	@cp -f /mingw64/bin/libwinpthread-1.dll .. 2>/dev/null || echo "libwinpthread-1.dll not found"
	@cp -f /mingw64/bin/libsndfile-1.dll .. 2>/dev/null || echo "libsndfile-1.dll not found"
	@cp -f /mingw64/bin/libmpg123-0.dll .. 2>/dev/null || echo "libmpg123-0.dll not found"
	@cp -f /mingw64/bin/libmp3lame-0.dll .. 2>/dev/null || echo "libmp3lame-0.dll not found"
	@cp -f /mingw64/bin/libogg-0.dll .. 2>/dev/null || echo "libogg-0.dll not found"
	@cp -f /mingw64/bin/libFLAC.dll .. 2>/dev/null || echo "libFLAC.dll not found"
	@cp -f /mingw64/bin/libopus-0.dll .. 2>/dev/null || echo "libopus-0.dll not found"
	@cp -f /mingw64/bin/libvorbis-0.dll .. 2>/dev/null || echo "libvorbis-0.dll not found"
	@cp -f /mingw64/bin/libvorbisenc-2.dll .. 2>/dev/null || echo "libvorbisenc-2.dll not found"
	@cp -f /mingw64/bin/libgcc_s_seh-1.dll .. 2>/dev/null || echo "libgcc_s_seh-1.dll not found"
	@cp -f /mingw64/bin/libstdc++-6.dll .. 2>/dev/null || echo "libstdc++-6.dll not found"
	@echo "DLL copying complete"
	endif
endif

.PHONY: all clean static embed-assets copy-dlls

##---------------------------------------------------------------------
## STATIC BUILD (Self-contained binary)
##---------------------------------------------------------------------
static: CXXFLAGS += -static-libgcc -static-libstdc++
static: LIBS = $(GL_LIBS) -lglfw -L../libraries -lbass -lsndfile -lm -lpthread -ldl -lX11 -lXrandr -lXinerama -lXcursor -lXxf86vm -lXfixes
static: $(EXE)
	@echo Static build complete for $(ECHO_MESSAGE)
	@echo "Binary is now self-contained and can be distributed without external dependencies"

##---------------------------------------------------------------------
## EMBED ASSETS (Embed hit sound into binary)
##---------------------------------------------------------------------
embed-assets: $(EXE)
	@echo "Embedding assets into binary..."
	objcopy --add-section .assets=../assets/hit.wav $(EXE)
	@echo "Assets embedded successfully"