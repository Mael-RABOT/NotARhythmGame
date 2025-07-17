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
#include "Common.hpp"
#include "AudioAnalazyer.hpp"

#define BASS_DEFAULT_DEVICE -1
#define BASS_MAX_FREQUENCY 44100
#define BASS_MIN_FREQUENCY 0

#define TIMELINE_OFFSET 4.0f

#define MIN_ZOOM_LEVEL 1.0f
#define MAX_ZOOM_LEVEL 50.0f

#define WAVEFORM_HEIGHT_MULTIPLIER 2.5f

namespace App {
namespace Windows {

    enum SortOrder {
        TIME = 0,
        LANE = 1,
        ID = 2,
    };

    class Editor {
        private:
            // Audio management
            Core::SoundManager* soundManager;
            std::string currentSongPath;
            std::string currentSongName;
            bool isSongLoaded;
            bool isPlaying;
            double currentPosition; // in seconds
            double songDuration; // in seconds

            // Waveform data
            std::unique_ptr<AudioAnalyzer> audioAnalyzer;
            AudioWaveform waveformData;
            bool showWaveform;
            bool waveformLoaded;
            bool isAnalyzing;
            std::string analysisProgress;
            float analysisProgressPercent;

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
            void drawWaveform();
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
            void analyzeAudioFile(const std::string& filepath);
            void onAnalysisProgress(const AnalysisProgress& progress);

            // Chart file operations
            bool saveChartFile(const std::string& filepath);
            bool loadChartFile(const std::string& filepath);
            std::vector<char> readAudioFile(const std::string& filepath);
            bool writeAudioFile(const std::string& filepath, const std::vector<char>& audioData);
            void extractAudioFromChart(const std::string& chartPath, const std::string& outputPath);

        public:
            Editor();
            Editor(Core::SoundManager* soundManager);
            ~Editor() = default;

            void render();
            void update();
    };

} // Windows
} // App
