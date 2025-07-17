#pragma once

#include "imgui.h"
#include "SoundManager.hpp"
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>

#define BASS_DEFAULT_DEVICE -1
#define BASS_MAX_FREQUENCY 44100
#define BASS_MIN_FREQUENCY 0

namespace App {
namespace Windows {

    struct Note {
        int id;
        int lane; // 0 = top, 1 = bottom // TODO: Use enum
        double timestamp;
    };

    class NodeManager { // TODO: Move into Core namespace
    public:
        NodeManager();
        int addNote(int lane, double timestamp);
        void removeNote(int id);
        void moveNote(int id, int newLane, double newTimestamp);
        Note* getNoteById(int id);
        std::vector<Note>& getNotes();
        void clear();
        int getNextId() const;
    private:
        std::vector<Note> notes;
        int nextId;
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

            NodeManager nodeManager;
            int selectedNoteId;
            int hoveredNoteId;
            std::vector<int> selectedNoteIds;

            bool showNotesList;
            bool showProperties;
            bool snapToGrid;
            bool showNoteIds;
            float noteRadius;
            int sortOrder; // 0 = by time, 1 = by lane, 2 = by id // TODO: Use enum

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
