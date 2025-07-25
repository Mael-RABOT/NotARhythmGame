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
#include <thread>
#include <complex>
#include <algorithm>
#include <numeric>

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

    enum SnapMode {
        NO_SNAP = 0,
        SNAP_TO_GRID = 1,
        SNAP_TO_GRID_AND_SUBGRID = 2,
    };

    class Editor {
        private:
            // Audio management
            SoundManager* soundManager;
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
            bool showSubGrid;
            int subGridDivisions;
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
            SnapMode snapMode;
            bool showNoteIds;
            bool showMilliseconds;
            float noteRadius;
            SortOrder sortOrder;

            // Chart file management
            bool showSaveDialog;
            bool showLoadDialog;
            bool showHelpWindow;
            bool showMultiNoteDialog;
            std::string chartTitle;
            std::string chartArtist;

            bool speedOverrideEnabled;
            float playbackSpeed;
            float originalPlaybackSpeed;

            // Metronome
            bool metronomeEnabled;
            double lastMetronomeBeat;
            int metronomeBeatCount;

            // Spectrum Analysis
            std::vector<float> spectrumData;
            std::vector<std::vector<float>> spectrumHistory;
            bool showSpectrum;
            int spectrumHistorySize;
            bool spectrumInitialized;
            bool metronomeSound1;

            bool showBpmFinder;
            bool bpmFinderActive;
            std::vector<double> bpmTapTimes;
            double bpmFinderStartTime;
            double lastTapTime;
            int minTapsForBpm;
            int maxTapsForBpm;

            void loadSong(const std::string& filepath);
            void updatePlayback();
            void updateMetronome();
            void handleKeyboardInput();
            void drawTimelineGrid();
            void drawPlaybackCursor();
            void drawTimelineRuler();
            void drawWaveform();
            void drawControlsWindow();
            void drawTimelineWindow();
            void drawFileBrowserPopup();
            void drawSaveChartPopup();
            void drawLoadChartPopup();
            void drawNoSongLoadedPopup();
            void drawMultiNoteDialog();
            void drawBpmFinder();
            void drawSpectrumWindow();
            void updateSpectrum();
            void calculateGridSpacing();
            float getSnapSpacing() const;
            bool shouldSnap() const;
            void updateAutoscroll();
            void refreshFileList();
            void navigateToDirectory(const std::string& dirName);
            void drawTimelineLanes();
            void handleNotePlacementAndInteraction();
            void drawNotesList();
            void drawPropertiesPanel();
            void drawHelpWindow();
            void jumpToPosition(Core::Note* note);
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
            Editor(SoundManager* soundManager);
            ~Editor() = default;

            void render();
            void update();
    };

} // Windows
} // App
