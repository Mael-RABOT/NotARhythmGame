#pragma once

#include "imgui.h"
#include "SoundManager.hpp"
#include <string>
#include <memory>
#include <vector>

namespace App {
namespace Windows {

    class Timeline {
        private:
            // Audio management
            std::unique_ptr<Core::SoundManager> soundManager;
            std::string currentSongPath;
            std::string currentSongName;
            bool isSongLoaded;
            bool isPlaying;
            double currentPosition; // in seconds
            double songDuration; // in seconds

            // Timeline configuration
            float bpm;
            bool showGrid;
            bool enableAutoscroll;
            float gridSpacing; // in seconds, calculated from BPM

            // UI state
            float timelineWidth;
            float timelineHeight;
            float zoomLevel;
            float scrollOffset;
            float targetScrollOffset; // For smooth scrolling
            float minZoomLevel;
            float maxZoomLevel;

            // File dialog
            bool showFileDialog;
            std::string currentDirectory;
            std::vector<std::string> directories;
            std::vector<std::string> files;
            int selectedFileIndex;

            void loadSong(const std::string& filepath);
            void updatePlayback();
            void handleKeyboardInput();
            void drawTimelineGrid();
            void drawPlaybackCursor();
            void drawTimelineRuler();
            void calculateGridSpacing();
            void updateAutoscroll();
            void refreshFileList();
            void navigateToDirectory(const std::string& dirName);

        public:
            Timeline();
            ~Timeline() = default;

            void render();
            void update();
    };

} // Windows
} // App
