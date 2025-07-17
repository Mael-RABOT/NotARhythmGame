#include "Timeline.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdlib>

namespace App {
namespace Windows {

Timeline::Timeline()
    : soundManager(std::make_unique<Core::SoundManager>()),
      currentSongPath(""),
      currentSongName(""),
      isSongLoaded(false),
      isPlaying(false),
      currentPosition(0.0),
      songDuration(0.0),
      bpm(120.0f),
      showGrid(true),
      enableAutoscroll(true),
      gridSpacing(0.5f),
      timelineWidth(800.0f),
      timelineHeight(200.0f),
      zoomLevel(1.0f),
      scrollOffset(0.0f),
      targetScrollOffset(0.0f),
      minZoomLevel(1.0f),
      maxZoomLevel(20.0f),
      showFileDialog(false),
      currentDirectory("."),
      selectedFileIndex(-1) {

    if (!soundManager->initialize(-1, 44100, 0)) {
        std::cerr << "Failed to initialize sound manager" << std::endl;
    }

    calculateGridSpacing();
    refreshFileList();
}

void Timeline::calculateGridSpacing() {
    float beatDuration = 60.0f / bpm;
    gridSpacing = beatDuration;
}

void Timeline::updateAutoscroll() {
    if (!isSongLoaded || songDuration <= 0.0 || !isPlaying || !enableAutoscroll) return;

    // Calculate visible time range
    float visible_duration = songDuration / zoomLevel;

    // Calculate the visible time window
    float visible_start = scrollOffset;
    float visible_end = visible_start + visible_duration;

    // Check if playhead is outside the visible area
    // Adjust margin based on zoom level - smaller margin when zoomed in for more precise control
    float margin = visible_duration * (zoomLevel > 5.0f ? 0.1f : 0.15f);

    bool should_scroll = false;

    if (currentPosition < visible_start + margin) {
        // Playhead is too far left, scroll left
        targetScrollOffset = std::max(0.0f, static_cast<float>(currentPosition - margin));
        should_scroll = true;
    } else if (currentPosition > visible_end - margin) {
        // Playhead is too far right, scroll right
        float max_scroll = std::max(0.0f, static_cast<float>(songDuration - visible_duration));
        targetScrollOffset = std::min(max_scroll, static_cast<float>(currentPosition - visible_duration + margin));
        should_scroll = true;
    }

    // Smooth scrolling interpolation
    if (should_scroll) {
        float scroll_speed = 5.0f; // Increased for more responsive scrolling
        float delta_time = ImGui::GetIO().DeltaTime;
        float diff = targetScrollOffset - scrollOffset;

        if (std::abs(diff) > 0.05f) { // Reduced threshold for smoother movement
            scrollOffset += diff * scroll_speed * delta_time;
        } else {
            scrollOffset = targetScrollOffset; // Snap to target when close
        }
    }
}

void Timeline::loadSong(const std::string& filepath) {
    // Extract filename from path
    size_t lastSlash = filepath.find_last_of("/\\");
    currentSongName = (lastSlash != std::string::npos) ? filepath.substr(lastSlash + 1) : filepath;

    // Load the song
    if (soundManager->loadSound("timeline_song", filepath)) {
        currentSongPath = filepath;
        isSongLoaded = true;
        currentPosition = 0.0;
        isPlaying = false;

        // Get actual song duration from BASS
        songDuration = soundManager->getDuration("timeline_song");

        std::cout << "Loaded song: " << currentSongName << " (Duration: " << songDuration << "s)" << std::endl;
    } else {
        std::cerr << "Failed to load song: " << filepath << std::endl;
    }
}

void Timeline::updatePlayback() {
    if (!isSongLoaded) return;

    // Get current position from BASS
    currentPosition = soundManager->getPosition("timeline_song");

    // Check if song has ended
    if (isPlaying && currentPosition >= songDuration) {
        currentPosition = 0.0;
        isPlaying = false;
        soundManager->stopSound("timeline_song");
    }
}

void Timeline::handleKeyboardInput() {
    if (!isSongLoaded) return;

    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
        if (isPlaying) {
            soundManager->pauseSound("timeline_song");
            isPlaying = false;
        } else {
            soundManager->resumeSound("timeline_song");
            isPlaying = true;
        }
    }

    float seekAmount = 5.0f; // 5 seconds per key press

    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        double newPosition = std::max(0.0, currentPosition - seekAmount);
        if (soundManager->seekTo("timeline_song", newPosition)) {
            currentPosition = newPosition;
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        double newPosition = std::min(songDuration, currentPosition + seekAmount);
        if (soundManager->seekTo("timeline_song", newPosition)) {
            currentPosition = newPosition;
        }
    }
}

void Timeline::drawTimelineRuler() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 content_pos = ImGui::GetCursorScreenPos();

    float ruler_height = 30.0f;
    float timeline_y = content_pos.y;

    // Draw ruler background
    draw_list->AddRectFilled(
        ImVec2(content_pos.x, timeline_y),
        ImVec2(content_pos.x + timelineWidth, timeline_y + ruler_height),
        IM_COL32(40, 40, 40, 255)
    );

    // Draw time markers
    float visible_duration = songDuration / zoomLevel;
    float pixels_per_second = timelineWidth / visible_duration;
    float visible_start = scrollOffset;
    float visible_end = visible_start + visible_duration;

    // Adjust marker spacing based on zoom level
    float marker_interval = (zoomLevel > 2.0f) ? 0.5f : (zoomLevel > 1.0f) ? 1.0f : (zoomLevel > 0.5f) ? 2.0f : 5.0f;

    // Find the first marker that should be visible
    float first_marker = std::floor(visible_start / marker_interval) * marker_interval;

    for (float time = first_marker; time <= visible_end + marker_interval; time += marker_interval) {
        float x = content_pos.x + (time - visible_start) * pixels_per_second;

        if (x >= content_pos.x - 50 && x <= content_pos.x + timelineWidth + 50) { // Extra margin for smooth scrolling
            // Draw major tick
            draw_list->AddLine(
                ImVec2(x, timeline_y),
                ImVec2(x, timeline_y + ruler_height),
                IM_COL32(100, 100, 100, 255),
                1.0f
            );

            // Draw time label
            int minutes = (int)time / 60;
            int seconds = (int)time % 60;
            char time_label[16];
            snprintf(time_label, sizeof(time_label), "%d:%02d", minutes, seconds);

            ImGui::SetCursorScreenPos(ImVec2(x + 2, timeline_y + 5));
            ImGui::Text("%s", time_label);
        }
    }
}

void Timeline::drawTimelineGrid() {
    if (!showGrid) return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 content_pos = ImGui::GetCursorScreenPos();

    float timeline_y = content_pos.y + 30.0f; // Below ruler
    float visible_duration = songDuration / zoomLevel;
    float pixels_per_second = timelineWidth / visible_duration;
    float visible_start = scrollOffset;
    float visible_end = visible_start + visible_duration;

    // Find the first grid line that should be visible
    float first_grid_line = std::floor(visible_start / gridSpacing) * gridSpacing;

    // Draw grid lines based on BPM
    for (float time = first_grid_line; time <= visible_end + gridSpacing; time += gridSpacing) {
        float x = content_pos.x + (time - visible_start) * pixels_per_second;

        if (x >= content_pos.x - 50 && x <= content_pos.x + timelineWidth + 50) { // Extra margin for smooth scrolling
            draw_list->AddLine(
                ImVec2(x, timeline_y),
                ImVec2(x, timeline_y + timelineHeight),
                IM_COL32(60, 60, 60, 255),
                1.0f
            );
        }
    }
}

void Timeline::drawPlaybackCursor() {
    if (!isSongLoaded) return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 content_pos = ImGui::GetCursorScreenPos();

    float timeline_y = content_pos.y + 30.0f; // Below ruler
    float visible_duration = songDuration / zoomLevel;
    float pixels_per_second = timelineWidth / visible_duration;
    float cursor_x = content_pos.x + (currentPosition - scrollOffset) * pixels_per_second;

    if (cursor_x >= content_pos.x && cursor_x <= content_pos.x + timelineWidth) {
        // Draw playback cursor
        draw_list->AddLine(
            ImVec2(cursor_x, timeline_y),
            ImVec2(cursor_x, timeline_y + timelineHeight),
            IM_COL32(255, 100, 100, 255),
            2.0f
        );

        // Draw cursor head
        draw_list->AddTriangleFilled(
            ImVec2(cursor_x - 5, timeline_y),
            ImVec2(cursor_x + 5, timeline_y),
            ImVec2(cursor_x, timeline_y - 10),
            IM_COL32(255, 100, 100, 255)
        );
    }
}

void Timeline::update() {
    updatePlayback();
    handleKeyboardInput();
    updateAutoscroll();
}

void Timeline::render() {
    // Create a dockspace for better layout
    ImGui::DockSpaceOverViewport();

    // Main Timeline Window
    ImGui::Begin("Timeline", nullptr, ImGuiWindowFlags_NoCollapse);

    // Get window size for responsive layout
    ImVec2 window_size = ImGui::GetWindowSize();
    timelineWidth = window_size.x - 20.0f; // Account for padding

    // Top controls section - fixed height
    ImGui::BeginChild("Controls", ImVec2(0, 120), ImGuiChildFlags_Borders);

    // First row: Song loading and playback controls
    ImGui::BeginGroup();

    if (ImGui::Button("Load Song", ImVec2(100, 25))) {
        showFileDialog = true;
    }

    ImGui::SameLine();
    if (isSongLoaded) {
        ImGui::Text("Loaded: %s", currentSongName.c_str());
    } else {
        ImGui::Text("No song loaded");
    }

    ImGui::SameLine();
    if (ImGui::Button(isPlaying ? "Pause" : "Play", ImVec2(60, 25))) {
        if (isSongLoaded) {
            if (isPlaying) {
                soundManager->pauseSound("timeline_song");
                isPlaying = false;
            } else {
                soundManager->resumeSound("timeline_song");
                isPlaying = true;
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Stop", ImVec2(60, 25))) {
        if (isSongLoaded) {
            soundManager->stopSound("timeline_song");
            isPlaying = false;
            soundManager->setPosition("timeline_song", 0.0);
            currentPosition = 0.0;
            scrollOffset = 0.0f;
            targetScrollOffset = 0.0f;
        }
    }

    ImGui::EndGroup();

    // Second row: Timeline configuration
    ImGui::Spacing();
    ImGui::BeginGroup();

    ImGui::Text("Configuration:");
    ImGui::SameLine();

        if (ImGui::Checkbox("Show Grid", &showGrid)) {
        calculateGridSpacing();
    }

    ImGui::SameLine();
    if (ImGui::Checkbox("Auto-scroll", &enableAutoscroll)) {
        // Toggle autoscroll
    }

    ImGui::SameLine();
    ImGui::Text("BPM:");
    ImGui::SameLine();
    if (ImGui::InputFloat("##bpm", &bpm, 1.0f, 10.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal)) {
        bpm = std::max(1.0f, bpm);
        calculateGridSpacing();
    }

    ImGui::SameLine();
    ImGui::Text("Grid: %.2fs", gridSpacing);

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::BeginGroup();

    ImGui::Text("Zoom:");
    ImGui::SameLine();
    if (ImGui::SliderFloat("##zoom", &zoomLevel, minZoomLevel, maxZoomLevel, "%.2fx")) {
        float visible_duration = songDuration / zoomLevel;
        float max_scroll = std::max(0.0f, static_cast<float>(songDuration - visible_duration));
        scrollOffset = std::min(scrollOffset, max_scroll);
        targetScrollOffset = scrollOffset;
    }

    ImGui::SameLine();
        if (ImGui::Button("Reset Zoom", ImVec2(80, 20))) {
        zoomLevel = 1.0f;
        scrollOffset = 0.0f;
        targetScrollOffset = 0.0f;
    }

    ImGui::SameLine();
    if (ImGui::Button("Fit to Window", ImVec2(80, 20))) {
        zoomLevel = 1.0f;
        scrollOffset = 0.0f;
        targetScrollOffset = 0.0f;
    }

    if (isSongLoaded) {
        ImGui::SameLine();
        float visible_duration = songDuration / zoomLevel;
        int scroll_min = (int)scrollOffset / 60;
        int scroll_sec = (int)scrollOffset % 60;
        int end_min = (int)(scrollOffset + visible_duration) / 60;
        int end_sec = (int)(scrollOffset + visible_duration) % 60;
        ImGui::Text("View: %d:%02d - %d:%02d", scroll_min, scroll_sec, end_min, end_sec);
    }

    ImGui::EndGroup();

    ImGui::EndChild();

    ImGui::BeginChild("TimelineDisplay", ImVec2(0, 0), ImGuiChildFlags_Borders);

    ImVec2 content_pos = ImGui::GetCursorScreenPos();

    drawTimelineRuler();

    ImGui::InvisibleButton("timeline_area", ImVec2(timelineWidth, timelineHeight));

    // Handle mouse wheel zooming
    if (ImGui::IsItemHovered()) {
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            float zoom_factor = (wheel > 0.0f) ? 1.1f : 0.9f;
            float new_zoom = zoomLevel * zoom_factor;
            new_zoom = std::clamp(new_zoom, minZoomLevel, maxZoomLevel);

            // Zoom towards mouse position
            if (new_zoom != zoomLevel) {
                float mouse_x = ImGui::GetIO().MousePos.x - content_pos.x;
                float visible_duration = songDuration / zoomLevel;
                float mouse_time = scrollOffset + (mouse_x / (timelineWidth / visible_duration));

                zoomLevel = new_zoom;

                // Adjust scroll to keep mouse position fixed
                float new_visible_duration = songDuration / zoomLevel;
                float new_mouse_time = scrollOffset + (mouse_x / (timelineWidth / new_visible_duration));
                scrollOffset += (mouse_time - new_mouse_time);

                // Clamp scroll offset
                float max_scroll = std::max(0.0f, static_cast<float>(songDuration - new_visible_duration));
                scrollOffset = std::clamp(scrollOffset, 0.0f, max_scroll);
                targetScrollOffset = scrollOffset; // Reset target to current position
            }
        }
    }

    drawTimelineGrid();
    drawPlaybackCursor();

    // Playback info overlay
    if (isSongLoaded) {
        ImGui::SetCursorScreenPos(ImVec2(content_pos.x + 10, content_pos.y + 40));
        ImGui::BeginGroup();

        int current_min = (int)currentPosition / 60;
        int current_sec = (int)currentPosition % 60;
        int total_min = (int)songDuration / 60;
        int total_sec = (int)songDuration % 60;

        ImGui::Text("Position: %d:%02d / %d:%02d", current_min, current_sec, total_min, total_sec);
        ImGui::Text("Status: %s", isPlaying ? "Playing" : "Paused");
        ImGui::Text("Zoom: %.2fx | Controls: Space=Play/Pause, <-/->=Seek, Scroll=Zoom", zoomLevel);

        ImGui::EndGroup();
    }

    ImGui::EndChild();

    ImGui::End();

    // File browser dialog
    if (showFileDialog) {
        ImGui::OpenPopup("File Browser");
        showFileDialog = false;
    }

        if (ImGui::BeginPopupModal("File Browser", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

        // Current directory display and manual path input
        ImGui::Text("Current Directory:");
        ImGui::SameLine();

        static char manualPath[512] = "";
        static bool pathUpdated = false;

        if (ImGui::IsWindowAppearing() || pathUpdated) {
            strncpy(manualPath, currentDirectory.c_str(), sizeof(manualPath) - 1);
            pathUpdated = false;
        }

        if (ImGui::InputText("##manualPath", manualPath, sizeof(manualPath), ImGuiInputTextFlags_EnterReturnsTrue)) {
            // Try to navigate to the manually entered path
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

        // Navigation buttons
        if (ImGui::Button("Refresh")) {
            refreshFileList();
        }
        ImGui::SameLine();
        if (ImGui::Button("Home")) {
            const char* homeDir = getenv("HOME");
            if (homeDir) {
                currentDirectory = homeDir;
            } else {
                currentDirectory = ".";
            }
            selectedFileIndex = -1;
            refreshFileList();
            pathUpdated = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::Separator();

                // File browser content
        ImGui::BeginChild("FileList", ImVec2(0, 450), ImGuiChildFlags_Borders);

        // Directories section
        if (!directories.empty()) {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Directories:");
            for (size_t i = 0; i < directories.size(); i++) {
                std::string dirName = directories[i];
                if (dirName == "..") {
                    dirName = "[..] Parent Directory";
                } else {
                    dirName = "[DIR] " + dirName;
                }

                if (ImGui::Selectable(dirName.c_str(), false)) {
                    navigateToDirectory(directories[i]);
                    pathUpdated = true;
                }
            }
            ImGui::Separator();
        }

        // Files section
        if (!files.empty()) {
            ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.8f, 1.0f), "Audio Files:");

            // Add a small table-like layout for better organization
            ImGui::BeginChild("FilesList", ImVec2(0, 0), ImGuiChildFlags_Borders);

            for (size_t i = 0; i < files.size(); i++) {
                std::string fileName = "[AUDIO] " + files[i];
                bool isSelected = (selectedFileIndex == static_cast<int>(i));

                if (ImGui::Selectable(fileName.c_str(), isSelected)) {
                    selectedFileIndex = static_cast<int>(i);
                }

                // Double-click to load
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    std::string fullPath = currentDirectory + "/" + files[i];
                    loadSong(fullPath);
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndChild();
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No audio files found in this directory");
        }

        ImGui::EndChild();

                // Load button for selected file
        if (selectedFileIndex >= 0 && selectedFileIndex < static_cast<int>(files.size())) {
            ImGui::Separator();
            ImGui::Text("Selected: %s", files[selectedFileIndex].c_str());

            if (ImGui::Button("Load Selected File", ImVec2(150, 25))) {
                std::string fullPath = currentDirectory + "/" + files[selectedFileIndex];
                loadSong(fullPath);
                ImGui::CloseCurrentPopup();
            }
        }

        // Status bar
        ImGui::Separator();
        ImGui::Text("Directories: %zu | Audio Files: %zu | Double-click to load", directories.size(), files.size());

        ImGui::EndPopup();
    }
}

void Timeline::refreshFileList() {
    files.clear();
    directories.clear();

    try {
        std::filesystem::path current_path(currentDirectory);

        // Add parent directory option if not at root
        if (current_path != current_path.root_path()) {
            directories.push_back("..");
        }

        for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
            if (entry.is_directory()) {
                directories.push_back(entry.path().filename().string());
            } else {
                // Check if it's an audio file
                std::string extension = entry.path().extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                if (extension == ".mp3" || extension == ".wav" || extension == ".ogg" ||
                    extension == ".flac" || extension == ".m4a" || extension == ".aac") {
                    files.push_back(entry.path().filename().string());
                }
            }
        }

        // Sort directories and files
        std::sort(directories.begin(), directories.end());
        std::sort(files.begin(), files.end());

    } catch (const std::exception& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
    }
}

void Timeline::navigateToDirectory(const std::string& dirName) {
    if (dirName == "..") {
        // Go to parent directory
        std::filesystem::path current_path(currentDirectory);
        currentDirectory = current_path.parent_path().string();
    } else {
        // Go to subdirectory
        std::filesystem::path new_path = std::filesystem::path(currentDirectory) / dirName;
        if (std::filesystem::is_directory(new_path)) {
            currentDirectory = new_path.string();
        }
    }

    selectedFileIndex = -1;
    refreshFileList();
}

} // Windows
} // App
