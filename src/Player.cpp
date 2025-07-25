#include "Player.hpp"

namespace App {
namespace Windows {

Player::Player()
    : soundManager(nullptr),
      currentSongPath(""),
      currentSongName(""),
      isSongLoaded(false),
      isPlaying(false),
      currentPosition(0.0),
      songDuration(0.0),
      gameState(MENU),
      stats({0, 0, 0, 0, 0, 0, 0.0, 0}),
      judgementWindow(0.2),
      perfectWindow(0.05),
      greatWindow(0.10),
      goodWindow(0.15),
      hitEffects(),
      hitSoundPath("assets/hit.wav"),
      bpm(120.0f),
      showGrid(true),
      markerInterval(5.0f),
      gridSpacing(0.5f),
      timelineWidth(800.0f),
      timelineHeight(200.0f),
      zoomLevel(1.0f),
      scrollOffset(0.0f),
      targetScrollOffset(0.0f),
      minZoomLevel(1.0f),
      maxZoomLevel(20.0f),
      showFileDialog(false),
      currentDirectory(""),
      selectedFileIndex(-1),
      showNoteIds(false),
      showMilliseconds(false),
      noteRadius(20.0f),
      approachTime(2.0f),
      hitZoneY(0.0f),
      showJudgement(false),
      judgementDisplayTime(0.5f),
      lastJudgementText(""),
      lastJudgementColor(ImVec4(1,1,1,1)),
      chartTitle(""),
      chartArtist(""),
      fKeyPressed(false),
      jKeyPressed(false),
      lastFKeyTime(0.0),
      lastJKeyTime(0.0),
      keyCooldown(0.1),
      displaySize(ImVec2(1920, 1080)),
      showGameplayWindow(true),
      showStatsWindow(true),
      showTimelineWindow(true),
      showControlsWindow(true),
      showSongInfoWindow(true),
      showChartBrowserModal(false),
      showMainMenu(true),
      showChartBrowser(true),
      showChartInfo(false),
      chartBrowserInitialized(false),
      recentCharts(),
      comboScale(1.0f),
      scoreScale(1.0f),
      judgementScale(1.0f),
      effectTimer(0.0),
      laneWidth(200.0f),
      laneHeight(400.0f),
      noteSpeed(300.0f),
      approachDistance(600.0f),
      laneTopColor(ImVec4(0.2f, 0.3f, 0.8f, 0.8f)),
      laneBottomColor(ImVec4(0.8f, 0.2f, 0.3f, 0.8f)),
      noteTopColor(ImVec4(0.4f, 0.6f, 1.0f, 1.0f)),
      noteBottomColor(ImVec4(1.0f, 0.4f, 0.6f, 1.0f)),
      hitZoneColor(ImVec4(1.0f, 1.0f, 0.0f, 1.0f)),
      comboColor(ImVec4(1.0f, 0.8f, 0.0f, 1.0f)),
      scoreColor(ImVec4(0.0f, 1.0f, 0.8f, 1.0f)),
      holdTickInterval(0.1),
      holdMissThreshold(0.2),
      holdBreakTime(0.0),
      fKeyHolding(false),
      jKeyHolding(false)
{
    calculateGridSpacing();
    refreshFileList();
    loadRecentCharts();
}

Player::Player(SoundManager* soundManager)
    : soundManager(soundManager),
      currentSongPath(""),
      currentSongName(""),
      isSongLoaded(false),
      isPlaying(false),
      currentPosition(0.0),
      songDuration(0.0),
      gameState(MENU),
      stats({0, 0, 0, 0, 0, 0, 0.0, 0}),
      judgementWindow(0.2),
      perfectWindow(0.05),
      greatWindow(0.10),
      goodWindow(0.15),
      hitEffects(),
      hitSoundPath("assets/hit.wav"),
      bpm(120.0f),
      showGrid(true),
      markerInterval(5.0f),
      gridSpacing(0.5f),
      timelineWidth(800.0f),
      timelineHeight(200.0f),
      zoomLevel(1.0f),
      scrollOffset(0.0f),
      targetScrollOffset(0.0f),
      minZoomLevel(1.0f),
      maxZoomLevel(20.0f),
      showFileDialog(false),
      currentDirectory(""),
      selectedFileIndex(-1),
      showNoteIds(false),
      showMilliseconds(false),
      noteRadius(20.0f),
      approachTime(2.0f),
      hitZoneY(0.0f),
      showJudgement(false),
      judgementDisplayTime(0.5f),
      lastJudgementText(""),
      lastJudgementColor(ImVec4(1,1,1,1)),
      chartTitle(""),
      chartArtist(""),
      fKeyPressed(false),
      jKeyPressed(false),
      lastFKeyTime(0.0),
      lastJKeyTime(0.0),
      keyCooldown(0.1),
      displaySize(ImVec2(1920, 1080)),
      showGameplayWindow(true),
      showStatsWindow(true),
      showTimelineWindow(true),
      showControlsWindow(true),
      showSongInfoWindow(true),
      showChartBrowserModal(false),
      showMainMenu(true),
      showChartBrowser(true),
      showChartInfo(false),
      chartBrowserInitialized(false),
      recentCharts(),
      comboScale(1.0f),
      scoreScale(1.0f),
      judgementScale(1.0f),
      effectTimer(0.0),
      laneWidth(200.0f),
      laneHeight(400.0f),
      noteSpeed(300.0f),
      approachDistance(600.0f),
      laneTopColor(ImVec4(0.2f, 0.3f, 0.8f, 0.8f)),
      laneBottomColor(ImVec4(0.8f, 0.2f, 0.3f, 0.8f)),
      noteTopColor(ImVec4(0.4f, 0.6f, 1.0f, 1.0f)),
      noteBottomColor(ImVec4(1.0f, 0.4f, 0.6f, 1.0f)),
      hitZoneColor(ImVec4(1.0f, 1.0f, 0.0f, 1.0f)),
      comboColor(ImVec4(1.0f, 0.8f, 0.0f, 1.0f)),
      scoreColor(ImVec4(0.0f, 1.0f, 0.8f, 1.0f)),
      holdTickInterval(0.1),
      holdMissThreshold(0.2),
      holdBreakTime(0.0),
      fKeyHolding(false),
      jKeyHolding(false)
{
    calculateGridSpacing();
    refreshFileList();
    loadRecentCharts();
}

void Player::render() {
    displaySize = ImGui::GetIO().DisplaySize;

    switch (gameState) {
        case MENU:
            if (showMainMenu) {
                drawMainMenu();
            }
            if (showChartBrowser) {
                drawChartBrowser();
            }
            if (showChartInfo) {
                drawChartInfo();
            }
            break;
        case PLAYING:
        case PAUSED:
            drawGameplayWindow();
            drawStatsWindow();
            drawControlsWindow();
            drawSongInfoWindow();
            drawJudgement();
            if (gameState == PAUSED) {
                // Do nothing on pause
            }
            break;
        case RESULTS:
            drawResults();
            break;
        default:
            break;
    }
}

void Player::update() {
    updatePlayback();
    handleKeyboardInput();
    updateAutoscroll();
    updateVisualEffects();
    updateHitEffects();
    if (gameState == PLAYING) {
        updateGameLogic();
        updateHoldNotes();
    }
}

bool Player::loadSong(const std::string& filepath) {
    if (!soundManager) return false;

    std::string extension = std::filesystem::path(filepath).extension().string();
    if (extension == ".chart") {
        return loadChartFile(filepath);
    } else {
        std::string songName = std::filesystem::path(filepath).filename().string();
        if (soundManager->loadSound(songName, filepath)) {
            currentSongPath = filepath;
            currentSongName = songName;
            isSongLoaded = true;
            songDuration = soundManager->getDuration(songName);
            currentPosition = 0.0;

            std::string chartPath = filepath.substr(0, filepath.find_last_of('.')) + ".chart";
            if (std::filesystem::exists(chartPath)) {
                loadChartFile(chartPath);
            }
            return true;
        }
        return false;
    }
}

void Player::updatePlayback() {
    if (!soundManager || !isSongLoaded || !isPlaying) return;

    currentPosition = soundManager->getPosition(currentSongName);

    if (currentPosition >= songDuration) {
        isPlaying = false;
        gameState = RESULTS;
    }
}

void Player::handleKeyboardInput() {
    bool fKeyJustPressed = ImGui::IsKeyPressed(ImGuiKey_F);
    bool jKeyJustPressed = ImGui::IsKeyPressed(ImGuiKey_J);
    bool fKeyJustReleased = ImGui::IsKeyReleased(ImGuiKey_F);
    bool jKeyJustReleased = ImGui::IsKeyReleased(ImGuiKey_J);

    if (fKeyJustPressed) {
        fKeyHolding = true;
        fKeyPressed = true;
        lastFKeyTime = currentPosition;
    }
    if (fKeyJustReleased) {
        fKeyHolding = false;
        fKeyPressed = false;
    }

    if (jKeyJustPressed) {
        jKeyHolding = true;
        jKeyPressed = true;
        lastJKeyTime = currentPosition;
    }
    if (jKeyJustReleased) {
        jKeyHolding = false;
        jKeyPressed = false;
    }

    if (fKeyJustPressed || jKeyJustPressed) {
        processInput();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Space) && gameState == PLAYING) {
        pauseGame();
    } else if (ImGui::IsKeyPressed(ImGuiKey_Space) && gameState == PAUSED) {
        resumeGame();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_R)) {
        resetGame();
    }
}

void Player::calculateGridSpacing() {
    float beats_per_second = bpm / 60.0f;
    float beat_duration = 1.0f / beats_per_second;
    gridSpacing = beat_duration / 4.0f;
}

bool Player::isKeyHeldForLane(Core::Lane lane) {
    if (lane == Core::Lane::TOP) {
        return fKeyHolding;
    } else {
        return jKeyHolding;
    }
}

void Player::updateAutoscroll() {
    if (gameState == PLAYING) {
        float target_x = currentPosition * zoomLevel;
        float window_center = timelineWidth * 0.5f;

        if (target_x < scrollOffset + window_center * 0.3f) {
            targetScrollOffset = target_x - window_center * 0.3f;
        } else if (target_x > scrollOffset + window_center * 0.7f) {
            targetScrollOffset = target_x - window_center * 0.7f;
        }

        scrollOffset += (targetScrollOffset - scrollOffset) * 0.1f;
    }
}

void Player::refreshFileList() {
    try {
        if (currentDirectory.empty()) {
            const char* homeDir = getenv("HOME");
            if (homeDir) {
                std::string musicDir = std::string(homeDir) + "/Music";
                if (std::filesystem::exists(musicDir)) {
                    currentDirectory = musicDir;
                } else {
                    currentDirectory = homeDir;
                }
            } else {
                currentDirectory = std::filesystem::current_path().string();
            }
        }

        if (!std::filesystem::exists(currentDirectory)) {
            currentDirectory = std::filesystem::current_path().string();
        }

        directories.clear();
        files.clear();

        for (const auto& entry : std::filesystem::directory_iterator(currentDirectory)) {
            if (entry.is_directory()) {
                directories.push_back(entry.path().filename().string());
            } else if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".chart") {
                    files.push_back(entry.path().filename().string());
                }
            }
        }

        std::sort(directories.begin(), directories.end());
        std::sort(files.begin(), files.end());
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
        currentDirectory = std::filesystem::current_path().string();
        directories.clear();
        files.clear();
    }
}

void Player::navigateToDirectory(const std::string& dirName) {
    try {
        if (dirName == "..") {
            std::filesystem::path parent = std::filesystem::path(currentDirectory).parent_path();
            currentDirectory = parent.string();
        } else {
            currentDirectory = (std::filesystem::path(currentDirectory) / dirName).string();
        }
        refreshFileList();
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error navigating directory: " << e.what() << std::endl;
    }
}

void Player::drawJudgement() {
    if (showJudgement && !lastJudgementText.empty()) {
        ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f - 100,
                                      displaySize.y * 0.5f - 50));
        ImGui::SetNextWindowSize(ImVec2(200, 100));

        ImGui::Begin("Judgement", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

        ImGui::PushStyleColor(ImGuiCol_Text, lastJudgementColor);
        ImGui::SetCursorPos(ImVec2(50, 30));
        ImGui::Text("%s", lastJudgementText.c_str());
        ImGui::PopStyleColor();

        ImGui::End();
    }
}

void Player::updateGameLogic() {
    checkNoteHits();

    if (showJudgement) {
        judgementDisplayTime -= ImGui::GetIO().DeltaTime;
        if (judgementDisplayTime <= 0.0) {
            showJudgement = false;
            lastJudgementText.clear();
        }
    }
}

void Player::processInput() {
    double currentTime = currentPosition;

    GameNote* bestTopNote = nullptr;
    GameNote* bestBottomNote = nullptr;
    double bestTopTimeDiff = judgementWindow;
    double bestBottomTimeDiff = judgementWindow;

    for (auto& note : gameNotes) {
        if (note.isActive && !note.hit) {
            if (note.type == Core::NoteType::TAP) {
                double time_diff = std::abs(currentTime - note.timestamp);

                if (time_diff <= judgementWindow) {
                    if (note.lane == Core::Lane::TOP && time_diff < bestTopTimeDiff) {
                        bestTopNote = &note;
                        bestTopTimeDiff = time_diff;
                    } else if (note.lane == Core::Lane::BOTTOM && time_diff < bestBottomTimeDiff) {
                        bestBottomNote = &note;
                        bestBottomTimeDiff = time_diff;
                    }
                }
            } else if (note.type == Core::NoteType::HOLD && !note.isHolding) {
                double time_diff = std::abs(currentTime - note.timestamp);

                if (time_diff <= judgementWindow) {
                    if (note.lane == Core::Lane::TOP && time_diff < bestTopTimeDiff) {
                        bestTopNote = &note;
                        bestTopTimeDiff = time_diff;
                    } else if (note.lane == Core::Lane::BOTTOM && time_diff < bestBottomTimeDiff) {
                        bestBottomNote = &note;
                        bestBottomTimeDiff = time_diff;
                    }
                }
            }
        }
    }

    if (fKeyPressed && bestTopNote) {
        processNoteHit(*bestTopNote, currentTime);
    }

    if (jKeyPressed && bestBottomNote) {
        processNoteHit(*bestBottomNote, currentTime);
    }
}

void Player::processNoteHit(GameNote& note, double currentTime) {
    if (note.type == Core::NoteType::TAP) {
        note.hit = true;
        note.hitTime = currentTime;
        Judgement judgement = calculateJudgement(currentTime, note.timestamp);
        note.judgement = judgement;
        updateStats(judgement);

        ImVec2 hitPosition = ImVec2(50.0f, displaySize.y * 0.5f);
        if (note.lane == Core::Lane::BOTTOM) {
            hitPosition.y += laneHeight * 0.25f;
        } else {
            hitPosition.y -= laneHeight * 0.25f;
        }
        createHitEffect(hitPosition, judgement);

        playHitSound();

        showJudgement = true;
        judgementDisplayTime = 0.5;
        switch (judgement) {
            case PERFECT:
                lastJudgementText = "PERFECT";
                lastJudgementColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case GREAT:
                lastJudgementText = "GREAT";
                lastJudgementColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                break;
            case GOOD:
                lastJudgementText = "GOOD";
                lastJudgementColor = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
                break;
            case MISS:
                lastJudgementText = "MISS";
                lastJudgementColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
        }
    } else if (note.type == Core::NoteType::HOLD) {
        note.isHolding = true;
        note.holdStartTime = currentTime;
        note.hitTime = currentTime;
        note.lastHoldTickTime = currentTime;
        note.holdTicks = 0;
        note.totalHoldTicks = static_cast<int>((note.endTimestamp - note.timestamp) / holdTickInterval) + 1;

        Judgement judgement = calculateJudgement(currentTime, note.timestamp);
        note.judgement = judgement;
        updateStats(judgement);

        ImVec2 hitPosition = ImVec2(50.0f, displaySize.y * 0.5f);
        if (note.lane == Core::Lane::BOTTOM) {
            hitPosition.y += laneHeight * 0.25f;
        } else {
            hitPosition.y -= laneHeight * 0.25f;
        }
        createHitEffect(hitPosition, judgement);

        playHitSound();

        showJudgement = true;
        judgementDisplayTime = 0.5;
        switch (judgement) {
            case PERFECT:
                lastJudgementText = "HOLD START";
                lastJudgementColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case GREAT:
                lastJudgementText = "HOLD START";
                lastJudgementColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                break;
            case GOOD:
                lastJudgementText = "HOLD START";
                lastJudgementColor = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
                break;
            case MISS:
                lastJudgementText = "MISS";
                lastJudgementColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
        }
    }
}

void Player::updateHoldNotes() {
    double currentTime = currentPosition;

    for (auto& note : gameNotes) {
        if (note.isActive && note.isHolding && !note.hit) {
            if (!isKeyHeldForLane(note.lane)) {
                breakHoldNote(note, "Key released");
                continue;
            }

            if (currentTime >= note.endTimestamp) {
                completeHoldNote(note);
                continue;
            }

            if (currentTime - note.lastHoldTickTime >= holdTickInterval) {
                processHoldTick(note);
            }
        }
    }
}

void Player::processHoldTick(GameNote& note) {
    double currentTime = currentPosition;
    note.lastHoldTickTime = currentTime;
    note.holdTicks++;

    stats.score += 5;

    if (note.totalHoldTicks > 0) {
        note.holdAccuracy = static_cast<double>(note.holdTicks) / note.totalHoldTicks;
    }
}

void Player::breakHoldNote(GameNote& note, const std::string& reason) {
    note.isHolding = false;
    note.hit = true;
    note.holdCompleted = false;
    note.judgement = MISS;
    updateStats(MISS);
    stats.combo = 0;

    ImVec2 missPosition = ImVec2(50.0f, displaySize.y * 0.5f);
    if (note.lane == Core::Lane::BOTTOM) {
        missPosition.y += laneHeight * 0.25f;
    } else {
        missPosition.y -= laneHeight * 0.25f;
    }
    createHitEffect(missPosition, MISS);

    showJudgement = true;
    judgementDisplayTime = 0.5;
    lastJudgementText = "HOLD BREAK";
    lastJudgementColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
}

void Player::completeHoldNote(GameNote& note) {
    note.isHolding = false;
    note.hit = true;
    note.holdCompleted = true;

    if (note.totalHoldTicks > 0) {
        note.holdAccuracy = static_cast<double>(note.holdTicks) / note.totalHoldTicks;
    }

    int holdBonus = static_cast<int>(note.holdAccuracy * 50);
    stats.score += holdBonus;

    ImVec2 hitPosition = ImVec2(50.0f, displaySize.y * 0.5f);
    if (note.lane == Core::Lane::BOTTOM) {
        hitPosition.y += laneHeight * 0.25f;
    } else {
        hitPosition.y -= laneHeight * 0.25f;
    }
    createHitEffect(hitPosition, note.judgement);

    showJudgement = true;
    judgementDisplayTime = 0.5;
    lastJudgementText = "HOLD COMPLETE";
    lastJudgementColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
}

void Player::checkNoteHits() {
    double currentTime = currentPosition;

    for (auto& note : gameNotes) {
        if (note.isActive && !note.hit) {
            if (note.type == Core::NoteType::TAP) {
                if (currentTime - note.timestamp > judgementWindow) {
                    note.hit = true;
                    note.judgement = MISS;
                    updateStats(MISS);
                    stats.combo = 0;

                    ImVec2 missPosition = ImVec2(50.0f, displaySize.y * 0.5f);
                    if (note.lane == Core::Lane::BOTTOM) {
                        missPosition.y += laneHeight * 0.25f;
                    } else {
                        missPosition.y -= laneHeight * 0.25f;
                    }
                    createHitEffect(missPosition, MISS);
                }
            } else if (note.type == Core::NoteType::HOLD) {
                if (!note.isHolding && currentTime - note.timestamp > judgementWindow) {
                    note.hit = true;
                    note.judgement = MISS;
                    note.holdCompleted = false;
                    updateStats(MISS);
                    stats.combo = 0;

                    ImVec2 missPosition = ImVec2(50.0f, displaySize.y * 0.5f);
                    if (note.lane == Core::Lane::BOTTOM) {
                        missPosition.y += laneHeight * 0.25f;
                    } else {
                        missPosition.y -= laneHeight * 0.25f;
                    }
                    createHitEffect(missPosition, MISS);
                }
            }
        }
    }
}

Judgement Player::calculateJudgement(double hitTime, double noteTime) {
    double time_diff = std::abs(hitTime - noteTime);

    if (time_diff <= perfectWindow) {
        return PERFECT;
    } else if (time_diff <= greatWindow) {
        return GREAT;
    } else if (time_diff <= goodWindow) {
        return GOOD;
    } else {
        return MISS;
    }
}

void Player::updateStats(Judgement judgement) {
    switch (judgement) {
        case PERFECT:
            stats.perfect++;
            stats.combo++;
            stats.score += 100;
            break;
        case GREAT:
            stats.great++;
            stats.combo++;
            stats.score += 50;
            break;
        case GOOD:
            stats.good++;
            stats.combo++;
            stats.score += 25;
            break;
        case MISS:
            stats.miss++;
            stats.combo = 0;
            break;
    }

    if (stats.combo > stats.maxCombo) {
        stats.maxCombo = stats.combo;
    }

    int total_notes = stats.perfect + stats.great + stats.good + stats.miss;
    if (total_notes > 0) {
        double weighted_score = (stats.perfect * 100.0) + (stats.great * 80.0) + (stats.good * 60.0);
        stats.accuracy = weighted_score / total_notes;
    }

    recentJudgements.push_back(judgement);
    if (recentJudgements.size() > 10) {
        recentJudgements.pop_front();
    }
}



void Player::drawResults() {
    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f - 200, displaySize.y * 0.5f - 150));
    ImGui::SetNextWindowSize(ImVec2(400, 300));

    ImGui::Begin("Game Results", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.15f, 0.95f));

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "FINAL RESULTS");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Chart: %s", chartTitle.c_str());
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Artist: %s", chartArtist.c_str());
    ImGui::TextColored(scoreColor, "Final Score: %d", stats.score);
    ImGui::TextColored(comboColor, "Max Combo: %d", stats.maxCombo);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "Accuracy: %.1f%%", stats.accuracy);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Perfect: %d", stats.perfect);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Great: %d", stats.great);
    ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "Good: %d", stats.good);
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Miss: %d", stats.miss);

    ImGui::Separator();

    if (ImGui::Button("PLAY AGAIN", ImVec2(120, 30))) {
        resetGame();
        startGame();
    }

    ImGui::SameLine();
    if (ImGui::Button("BACK TO MENU", ImVec2(120, 30))) {
        gameState = MENU;
    }

    ImGui::PopStyleColor();
    ImGui::End();
}

void Player::drawMainMenu() {
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 50, main_viewport->WorkPos.y + 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

    ImGui::Begin("Not A Rhythm Game - Player", nullptr);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.15f, 0.95f));

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "NOT A RHYTHM GAME");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Player Mode");
    ImGui::Separator();

    ImGui::Spacing();

    if (ImGui::Button("Start a Not Game", ImVec2(200, 40)) && isSongLoaded) {
        startGame();
    }

    if (!isSongLoaded) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "(No chart loaded)");
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (!recentCharts.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Recent Charts:");
        ImGui::Spacing();

        for (size_t i = 0; i < recentCharts.size() && i < 5; ++i) {
            std::string chartName = std::filesystem::path(recentCharts[i]).stem().string();
            std::string displayName = std::to_string(i) + " " + chartName;

            if (ImGui::Selectable(displayName.c_str())) {
                loadSong(recentCharts[i]);
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Click to load: %s", recentCharts[i].c_str());
            }
        }

        ImGui::Spacing();
    }

    if (isSongLoaded) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Current Chart:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Title: %s", chartTitle.c_str());
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Artist: %s", chartArtist.c_str());
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Notes: %zu", gameNotes.size());

        ImGui::Spacing();
        if (ImGui::Button("Chart Details", ImVec2(120, 25))) {
            showChartInfo = true;
        }
    }

    ImGui::PopStyleColor();
    ImGui::End();
}

void Player::drawChartBrowser() {
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

    ImGui::Begin("Chart Browser", nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Refresh")) {
                refreshFileList();
            }
            if (ImGui::MenuItem("Home")) {
                const char* homeDir = getenv("HOME");
                if (homeDir) {
                    std::string musicDir = std::string(homeDir) + "/Music";
                    if (std::filesystem::exists(musicDir)) {
                        currentDirectory = musicDir;
                    } else {
                        currentDirectory = homeDir;
                    }
                } else {
                    currentDirectory = ".";
                }
                selectedFileIndex = -1;
                refreshFileList();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Current Directory:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", currentDirectory.c_str());

    static char manualPath[512] = "";
    static bool pathUpdated = false;

    if (ImGui::IsWindowAppearing() || pathUpdated) {
        strncpy(manualPath, currentDirectory.c_str(), sizeof(manualPath) - 1);
        pathUpdated = false;
    }

    ImGui::Text("Path:");
    ImGui::SameLine();
    if (ImGui::InputText("##manualPath", manualPath, sizeof(manualPath), ImGuiInputTextFlags_EnterReturnsTrue)) {
        try {
            std::filesystem::path testPath(manualPath);
            if (std::filesystem::is_directory(testPath)) {
                currentDirectory = testPath.string();
                selectedFileIndex = -1;
                refreshFileList();
                pathUpdated = true;
            }
        } catch (const std::exception& e) {
            // Invalid path, ignore
        }
    }

    ImGui::Separator();

    ImGui::BeginChild("FileList", ImVec2(0, 0), ImGuiChildFlags_Borders);

    if (!directories.empty()) {
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "D Directories:");
        for (size_t i = 0; i < directories.size(); i++) {
            std::string dirName = directories[i];
            if (dirName == "..") {
                dirName = "D .. (Parent Directory)";
            } else {
                dirName = "D " + dirName;
            }

            if (ImGui::Selectable(dirName.c_str(), false)) {
                navigateToDirectory(directories[i]);
                pathUpdated = true;
            }
        }
        ImGui::Separator();
    }

    if (!files.empty()) {
        ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.8f, 1.0f), "C Chart Files:");

        for (size_t i = 0; i < files.size(); i++) {
            std::string fileName = "C " + files[i];
            bool isSelected = (selectedFileIndex == static_cast<int>(i));

            if (ImGui::Selectable(fileName.c_str(), isSelected)) {
                selectedFileIndex = static_cast<int>(i);
            }

            if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {
                std::string fullPath = currentDirectory + "/" + files[i];
                if (loadSong(fullPath)) {
                    addToRecentCharts(fullPath);
                }
            }
        }
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No chart files found in this directory");
    }

    ImGui::EndChild();

    if (selectedFileIndex >= 0 && selectedFileIndex < static_cast<int>(files.size())) {
        ImGui::Separator();
        ImGui::Text("Selected: %s", files[selectedFileIndex].c_str());

        if (ImGui::Button("Load Selected File", ImVec2(150, 25))) {
            std::string fullPath = currentDirectory + "/" + files[selectedFileIndex];
            if (loadSong(fullPath)) {
                addToRecentCharts(fullPath);
            }
        }
    }

    ImGui::Separator();
    ImGui::Text("Directories: %zu | Charts: %zu | Double-click to load", directories.size(), files.size());

    ImGui::End();
}

void Player::resetGame() {
    stats = {0, 0, 0, 0, 0, 0, 0.0, 0};
    currentPosition = 0.0;
    recentJudgements.clear();
    hitEffects.clear();

    for (auto& note : gameNotes) {
        note.hit = false;
        note.isActive = true;
        note.isHolding = false;
        note.holdCompleted = false;
        note.holdAccuracy = 0.0;
        note.holdTicks = 0;
        note.totalHoldTicks = 0;
        note.lastHoldTickTime = 0.0;
    }
}

void Player::startGame() {
    if (!soundManager || !isSongLoaded) return;

    gameState = PLAYING;
    isPlaying = true;
    currentPosition = 0.0;
    soundManager->playSound(currentSongName);
    resetGame();
}

void Player::pauseGame() {
    if (!soundManager || gameState != PLAYING) return;

    gameState = PAUSED;
    isPlaying = false;
    soundManager->pauseSound(currentSongName);
}

void Player::resumeGame() {
    if (!soundManager || gameState != PAUSED) return;

    gameState = PLAYING;
    isPlaying = true;
    soundManager->resumeSound(currentSongName);
}

bool Player::loadChartFile(const std::string& filepath) {
    cleanupTempFiles();

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open chart file: " << filepath << std::endl;
        return false;
    }

    ChartHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(ChartHeader));

    if (std::string(header.magic) != "NOTARHYTHM") {
        std::cerr << "Invalid chart file format - wrong magic number: " << header.magic << std::endl;
        return false;
    }

    if (header.version < 1 || header.version > 2) {
        std::cerr << "Unsupported chart file version: " << header.version << std::endl;
        return false;
    }

    chartTitle = header.title;
    chartArtist = header.artist;
    bpm = header.bpm;
    songDuration = header.duration;

    std::vector<char> audioData(header.audioSize);
    file.read(audioData.data(), header.audioSize);
    if (!file.good()) {
        std::cerr << "Failed to read audio data from chart" << std::endl;
        return false;
    }

    std::filesystem::path chartPath(filepath);
    std::string tempAudioPath = chartPath.parent_path().string() + "/temp_audio_" + chartPath.stem().string() + ".wav";

    if (!writeAudioFile(tempAudioPath, audioData)) {
        std::cerr << "Failed to write temporary audio file: " << tempAudioPath << std::endl;
        return false;
    }

    std::string songName = chartPath.stem().string();
    if (!soundManager->loadSound(songName, tempAudioPath)) {
        std::cerr << "Failed to load audio from chart" << std::endl;
        std::filesystem::remove(tempAudioPath);
        return false;
    }

    currentSongPath = tempAudioPath;
    currentSongName = songName;
    isSongLoaded = true;
    currentPosition = 0.0;
    isPlaying = false;

    gameNotes.clear();
    for (uint32_t i = 0; i < header.notesCount; ++i) {
        GameNote note;
        file.read(reinterpret_cast<char*>(&note.id), sizeof(int));
        file.read(reinterpret_cast<char*>(&note.lane), sizeof(Core::Lane));

        if (header.version >= 2) {
            file.read(reinterpret_cast<char*>(&note.type), sizeof(Core::NoteType));
            file.read(reinterpret_cast<char*>(&note.timestamp), sizeof(double));
            file.read(reinterpret_cast<char*>(&note.endTimestamp), sizeof(double));
        } else {
            // Version 1 compatibility - all notes are TAP notes
            note.type = Core::NoteType::TAP;
            file.read(reinterpret_cast<char*>(&note.timestamp), sizeof(double));
            note.endTimestamp = note.timestamp;
        }

        note.hit = false;
        note.judgement = MISS;
        note.hitTime = 0.0;
        note.isActive = true;
        note.isHolding = false;
        note.holdStartTime = 0.0;
        note.holdCompleted = false;
        note.holdAccuracy = 0.0;
        note.holdTicks = 0;
        note.totalHoldTicks = 0;
        note.lastHoldTickTime = 0.0;
        gameNotes.push_back(note);
    }

    calculateGridSpacing();
    std::cout << "Successfully loaded chart: " << chartTitle << " by " << chartArtist << std::endl;
    std::cout << "Notes loaded: " << gameNotes.size() << std::endl;
    return true;
}

std::vector<char> Player::readAudioFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(file_size);
    file.read(buffer.data(), file_size);

    return buffer;
}

bool Player::writeAudioFile(const std::string& filepath, const std::vector<char>& audioData) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.write(audioData.data(), audioData.size());
    return true;
}



void Player::drawGameplayWindow() {
    ImGui::Begin("Not A Game", nullptr, ImGuiWindowFlags_NoCollapse);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    draw_list->AddRectFilled(window_pos,
                            ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y),
                            IM_COL32(20, 20, 30, 255));

    drawGameplayLanes();
    drawApproachingNotes();
    drawHitEffects();
    drawComboDisplay();
    drawScoreDisplay();
    drawProgressBar();

    ImGui::End();
}

void Player::drawStatsWindow() {
    ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.15f, 0.9f));

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "SCORE");
    ImGui::SameLine();
    ImGui::TextColored(scoreColor, "%d", stats.score);

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "COMBO");
    ImGui::SameLine();
    ImGui::TextColored(comboColor, "%d", stats.combo);

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "ACCURACY");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "%.1f%%", stats.accuracy);

    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "PERFECT: %d", stats.perfect);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "GREAT: %d", stats.great);
    ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "GOOD: %d", stats.good);
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "MISS: %d", stats.miss);

    ImGui::PopStyleColor();
    ImGui::End();
}

void Player::drawControlsWindow() {
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.15f, 0.9f));

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "CONTROLS");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "F Key - Top Lane");
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.6f, 1.0f), "J Key - Bottom Lane");
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Space - Pause/Resume");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "HOLD NOTES:");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "• Press and hold key when note arrives");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "• Keep holding until note ends");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "• Release early = HOLD BREAK");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "• Complete hold = bonus points");

    ImGui::Spacing();

    if (gameState == PLAYING) {
        if (ImGui::Button("PAUSE", ImVec2(80, 30))) {
            pauseGame();
        }
    } else if (gameState == PAUSED) {
        if (ImGui::Button("RESUME", ImVec2(80, 30))) {
            resumeGame();
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("MENU", ImVec2(80, 30))) {
        pauseGame();
        gameState = MENU;
        isPlaying = false;
    }

    ImGui::PopStyleColor();
    ImGui::End();
}

void Player::drawSongInfoWindow() {
    ImGui::Begin("Song Info", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.15f, 0.9f));

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "CHART INFO");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Title: %s", chartTitle.c_str());
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Artist: %s", chartArtist.c_str());
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "BPM: %.1f", bpm);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Duration: %.1fs", songDuration);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Notes: %zu", gameNotes.size());

    ImGui::PopStyleColor();
    ImGui::End();
}

void Player::updateVisualEffects() {
    float deltaTime = ImGui::GetIO().DeltaTime;
    effectTimer += deltaTime;

    if (stats.combo > 0) {
        comboScale = 1.0f + std::sin(effectTimer * 5.0f) * 0.1f;
    } else {
        comboScale = 1.0f;
    }

    scoreScale = 1.0f + std::sin(effectTimer * 3.0f) * 0.05f;

    if (showJudgement) {
        judgementScale = 1.0f + std::sin(effectTimer * 10.0f) * 0.2f;
    } else {
        judgementScale = 1.0f;
    }
}

void Player::drawGameplayLanes() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    float hit_zone_x = window_pos.x + 50.0f;
    float lane_y = window_pos.y + window_size.y * 0.5f;

    ImVec2 top_lane_start(hit_zone_x, lane_y - laneHeight * 0.5f);
    ImVec2 top_lane_end(window_pos.x + window_size.x - 50.0f, lane_y);
    draw_list->AddRectFilled(top_lane_start, top_lane_end,
                            IM_COL32(laneTopColor.x * 255, laneTopColor.y * 255,
                                   laneTopColor.z * 255, laneTopColor.w * 255));

    ImVec2 bottom_lane_start(hit_zone_x, lane_y);
    ImVec2 bottom_lane_end(window_pos.x + window_size.x - 50.0f, lane_y + laneHeight * 0.5f);
    draw_list->AddRectFilled(bottom_lane_start, bottom_lane_end,
                            IM_COL32(laneBottomColor.x * 255, laneBottomColor.y * 255,
                                   laneBottomColor.z * 255, laneBottomColor.w * 255));

    draw_list->AddRect(top_lane_start, top_lane_end, IM_COL32(255, 255, 255, 100), 0.0f, 0, 2.0f);
    draw_list->AddRect(bottom_lane_start, bottom_lane_end, IM_COL32(255, 255, 255, 100), 0.0f, 0, 2.0f);

    ImVec2 hit_zone_start(hit_zone_x - 5.0f, lane_y - laneHeight * 0.5f);
    ImVec2 hit_zone_end(hit_zone_x + 5.0f, lane_y + laneHeight * 0.5f);
    draw_list->AddRectFilled(hit_zone_start, hit_zone_end,
                            IM_COL32(hitZoneColor.x * 255, hitZoneColor.y * 255,
                                   hitZoneColor.z * 255, hitZoneColor.w * 255));
}

void Player::drawApproachingNotes() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    float lane_y = window_pos.y + window_size.y * 0.5f;
    float lane_width = window_size.x - 100.0f;

    for (const auto& note : gameNotes) {
        if (note.isActive) {
            if (note.type == Core::NoteType::TAP) {
                if (note.hit) continue;

                double time_until_hit = note.timestamp - currentPosition;

                if (time_until_hit >= -judgementWindow && time_until_hit <= approachTime) {
                    float progress = 1.0f - (time_until_hit / approachTime);
                    float note_x = window_pos.x + window_size.x - 50.0f - progress * lane_width;

                    if (note_x >= window_pos.x && note_x <= window_pos.x + window_size.x) {
                        float note_y = lane_y;
                        if (note.lane == Core::Lane::BOTTOM) {
                            note_y += laneHeight * 0.25f;
                        } else {
                            note_y -= laneHeight * 0.25f;
                        }

                        ImVec4 note_color = (note.lane == Core::Lane::TOP) ? noteTopColor : noteBottomColor;
                        ImVec2 note_center(note_x, note_y);

                        float approach_intensity = 1.0f - progress;
                        float glow_alpha = 40 + (int)(approach_intensity * 60);
                        float main_alpha = 200 + (int)(approach_intensity * 55);

                        draw_list->AddCircleFilled(note_center, noteRadius * 1.8f,
                                                 IM_COL32(note_color.x * 255, note_color.y * 255,
                                                        note_color.z * 255, (int)glow_alpha));

                        draw_list->AddCircleFilled(note_center, noteRadius * 1.2f,
                                                 IM_COL32(note_color.x * 255, note_color.y * 255,
                                                        note_color.z * 255, (int)main_alpha));

                        draw_list->AddCircleFilled(note_center, noteRadius * 0.8f,
                                                 IM_COL32(note_color.x * 255, note_color.y * 255,
                                                        note_color.z * 255, 255));

                        draw_list->AddCircle(note_center, noteRadius * 1.2f,
                                           IM_COL32(255, 255, 255, 200), 24, 2.0f);
                    }
                }
            } else if (note.type == Core::NoteType::HOLD) {
                double time_until_hit = note.timestamp - currentPosition;
                double time_until_end = note.endTimestamp - currentPosition;

                bool should_render = (time_until_end >= -judgementWindow);

                if (should_render) {
                    float progress_start = 1.0f - (time_until_hit / approachTime);
                    float progress_end = 1.0f - (time_until_end / approachTime);

                    float start_x = window_pos.x + window_size.x - 50.0f - progress_start * lane_width;
                    float end_x = window_pos.x + window_size.x - 50.0f - progress_end * lane_width;

                    float display_start_x = std::max(start_x, window_pos.x);
                    float display_end_x = std::min(end_x, window_pos.x + window_size.x);

                    if (display_end_x > display_start_x) {
                        float note_y = lane_y;
                        if (note.lane == Core::Lane::BOTTOM) {
                            note_y += laneHeight * 0.25f;
                        } else {
                            note_y -= laneHeight * 0.25f;
                        }

                        ImVec4 note_color = (note.lane == Core::Lane::TOP) ? noteTopColor : noteBottomColor;

                        if (note.hit && !note.holdCompleted) {
                            note_color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
                        }

                        float approach_intensity = 1.0f - std::min(progress_start, progress_end);
                        float base_alpha = note.isHolding ? 0.9f : 0.7f;
                        float alpha = base_alpha + (approach_intensity * 0.2f);

                        float hold_height = noteRadius * 1.2f;

                        draw_list->AddRectFilled(
                            ImVec2(display_start_x, note_y - hold_height * 0.5f),
                            ImVec2(display_end_x, note_y + hold_height * 0.5f),
                            IM_COL32(note_color.x * 255, note_color.y * 255, note_color.z * 255, (int)(120 * alpha))
                        );

                        draw_list->AddRect(
                            ImVec2(display_start_x, note_y - hold_height * 0.5f),
                            ImVec2(display_end_x, note_y + hold_height * 0.5f),
                            IM_COL32(note_color.x * 255, note_color.y * 255, note_color.z * 255, (int)(200 * alpha)),
                            0.0f, 0, 2.0f
                        );

                        if (start_x >= window_pos.x - noteRadius * 2.0f &&
                            start_x <= window_pos.x + window_size.x + noteRadius * 2.0f) {

                            ImVec2 start_center(start_x, note_y);

                            draw_list->AddCircleFilled(start_center, noteRadius * 1.8f,
                                                     IM_COL32(note_color.x * 255, note_color.y * 255,
                                                            note_color.z * 255, 30));

                            draw_list->AddCircleFilled(start_center, noteRadius * 1.2f,
                                                     IM_COL32(note_color.x * 255, note_color.y * 255,
                                                            note_color.z * 255, (int)(200 * alpha)));

                            draw_list->AddCircleFilled(start_center, noteRadius * 0.8f,
                                                     IM_COL32(note_color.x * 255, note_color.y * 255,
                                                            note_color.z * 255, (int)(255 * alpha)));

                            draw_list->AddCircle(start_center, noteRadius * 1.2f,
                                               IM_COL32(255, 255, 255, (int)(200 * alpha)), 24, 2.0f);
                        }

                        if (end_x >= window_pos.x - noteRadius * 2.0f &&
                            end_x <= window_pos.x + window_size.x + noteRadius * 2.0f) {

                            ImVec2 end_center(end_x, note_y);

                            draw_list->AddCircleFilled(end_center, noteRadius * 1.5f,
                                                     IM_COL32(note_color.x * 255, note_color.y * 255,
                                                            note_color.z * 255, 30));

                            draw_list->AddCircleFilled(end_center, noteRadius * 1.0f,
                                                     IM_COL32(note_color.x * 255, note_color.y * 255,
                                                            note_color.z * 255, (int)(180 * alpha)));

                            draw_list->AddCircleFilled(end_center, noteRadius * 0.6f,
                                                     IM_COL32(note_color.x * 255, note_color.y * 255,
                                                            note_color.z * 255, (int)(220 * alpha)));

                            draw_list->AddCircle(end_center, noteRadius * 1.0f,
                                               IM_COL32(255, 255, 255, (int)(180 * alpha)), 24, 2.0f);
                        }

                        if (note.isHolding) {
                            float pulse = 0.5f + 0.3f * std::sin(currentPosition * 8.0f);
                            float pulse_radius = noteRadius * 0.3f * pulse;

                            double hold_progress = 0.0;
                            if (note.endTimestamp > note.timestamp) {
                                hold_progress = (currentPosition - note.timestamp) / (note.endTimestamp - note.timestamp);
                                hold_progress = std::clamp(hold_progress, 0.0, 1.0);
                            }

                            float pulse_x = display_start_x + (display_end_x - display_start_x) * static_cast<float>(hold_progress);

                            if (pulse_x >= window_pos.x && pulse_x <= window_pos.x + window_size.x) {
                                draw_list->AddCircleFilled(
                                    ImVec2(pulse_x, note_y),
                                    pulse_radius,
                                    IM_COL32(255, 255, 255, (int)(100 * pulse))
                                );
                            }
                        }
                    }
                }
            }
        }
    }
}

void Player::drawHitEffects() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();

    for (const auto& effect : hitEffects) {
        if (!effect.active) continue;

        ImVec4 color;
        switch (effect.judgement) {
            case PERFECT:
                color = ImVec4(1.0f, 1.0f, 0.0f, effect.alpha);
                break;
            case GREAT:
                color = ImVec4(0.0f, 1.0f, 0.0f, effect.alpha);
                break;
            case GOOD:
                color = ImVec4(0.0f, 0.0f, 1.0f, effect.alpha);
                break;
            case MISS:
                color = ImVec4(1.0f, 0.0f, 0.0f, effect.alpha);
                break;
        }

        ImVec2 center = ImVec2(window_pos.x + effect.position.x, window_pos.y + effect.position.y);
        float radius = noteRadius * effect.scale;

        draw_list->AddCircleFilled(center, radius * 1.2f,
                                  IM_COL32(color.x * 255, color.y * 255, color.z * 255,
                                          (int)(color.w * 50)));

        draw_list->AddCircleFilled(center, radius,
                                  IM_COL32(color.x * 255, color.y * 255, color.z * 255,
                                          (int)(color.w * 180)));

        draw_list->AddCircleFilled(center, radius * 0.7f,
                                  IM_COL32(255, 255, 255, (int)(color.w * 80)));

        std::string judgementText;
        switch (effect.judgement) {
            case PERFECT: judgementText = "PERFECT"; break;
            case GREAT: judgementText = "GREAT"; break;
            case GOOD: judgementText = "GOOD"; break;
            case MISS: judgementText = "MISS"; break;
        }

        ImVec2 text_size = ImGui::CalcTextSize(judgementText.c_str());
        ImVec2 text_pos(center.x - text_size.x * 0.5f * effect.scale,
                       center.y - text_size.y * 0.5f * effect.scale);

        draw_list->AddText(nullptr, text_size.x * effect.scale,
                          ImVec2(text_pos.x + 1, text_pos.y + 1),
                          IM_COL32(0, 0, 0, (int)(color.w * 100)),
                          judgementText.c_str());

        draw_list->AddText(nullptr, text_size.x * effect.scale, text_pos,
                          IM_COL32(color.x * 255, color.y * 255, color.z * 255,
                                  (int)(color.w * 180)),
                          judgementText.c_str());

        for (int i = 0; i < 4; ++i) {
            float angle = (float)i * 3.14159f * 2.0f / 4.0f;
            float distance = radius * 0.8f * effect.scale;
            ImVec2 particle_pos(center.x + cos(angle) * distance,
                               center.y + sin(angle) * distance);

            draw_list->AddCircleFilled(particle_pos, 2.0f * effect.scale,
                                      IM_COL32(color.x * 255, color.y * 255, color.z * 255,
                                              (int)(color.w * 80)));
        }
    }
}

void Player::drawComboDisplay() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    if (stats.combo > 0) {
        std::string combo_text = std::to_string(stats.combo) + " COMBO";
        ImVec2 text_size = ImGui::CalcTextSize(combo_text.c_str());
        ImVec2 text_pos(window_pos.x + window_size.x * 0.5f - text_size.x * 0.5f * comboScale,
                       window_pos.y + window_size.y * 0.3f - text_size.y * 0.5f * comboScale);

        draw_list->AddText(nullptr, text_size.x * comboScale,
                          ImVec2(text_pos.x + 2, text_pos.y + 2),
                          IM_COL32(0, 0, 0, 100), combo_text.c_str());
        draw_list->AddText(nullptr, text_size.x * comboScale, text_pos,
                          IM_COL32(comboColor.x * 255, comboColor.y * 255,
                                 comboColor.z * 255, comboColor.w * 255), combo_text.c_str());
    }
}

void Player::drawScoreDisplay() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    std::string score_text = std::to_string(stats.score);
    ImVec2 text_size = ImGui::CalcTextSize(score_text.c_str());
    ImVec2 text_pos(window_pos.x + window_size.x * 0.5f - text_size.x * 0.5f * scoreScale,
                   window_pos.y + window_size.y * 0.1f - text_size.y * 0.5f * scoreScale);

    draw_list->AddText(nullptr, text_size.x * scoreScale,
                      ImVec2(text_pos.x + 2, text_pos.y + 2),
                      IM_COL32(0, 0, 0, 100), score_text.c_str());
    draw_list->AddText(nullptr, text_size.x * scoreScale, text_pos,
                      IM_COL32(scoreColor.x * 255, scoreColor.y * 255,
                             scoreColor.z * 255, scoreColor.w * 255), score_text.c_str());
}

void Player::drawProgressBar() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    float progress = (float)(currentPosition / songDuration);
    float bar_width = window_size.x * 0.8f;
    float bar_height = 10.0f;
    float bar_x = window_pos.x + (window_size.x - bar_width) * 0.5f;
    float bar_y = window_pos.y + window_size.y - 30.0f;

    draw_list->AddRectFilled(ImVec2(bar_x, bar_y),
                            ImVec2(bar_x + bar_width, bar_y + bar_height),
                            IM_COL32(50, 50, 50, 255));

    draw_list->AddRectFilled(ImVec2(bar_x, bar_y),
                            ImVec2(bar_x + bar_width * progress, bar_y + bar_height),
                            IM_COL32(0, 200, 100, 255));

    draw_list->AddRect(ImVec2(bar_x, bar_y),
                      ImVec2(bar_x + bar_width, bar_y + bar_height),
                      IM_COL32(255, 255, 255, 100), 0.0f, 0, 2.0f);
}

void Player::drawChartInfo() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

    ImGui::Begin("Chart Information", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.15f, 0.95f));

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "CHART DETAILS");
    ImGui::Separator();

    if (isSongLoaded) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Title: %s", chartTitle.c_str());
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Artist: %s", chartArtist.c_str());
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "BPM: %.1f", bpm);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Duration: %.1fs", songDuration);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Notes: %zu", gameNotes.size());

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Button("Start a Not Game", ImVec2(120, 30))) {
            startGame();
        }

        ImGui::SameLine();
        if (ImGui::Button("Close Info", ImVec2(120, 30))) {
            showChartInfo = false;
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No chart loaded");

        ImGui::Spacing();
        if (ImGui::Button("Close Info", ImVec2(120, 30))) {
            showChartInfo = false;
        }
    }

    ImGui::PopStyleColor();
    ImGui::End();
}

void Player::loadRecentCharts() {
    recentCharts.clear();

    std::string recentFile = RECENT_CHART_FILE;
    std::ifstream file(recentFile);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line) && recentCharts.size() < 10) {
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (!line.empty() && std::filesystem::exists(line)) {
                recentCharts.push_back(line);
            }
        }
        file.close();
    }
}

void Player::saveRecentCharts() {
    std::string recentFile = RECENT_CHART_FILE;
    std::ofstream file(recentFile);
    if (file.is_open()) {
        for (const auto& chart : recentCharts) {
            file << chart << "\n";
        }
        file.close();
    }
}

void Player::addToRecentCharts(const std::string& chartPath) {
    recentCharts.erase(std::remove(recentCharts.begin(), recentCharts.end(), chartPath), recentCharts.end());

    recentCharts.insert(recentCharts.begin(), chartPath);

    if (recentCharts.size() > 10) {
        recentCharts.resize(10);
    }

    saveRecentCharts();
}

void Player::createHitEffect(const ImVec2& position, Judgement judgement) {
    HitEffect effect;
    effect.position = position;
    effect.scale = 0.3f;
    effect.alpha = 0.7f;
    effect.startTime = currentPosition;
    effect.duration = 0.5f;
    effect.judgement = judgement;
    effect.active = true;

    hitEffects.push_back(effect);
}

void Player::updateHitEffects() {
    double currentTime = currentPosition;

    for (auto it = hitEffects.begin(); it != hitEffects.end();) {
        double elapsed = currentTime - it->startTime;
        float progress = (float)(elapsed / it->duration);

        if (progress >= 1.0f) {
            it = hitEffects.erase(it);
        } else {
            it->scale = 0.3f + progress * 1.0f;
            it->alpha = 0.7f - progress * 0.7f;
            ++it;
        }
    }
}

void Player::playHitSound() {
    if (soundManager && std::filesystem::exists(hitSoundPath)) {
        static bool hitSoundLoaded = false;
        if (!hitSoundLoaded) {
            if (soundManager->loadSound("hit_sound", hitSoundPath)) {
                hitSoundLoaded = true;
                soundManager->setVolume("hit_sound", 1.0f);
            }
        }

        if (hitSoundLoaded) {
            soundManager->playSound("hit_sound");
        }
    }
}

Player::~Player() {
    cleanupTempFiles();
}

void Player::cleanupTempFiles() {
    if (!currentSongPath.empty() && currentSongPath.find("temp_audio_") != std::string::npos) {
        try {
            if (std::filesystem::exists(currentSongPath)) {
                std::filesystem::remove(currentSongPath);
                std::cout << "Cleaned up temporary audio file: " << currentSongPath << std::endl;
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error cleaning up temporary file: " << e.what() << std::endl;
        }
    }
}

} // Windows
} // App
