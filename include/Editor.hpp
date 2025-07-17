#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <array>
#include <tuple>

#include "imgui.h"

#include "SoundManager.hpp"
#include "NodeManager.hpp"

#define BASS_DEFAULT_DEVICE -1
#define BASS_MAX_FREQUENCY 44100
#define BASS_MIN_FREQUENCY 0

#define TIMELINE_OFFSET 4.0f

namespace App {
namespace Windows {

    struct SeekPerZoom {
        static constexpr std::array<std::tuple<float, float>, 4> data = {
            std::tuple<float, float>{1.0f, 5.0f},
            std::tuple<float, float>{5.0f, 2.5f},
            std::tuple<float, float>{10.0f, 1.0f},
            std::tuple<float, float>{15.0f, 0.250f},
        };
    };

    enum SortOrder {
        TIME = 0,
        LANE = 1,
        ID = 2,
    };

    class Editor {
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
            float markerInterval;
            float gridSpacing; // in seconds, calculated from BPM

            // UI state
            float timelineWidth;
            float timelineHeight;
            float zoomLevel;
            float scrollOffset;
            float targetScrollOffset; // For smooth scrolling
            float minZoomLevel;
            float maxZoomLevel;

            bool showFileDialog;
            std::string currentDirectory;
            std::vector<std::string> directories;
            std::vector<std::string> files;
            int selectedFileIndex;

            App::Core::NodeManager nodeManager;
            int selectedNoteId;
            int hoveredNoteId;
            std::vector<int> selectedNoteIds;

            bool showNotesList;
            bool showProperties;
            bool snapToGrid;
            bool showNoteIds;
            bool showMilliseconds;
            float noteRadius;
            SortOrder sortOrder;

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
            void drawTimelineLanes();
            void handleNotePlacementAndInteraction();
            void drawNotesList();
            void jumpToPosition(Core::Note* note);
            void drawPropertiesPanel();
            void sortNotes();

        public:
            Editor();
            ~Editor() = default;

            void render();
            void update();
    };

} // Windows
} // App
