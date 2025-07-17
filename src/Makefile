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
LINUX_GL_LIBS = -lGL
CXXFLAGS = -std=c++17 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I../include
CXXFLAGS += -g -Wall -Wformat
LIBS =
##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------
## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS += -DIMGUI_IMPL_OPENGL_ES2
# LINUX_GL_LIBS = -lGLESv2
##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------
ifeq ($(UNAME_S), Linux) #LINUX
    ECHO_MESSAGE = "Linux"
    LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3` -L../libraries -lbass
    LIBS += -lsndfile -lm -lpthread
    LIBS += -Wl,-rpath,'$$ORIGIN/libraries'
    CXXFLAGS += `pkg-config --cflags glfw3`
    CFLAGS = $(CXXFLAGS)
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

clean:
	rm -f $(EXE) $(OBJS)

.PHONY: all clean static embed-assets

##---------------------------------------------------------------------
## STATIC BUILD (Self-contained binary)
##---------------------------------------------------------------------
static: CXXFLAGS += -static-libgcc -static-libstdc++
static: LIBS = $(LINUX_GL_LIBS) -lglfw -L../libraries -lbass -lsndfile -lm -lpthread -ldl -lX11 -lXrandr -lXinerama -lXcursor -lXxf86vm -lXfixes
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
