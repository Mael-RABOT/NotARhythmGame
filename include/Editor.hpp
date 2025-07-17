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
#include <fstream>
#include <cstring>
#include <filesystem>

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

    struct ChartHeader {
        char magic[12];        // "NOTARHYTHM" (11 chars + null terminator)
        uint32_t version;      // File format version
        uint32_t headerSize;   // Size of this header
        uint32_t audioSize;    // Size of embedded audio data
        uint32_t notesCount;   // Number of notes
        char title[256];       // Song title
        char artist[256];      // Artist name
        float bpm;             // Beats per minute
        double duration;       // Song duration in seconds
        uint32_t reserved[16]; // Reserved for future use
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

            // Chart file management
            bool showSaveDialog;
            bool showLoadDialog;
            std::string chartTitle;
            std::string chartArtist;

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

            // Chart file operations
            bool saveChartFile(const std::string& filepath);
            bool loadChartFile(const std::string& filepath);
            std::vector<char> readAudioFile(const std::string& filepath);
            bool writeAudioFile(const std::string& filepath, const std::vector<char>& audioData);
            void extractAudioFromChart(const std::string& chartPath, const std::string& outputPath);

        public:
            Editor();
            ~Editor() = default;

            void render();
            void update();
    };

} // Windows
} // App
