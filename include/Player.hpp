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
#include <deque>

#include "imgui.h"

#include "SoundManager.hpp"
#include "NodeManager.hpp"
#include "Common.hpp"

#define BASS_DEFAULT_DEVICE -1
#define BASS_MAX_FREQUENCY 44100
#define BASS_MIN_FREQUENCY 0

#define TIMELINE_OFFSET 4.0f

#define RECENT_CHART_FILE "recent_charts.txt"

namespace App {
namespace Windows {

    enum GameState {
        MENU = 0,
        PLAYING = 1,
        PAUSED = 2,
        GAME_OVER = 3,
        RESULTS = 4,
    };

    enum Judgement {
        PERFECT = 0,
        GREAT = 1,
        GOOD = 2,
        MISS = 3,
    };

    struct GameNote {
        int id;
        Core::Lane lane;
        Core::NoteType type;
        double timestamp;
        double endTimestamp;
        bool hit;
        Judgement judgement;
        double hitTime;
        bool isActive;
        bool isHolding;
        double holdStartTime;
        bool holdCompleted;
        double holdAccuracy;
        int holdTicks;
        int totalHoldTicks;
        double lastHoldTickTime;
    };

    struct GameStats {
        int perfect;
        int great;
        int good;
        int miss;
        int combo;
        int maxCombo;
        double accuracy;
        int score;
    };

    struct HitEffect {
        ImVec2 position;
        float scale;
        float alpha;
        double startTime;
        double duration;
        Judgement judgement;
        bool active;
    };

    class Player {
        private:
            Core::SoundManager* soundManager;
            std::string currentSongPath;
            std::string currentSongName;
            bool isSongLoaded;
            bool isPlaying;
            double currentPosition; // in seconds
            double songDuration; // in seconds

            GameState gameState;
            GameStats stats;
            std::vector<GameNote> gameNotes;
            std::deque<Judgement> recentJudgements;
            double judgementWindow; // in seconds
            double perfectWindow;
            double greatWindow;
            double goodWindow;

            std::vector<HitEffect> hitEffects;
            std::string hitSoundPath;

            float bpm;
            bool showGrid;
            float markerInterval;
            float gridSpacing; // in seconds, calculated from BPM

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

            bool showNoteIds;
            bool showMilliseconds;
            float noteRadius;
            float approachTime;
            float hitZoneY;
            bool showJudgement;
            double judgementDisplayTime;
            std::string lastJudgementText;
            ImVec4 lastJudgementColor;

            std::string chartTitle;
            std::string chartArtist;

            bool fKeyPressed;
            bool jKeyPressed;
            double lastFKeyTime;
            double lastJKeyTime;
            double keyCooldown;

            ImVec2 displaySize;

            bool showGameplayWindow;
            bool showStatsWindow;
            bool showTimelineWindow;
            bool showControlsWindow;
            bool showSongInfoWindow;

            bool showChartBrowserModal;

            bool showMainMenu;
            bool showChartBrowser;
            bool showChartInfo;

            bool chartBrowserInitialized;
            std::vector<std::string> recentCharts;

            float comboScale;
            float scoreScale;
            float judgementScale;
            double effectTimer;

            float laneWidth;
            float laneHeight;
            float noteSpeed;
            float approachDistance;

            ImVec4 laneTopColor;
            ImVec4 laneBottomColor;
            ImVec4 noteTopColor;
            ImVec4 noteBottomColor;
            ImVec4 hitZoneColor;
            ImVec4 comboColor;
            ImVec4 scoreColor;

            double holdTickInterval;
            double holdMissThreshold;
            double holdBreakTime;
            bool fKeyHolding;
            bool jKeyHolding;

            bool loadSong(const std::string& filepath);
            void updatePlayback();
            void handleKeyboardInput();
            void calculateGridSpacing();
            void updateAutoscroll();
            void refreshFileList();
            void navigateToDirectory(const std::string& dirName);
            void drawJudgement();
            void updateGameLogic();
            void processInput();
            void processNoteHit(GameNote& note, double currentTime);
            void checkNoteHits();
            Judgement calculateJudgement(double hitTime, double noteTime);
            void updateStats(Judgement judgement);
            void drawResults();
            void resetGame();
            void startGame();
            void pauseGame();
            void resumeGame();

            bool loadChartFile(const std::string& filepath);
            std::vector<char> readAudioFile(const std::string& filepath);
            bool writeAudioFile(const std::string& filepath, const std::vector<char>& audioData);

            void drawGameplayWindow();
            void drawStatsWindow();
            void drawControlsWindow();
            void drawSongInfoWindow();
            void updateVisualEffects();
            void drawGameplayLanes();
            void drawApproachingNotes();
            void drawHitEffects();
            void drawComboDisplay();
            void drawScoreDisplay();
            void drawProgressBar();
            void cleanupTempFiles();
            void drawMainMenu();
            void drawChartBrowser();
            void drawChartInfo();
            void loadRecentCharts();
            void saveRecentCharts();
            void addToRecentCharts(const std::string& chartPath);
            void createHitEffect(const ImVec2& position, Judgement judgement);
            void updateHitEffects();
            void playHitSound();

            void updateHoldNotes();
            void breakHoldNote(GameNote& note, const std::string& reason);
            void completeHoldNote(GameNote& note);
            void processHoldTick(GameNote& note);
            bool isKeyHeldForLane(Core::Lane lane);

        public:
            Player();
            Player(Core::SoundManager* soundManager);
            ~Player();

            void render();
            void update();
    };

} // Windows
} // App
