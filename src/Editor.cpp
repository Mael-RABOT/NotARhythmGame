#include "Editor.hpp"

namespace App {
namespace Windows {

void Editor::drawTimelineLanes() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 content_pos = ImGui::GetCursorScreenPos();
    float timeline_y = content_pos.y + 30.0f;
    float visible_duration = songDuration / zoomLevel;
    float pixels_per_second = timelineWidth / visible_duration;
    float visible_start = scrollOffset;
    float laneHeight = timelineHeight / 2.0f;

    ImU32 laneColors[2] = {
        IM_COL32(40, 60, 80, 120),   // Top lane
        IM_COL32(80, 40, 60, 120)    // Bottom lane
    };
    ImU32 laneBorderColors[2] = {
        IM_COL32(60, 90, 120, 200),
        IM_COL32(120, 60, 90, 200)
    };

    for (int lane = 0; lane < 2; ++lane) {
        float y0 = timeline_y + lane * laneHeight;

        // Lane background
        draw_list->AddRectFilled(
            ImVec2(content_pos.x, y0),
            ImVec2(content_pos.x + timelineWidth, y0 + laneHeight),
            laneColors[lane]
        );

        // Lane border
        draw_list->AddRect(
            ImVec2(content_pos.x, y0),
            ImVec2(content_pos.x + timelineWidth, y0 + laneHeight),
            laneBorderColors[lane],
            0.0f,
            0,
            2.0f
        );

        // Lane label
        const char* laneNames[2] = { "TOP", "BOTTOM" };
        ImVec2 textSize = ImGui::CalcTextSize(laneNames[lane]);
        ImVec2 textPos = ImVec2(content_pos.x + 5, y0 + (laneHeight - textSize.y) * 0.5f);

        draw_list->AddRectFilled(
            ImVec2(textPos.x - 2, textPos.y - 2),
            ImVec2(textPos.x + textSize.x + 2, textPos.y + textSize.y + 2),
            IM_COL32(0, 0, 0, 100)
        );

        draw_list->AddText(
            textPos,
            IM_COL32(255, 255, 255, 255),
            laneNames[lane]
        );
    }

    for (const auto& note : nodeManager.getNotes()) {
        if (note.timestamp < visible_start || note.timestamp > visible_start + visible_duration) continue;

        float x = content_pos.x + (note.timestamp - visible_start) * pixels_per_second;
        float y = timeline_y + note.lane * laneHeight + laneHeight * 0.5f;

        ImU32 noteColor, borderColor;
        float radius = noteRadius;

        bool isMultiSelected = std::find(selectedNoteIds.begin(), selectedNoteIds.end(), note.id) != selectedNoteIds.end();

        if (note.id == selectedNoteId) {
            noteColor = IM_COL32(255, 255, 0, 255);
            borderColor = IM_COL32(255, 200, 0, 255);
            radius = noteRadius + 2.0f;
        } else if (isMultiSelected) {
            noteColor = IM_COL32(255, 165, 0, 220);
            borderColor = IM_COL32(255, 140, 0, 255);
            radius = noteRadius + 1.0f;
        } else if (note.id == hoveredNoteId) {
            noteColor = IM_COL32(255, 200, 100, 200);
            borderColor = IM_COL32(255, 180, 80, 255);
        } else {
            if (note.lane == Core::Lane::TOP) {
                noteColor = IM_COL32(100, 150, 255, 200);
                borderColor = IM_COL32(80, 120, 200, 255);
            } else {
                noteColor = IM_COL32(255, 100, 150, 200);
                borderColor = IM_COL32(200, 80, 120, 255);
            }
        }

        draw_list->AddCircleFilled( // Shadow
            ImVec2(x + 2, y + 2),
            radius,
            IM_COL32(0, 0, 0, 50),
            24
        );

        draw_list->AddCircleFilled( // Body
            ImVec2(x, y),
            radius,
            noteColor,
            24
        );

        draw_list->AddCircle( // Border
            ImVec2(x, y),
            radius,
            borderColor,
            24,
            2.0f
        );

        if (showNoteIds || note.id == selectedNoteId || isMultiSelected) {
            char idText[16];
            snprintf(idText, sizeof(idText), "%d", note.id);
            ImVec2 textSize = ImGui::CalcTextSize(idText);
            ImVec2 textPos = ImVec2(x - textSize.x * 0.5f, y - textSize.y * 0.5f);

            draw_list->AddRectFilled(
                ImVec2(textPos.x - 1, textPos.y - 1),
                ImVec2(textPos.x + textSize.x + 1, textPos.y + textSize.y + 1),
                IM_COL32(0, 0, 0, 150)
            );

            draw_list->AddText(
                textPos,
                IM_COL32(255, 255, 255, 255),
                idText
            );
        }

        if (ImGui::IsMouseHoveringRect(ImVec2(x-radius, y-radius), ImVec2(x+radius, y+radius))) {
            hoveredNoteId = note.id;

            const char* selectionStatus = "";
            if (note.id == selectedNoteId) {
                selectionStatus = " (Primary Selected)";
            } else if (isMultiSelected) {
                selectionStatus = " (Multi-Selected)";
            }

            ImGui::SetTooltip(
                "Note #%d%s\n"
                "Lane: %s\n"
                "Time: %.2fs (%.1f BPM)\n"
                "Click to select\n"
                "Ctrl+Click for multi-select\n"
                "Double-click to delete\n"
                "Drag to move",
                note.id,
                selectionStatus,
                note.lane == Core::Lane::TOP ? "Top" : "Bottom",
                note.timestamp,
                note.timestamp > 0 ? 60.0 / note.timestamp : 0.0
            );
        }
    }

    // Lane separator
    draw_list->AddLine(
        ImVec2(content_pos.x, timeline_y + laneHeight),
        ImVec2(content_pos.x + timelineWidth, timeline_y + laneHeight),
        IM_COL32(200, 200, 200, 150),
        3.0f
    );

    for (int lane = 0; lane < 2; ++lane) {
        float y0 = timeline_y + lane * laneHeight;
        float centerY = y0 + laneHeight * 0.5f;

        draw_list->AddLine(
            ImVec2(content_pos.x, centerY),
            ImVec2(content_pos.x + timelineWidth, centerY),
            IM_COL32(255, 255, 255, 30),
            1.0f
        );
    }
}

void Editor::handleNotePlacementAndInteraction() {
    ImVec2 content_pos = ImGui::GetCursorScreenPos();
    float timeline_y = content_pos.y + 30.0f;
    float visible_duration = songDuration / zoomLevel;
    float pixels_per_second = timelineWidth / visible_duration;
    float visible_start = scrollOffset;
    float laneHeight = timelineHeight / 2.0f;

    ImGui::InvisibleButton("timeline_note_area", ImVec2(timelineWidth, timelineHeight));

    static bool isDragging = false;
    static int draggedNoteId = -1;
    static ImVec2 dragStartPos;

    if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered()) {
        selectedNoteId = -1;
        selectedNoteIds.clear();
        hoveredNoteId = -1;
    }

    if (ImGui::IsItemHovered()) {
        ImVec2 mouse = ImGui::GetIO().MousePos;
        float rel_x = mouse.x - content_pos.x;
        float rel_y = mouse.y - timeline_y;

        bool clickedOnNote = false;
        for (const auto& note : nodeManager.getNotes()) { // Select notes
            float note_x = content_pos.x + (note.timestamp - visible_start) * pixels_per_second;
            float note_y = timeline_y + note.lane * laneHeight + laneHeight * 0.5f;
            float distance = sqrtf((mouse.x - note_x) * (mouse.x - note_x) + (mouse.y - note_y) * (mouse.y - note_y));

            if (distance <= noteRadius) {
                clickedOnNote = true;
                if (ImGui::IsMouseClicked(0)) {
                    bool ctrlPressed = ImGui::GetIO().KeyCtrl;
                    if (ctrlPressed) {
                        auto it = std::find(selectedNoteIds.begin(), selectedNoteIds.end(), note.id);
                        if (it != selectedNoteIds.end()) {
                            selectedNoteIds.erase(it);
                        } else {
                            selectedNoteIds.push_back(note.id);
                        }
                        selectedNoteId = note.id;
                    } else {
                        selectedNoteId = note.id;
                        selectedNoteIds.clear();
                        selectedNoteIds.push_back(note.id);
                    }
                    hoveredNoteId = note.id;
                }
                break;
            }
        }

        if (ImGui::IsMouseClicked(0) && !clickedOnNote) { // Create note on click
            if (rel_x >= 0 && rel_x <= timelineWidth && rel_y >= 0 && rel_y <= timelineHeight) {
                int lane = (rel_y < laneHeight) ? 0 : 1;
                double t = visible_start + (rel_x / pixels_per_second);

                double snapped = t;
                if (snapToGrid) {
                    snapped = std::round(t / gridSpacing) * gridSpacing;
                }

                snapped = std::clamp(snapped, 0.0, songDuration);

                int newNoteId = nodeManager.addNote(lane, snapped);
                selectedNoteId = newNoteId;
                hoveredNoteId = newNoteId;
            }
        }

        if (ImGui::IsMouseDoubleClicked(0)) { // Delete note on double click
            for (const auto& note : nodeManager.getNotes()) {
                float note_x = content_pos.x + (note.timestamp - visible_start) * pixels_per_second;
                float note_y = timeline_y + note.lane * laneHeight + laneHeight * 0.5f;
                float distance = sqrtf((mouse.x - note_x) * (mouse.x - note_x) + (mouse.y - note_y) * (mouse.y - note_y));

                if (distance <= noteRadius) {
                    nodeManager.removeNote(note.id);
                    if (selectedNoteId == note.id) selectedNoteId = -1;
                    if (hoveredNoteId == note.id) hoveredNoteId = -1;
                    break;
                }
            }
        }

        if (ImGui::IsMouseDown(0) && selectedNoteId != -1 && !isDragging) {
            Core::Note* selectedNote = nodeManager.getNoteById(selectedNoteId);
            if (selectedNote) {
                float note_x = content_pos.x + (selectedNote->timestamp - visible_start) * pixels_per_second;
                float note_y = timeline_y + selectedNote->lane * laneHeight + laneHeight * 0.5f;
                float distance = sqrtf((mouse.x - note_x) * (mouse.x - note_x) + (mouse.y - note_y) * (mouse.y - note_y));

                if (distance <= noteRadius) {
                    isDragging = true;
                    draggedNoteId = selectedNoteId;
                    dragStartPos = mouse;
                }
            }
        }
    }

    if (isDragging && ImGui::IsMouseDown(0)) { // Handle dragging
        ImVec2 mouse = ImGui::GetIO().MousePos;
        float rel_x = mouse.x - content_pos.x;
        float rel_y = mouse.y - timeline_y;

        if (rel_x >= 0 && rel_x <= timelineWidth && rel_y >= 0 && rel_y <= timelineHeight) {
            int newLane = (rel_y < laneHeight) ? 0 : 1;
            double newTimestamp = visible_start + (rel_x / pixels_per_second);

            if (snapToGrid) {
                newTimestamp = std::round(newTimestamp / gridSpacing) * gridSpacing;
            }

            newTimestamp = std::clamp(newTimestamp, 0.0, songDuration);

            nodeManager.moveNote(draggedNoteId, newLane, newTimestamp);
        }
    } else if (isDragging && !ImGui::IsMouseDown(0)) {
        isDragging = false;
        draggedNoteId = -1;
    }

    if (selectedNoteId != -1) {
        if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
            if (!selectedNoteIds.empty()) {
                for (int id : selectedNoteIds) {
                    nodeManager.removeNote(id);
                }
                selectedNoteIds.clear();
                selectedNoteId = -1;
            } else {
                nodeManager.removeNote(selectedNoteId);
                selectedNoteId = -1;
            }
        }

        Core::Note* selectedNote = nodeManager.getNoteById(selectedNoteId);
        if (selectedNote) {
            float seekAmount = 5.0f;
            for (const auto& [level, amount] : SeekPerZoom::data) {
                if (zoomLevel >= level) {
                    seekAmount = amount;
                } else {
                    break;
                 }
            }

            bool modified = false;
            double newTimestamp = selectedNote->timestamp;
            int newLane = selectedNote->lane;

            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && ImGui::GetIO().KeyCtrl) {
                newTimestamp -= seekAmount;
                modified = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) && ImGui::GetIO().KeyCtrl) {
                newTimestamp += seekAmount;
                modified = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && ImGui::GetIO().KeyCtrl) {
                newLane = Core::Lane::TOP;
                modified = true;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && ImGui::GetIO().KeyCtrl) {
                newLane = Core::Lane::BOTTOM;
                modified = true;
            }

            if (modified) {
                newTimestamp = std::clamp(newTimestamp, 0.0, songDuration);
                nodeManager.moveNote(selectedNoteId, newLane, newTimestamp);
                jumpToPosition(selectedNote);
            }
        }
    }

    if (!ImGui::IsItemHovered()) {
        hoveredNoteId = -1;
    }
}

Editor::Editor()
    : soundManager(nullptr),
      currentSongPath(""),
      currentSongName(""),
      isSongLoaded(false),
      isPlaying(false),
      currentPosition(0.0),
      songDuration(0.0),
      bpm(120.0f),
      showGrid(true),
      enableAutoscroll(true),
      markerInterval(5.0),
      gridSpacing(0.5f),
      timelineWidth(800.0f),
      timelineHeight(200.0f),
      zoomLevel(1.0f),
      scrollOffset(0.0f),
      targetScrollOffset(0.0f),
      minZoomLevel(1.0f),
      maxZoomLevel(20.0f),
      showFileDialog(false),
      selectedFileIndex(-1),
      selectedNoteId(-1),
      hoveredNoteId(-1),
      showNotesList(false),
      showProperties(false),
      snapToGrid(true),
      showNoteIds(false),
      showMilliseconds(false),
      noteRadius(12.0f),
      sortOrder(SortOrder::TIME),
      showSaveDialog(false),
      showLoadDialog(false),
      chartTitle(""),
      chartArtist("") {

    calculateGridSpacing();
    refreshFileList();

    const char* home = getenv("HOME");
    if (home) {
        currentDirectory = std::string(home) + "/Music";
        if (!std::filesystem::exists(currentDirectory)) {
            currentDirectory = home;
        }
    } else {
        currentDirectory = ".";
    }
    refreshFileList();
}

Editor::Editor(Core::SoundManager* soundManager)
    : soundManager(soundManager),
      currentSongPath(""),
      currentSongName(""),
      isSongLoaded(false),
      isPlaying(false),
      currentPosition(0.0),
      songDuration(0.0),
      bpm(120.0f),
      showGrid(true),
      enableAutoscroll(true),
      markerInterval(5.0),
      gridSpacing(0.5f),
      timelineWidth(800.0f),
      timelineHeight(200.0f),
      zoomLevel(1.0f),
      scrollOffset(0.0f),
      targetScrollOffset(0.0f),
      minZoomLevel(1.0f),
      maxZoomLevel(20.0f),
      showFileDialog(false),
      selectedFileIndex(-1),
      selectedNoteId(-1),
      hoveredNoteId(-1),
      showNotesList(false),
      showProperties(false),
      snapToGrid(true),
      showNoteIds(false),
      showMilliseconds(false),
      noteRadius(12.0f),
      sortOrder(SortOrder::TIME),
      showSaveDialog(false),
      showLoadDialog(false),
      chartTitle(""),
      chartArtist("") {

    calculateGridSpacing();
    refreshFileList();

    const char* home = getenv("HOME");
    if (home) {
        currentDirectory = std::string(home) + "/Music";
        if (!std::filesystem::exists(currentDirectory)) {
            currentDirectory = home;
        }
    } else {
        currentDirectory = ".";
    }
    refreshFileList();
}

void Editor::calculateGridSpacing() {
    float beatDuration = 60.0f / bpm;
    gridSpacing = beatDuration;
}

void Editor::updateAutoscroll() {
    if (!isSongLoaded || songDuration <= 0.0 || !enableAutoscroll) return;

    float visible_duration = songDuration / zoomLevel;

    float target_scroll = currentPosition - (visible_duration * 0.5f);
    target_scroll = std::clamp(target_scroll, 0.0f, std::max(0.0f, static_cast<float>(songDuration - visible_duration)));

    float scroll_speed = 5.0f;
    float delta_time = ImGui::GetIO().DeltaTime;
    float diff = target_scroll - scrollOffset;

    if (std::abs(diff) > 0.05f) {
        scrollOffset += diff * scroll_speed * delta_time;
    } else {
        scrollOffset = target_scroll;
    }

    targetScrollOffset = target_scroll;
}

void Editor::loadSong(const std::string& filepath) {
    if (!soundManager) return;

    size_t lastSlash = filepath.find_last_of("/\\");
    currentSongName = (lastSlash != std::string::npos) ? filepath.substr(lastSlash + 1) : filepath;

    if (soundManager->loadSound("timeline_song", filepath)) {
        currentSongPath = filepath;
        isSongLoaded = true;
        currentPosition = 0.0;
        isPlaying = false;

        songDuration = soundManager->getDuration("timeline_song");

    } else {
        std::cerr << "Failed to load song: " << filepath << std::endl;
    }
}

void Editor::updatePlayback() {
    if (!soundManager || !isSongLoaded) return;

    currentPosition = soundManager->getPosition("timeline_song");

    if (isPlaying && currentPosition >= songDuration) {
        currentPosition = 0.0;
        isPlaying = false;
        soundManager->stopSound("timeline_song");
    }
}

void Editor::handleKeyboardInput() {
    if (!soundManager || !isSongLoaded) return;

    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
        if (isPlaying) {
            soundManager->pauseSound("timeline_song");
            isPlaying = false;
        } else {
            soundManager->resumeSound("timeline_song");
            isPlaying = true;
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        double zero = 0;
        currentPosition = zero;
        soundManager->seekTo("timeline_song", currentPosition);
    }

    float seekAmount = 5.0f;
    for (const auto& [level, amount] : SeekPerZoom::data) {
        if (zoomLevel >= level) {
            seekAmount = amount;
        } else {
            break;
        }
    }

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

    if (ImGui::IsKeyPressed(ImGuiKey_F)) {
        double snapped = currentPosition;
        if (snapToGrid) {
            snapped = std::round(currentPosition / gridSpacing) * gridSpacing;
        }

        snapped = std::clamp(snapped, 0.0, songDuration);

        int newNoteId = nodeManager.addNote(Core::Lane::TOP, snapped);
        selectedNoteId = newNoteId;
        hoveredNoteId = newNoteId;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_J)) {
        double snapped = currentPosition;
        if (snapToGrid) {
            snapped = std::round(currentPosition / gridSpacing) * gridSpacing;
        }

        snapped = std::clamp(snapped, 0.0, songDuration);

        int newNoteId = nodeManager.addNote(Core::Lane::BOTTOM, snapped);
        selectedNoteId = newNoteId;
        hoveredNoteId = newNoteId;
    }
}

void Editor::drawTimelineRuler() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 content_pos = ImGui::GetCursorScreenPos();

    float ruler_height = 30.0f;
    float timeline_y = content_pos.y;

    draw_list->AddRectFilled(
        ImVec2(content_pos.x, timeline_y),
        ImVec2(content_pos.x + timelineWidth, timeline_y + ruler_height),
        IM_COL32(40, 40, 40, 255)
    );

    float visible_duration = songDuration / zoomLevel;
    float pixels_per_second = timelineWidth / visible_duration;
    float visible_start = scrollOffset;
    float visible_end = visible_start + visible_duration;

    // Find the first marker that should be visible
    float first_marker = std::floor(visible_start / markerInterval) * markerInterval;

    for (float time = first_marker; time <= visible_end + markerInterval; time += markerInterval) {
        float x = content_pos.x + (time - visible_start) * pixels_per_second;

        if (x >= content_pos.x - 50 && x <= content_pos.x + timelineWidth + 50) {
            draw_list->AddLine(
                ImVec2(x, timeline_y),
                ImVec2(x, timeline_y + ruler_height),
                IM_COL32(100, 100, 100, 255),
                1.0f
            );

            char time_label[32];
            if (showMilliseconds) {
                int minutes = (int)time / 60;
                int seconds = (int)time % 60;
                int milliseconds = (int)((time - (int)time) * 1000);
                snprintf(time_label, sizeof(time_label), "%d:%02d.%03d", minutes, seconds, milliseconds);
            } else {
                int minutes = (int)time / 60;
                int seconds = (int)time % 60;
                snprintf(time_label, sizeof(time_label), "%d:%02d", minutes, seconds);
            }

            ImGui::SetCursorScreenPos(ImVec2(x + 2, timeline_y + 5));
            ImGui::Text("%s", time_label);
        }
    }
}

void Editor::drawTimelineGrid() {
    if (!showGrid) return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 content_pos = ImGui::GetCursorScreenPos();

    float timeline_y = content_pos.y + 30.0f;
    float visible_duration = songDuration / zoomLevel;
    float pixels_per_second = timelineWidth / visible_duration;
    float visible_start = scrollOffset;
    float visible_end = visible_start + visible_duration;

    float first_grid_line = std::floor(visible_start / gridSpacing) * gridSpacing;

    for (float time = first_grid_line; time <= visible_end + gridSpacing; time += gridSpacing) {
        float x = content_pos.x + (time - visible_start) * pixels_per_second;

        if (x >= content_pos.x - 50 && x <= content_pos.x + timelineWidth + 50) {
            draw_list->AddLine(
                ImVec2(x, timeline_y),
                ImVec2(x, timeline_y + timelineHeight),
                IM_COL32(60, 60, 60, 255),
                1.0f
            );
        }
    }
}

void Editor::drawPlaybackCursor() {
    if (!isSongLoaded) return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 content_pos = ImGui::GetCursorScreenPos();

    float timeline_y = content_pos.y + 30.0f;
    float visible_duration = songDuration / zoomLevel;
    float pixels_per_second = timelineWidth / visible_duration;
    float cursor_x = content_pos.x + (currentPosition - scrollOffset) * pixels_per_second;

    if (cursor_x >= content_pos.x && cursor_x <= content_pos.x + timelineWidth) {
        ImU32 cursorColor = IM_COL32(255, 100, 100, 255);
        ImU32 shadowColor = IM_COL32(0, 0, 0, 100);
        float headSize = 8.0f;

        // Top head
        float topLaneY = timeline_y - timelineHeight - TIMELINE_OFFSET;
        ImVec2 topTriangleTop = ImVec2(cursor_x - headSize, topLaneY);
        ImVec2 topTriangleBottom = ImVec2(cursor_x + headSize, topLaneY);
        ImVec2 topTriangleTip = ImVec2(cursor_x, topLaneY + headSize);

        draw_list->AddTriangleFilled(
            ImVec2(topTriangleTop.x + 1, topTriangleTop.y + 1),
            ImVec2(topTriangleBottom.x + 1, topTriangleBottom.y + 1),
            ImVec2(topTriangleTip.x + 1, topTriangleTip.y + 1),
            shadowColor
        );

        draw_list->AddTriangleFilled(
            topTriangleTop,
            topTriangleBottom,
            topTriangleTip,
            cursorColor
        );

        draw_list->AddTriangle(
            topTriangleTop,
            topTriangleBottom,
            topTriangleTip,
            IM_COL32(255, 255, 255, 255),
            1.0f
        );

        // Bottom head
        float bottomLaneY = timeline_y - TIMELINE_OFFSET;
        ImVec2 bottomTriangleTop = ImVec2(cursor_x - headSize, bottomLaneY);
        ImVec2 bottomTriangleBottom = ImVec2(cursor_x + headSize, bottomLaneY);
        ImVec2 bottomTriangleTip = ImVec2(cursor_x, bottomLaneY - headSize);

        draw_list->AddTriangleFilled(
            ImVec2(bottomTriangleTop.x + 1, bottomTriangleTop.y + 1),
            ImVec2(bottomTriangleBottom.x + 1, bottomTriangleBottom.y + 1),
            ImVec2(bottomTriangleTip.x + 1, bottomTriangleTip.y + 1),
            shadowColor
        );

        draw_list->AddTriangleFilled(
            bottomTriangleTop,
            bottomTriangleBottom,
            bottomTriangleTip,
            cursorColor
        );

        draw_list->AddTriangle(
            bottomTriangleTop,
            bottomTriangleBottom,
            bottomTriangleTip,
            IM_COL32(255, 255, 255, 255),
            1.0f
        );

        char timeLabel[32];
        int current_min = (int)currentPosition / 60;
        int current_sec = (int)currentPosition % 60;
        int current_ms = (int)((currentPosition - (int)currentPosition) * 1000);
        snprintf(timeLabel, sizeof(timeLabel), "%d:%02d.%03d", current_min, current_sec, current_ms);

        ImVec2 textSize = ImGui::CalcTextSize(timeLabel);
        ImVec2 textPos = ImVec2(cursor_x - textSize.x * 0.5f, topLaneY - headSize - textSize.y - TIMELINE_OFFSET);

        draw_list->AddRectFilled(
            ImVec2(textPos.x - 2, textPos.y - 2),
            ImVec2(textPos.x + textSize.x + 2, textPos.y + textSize.y + 2),
            IM_COL32(0, 0, 0, 150)
        );

        draw_list->AddText(
            textPos,
            IM_COL32(255, 255, 255, 255),
            timeLabel
        );
    }
}

void Editor::update() {
    updatePlayback();
    handleKeyboardInput();
    updateAutoscroll();
}

void Editor::render() {
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::BeginGroup();
    ImGui::Text("Audio Controls");
    ImGui::Separator();

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
        if (soundManager && isSongLoaded) {
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
        if (soundManager && isSongLoaded) {
            soundManager->stopSound("timeline_song");
            isPlaying = false;
            soundManager->setPosition("timeline_song", 0.0);
            currentPosition = 0.0;
            scrollOffset = 0.0f;
            targetScrollOffset = 0.0f;
        }
    }

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::BeginGroup();

    ImGui::Text("Level Editor");
    ImGui::Separator();

    if (ImGui::Button("Clear All Notes", ImVec2(120, 25))) {
        nodeManager.clear();
        selectedNoteId = -1;
        hoveredNoteId = -1;
    }

    ImGui::SameLine();
    if (ImGui::Button("Save Chart", ImVec2(100, 25))) {
        if (isSongLoaded) {
            showSaveDialog = true;
        } else {
            ImGui::OpenPopup("No Song Loaded");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Load Chart", ImVec2(100, 25))) {
        showLoadDialog = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Browse Charts", ImVec2(100, 25))) {
        showFileDialog = true;
    }

    ImGui::SameLine();
    if (selectedNoteId != -1) {
        if (ImGui::Button("Delete Note", ImVec2(100, 25))) {
            nodeManager.removeNote(selectedNoteId);
            selectedNoteId = -1;
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("Delete Note", ImVec2(100, 25));
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    if (ImGui::Button("Notes List", ImVec2(100, 25))) {
        showNotesList = !showNotesList;
    }

    ImGui::SameLine();
    if (ImGui::Button("Properties", ImVec2(100, 25))) {
        showProperties = !showProperties;
    }

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::BeginGroup();

    ImGui::Text("Timeline Configuration");
    ImGui::Separator();

    if (ImGui::Checkbox("Show Grid", &showGrid)) {
        calculateGridSpacing();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &enableAutoscroll);

    ImGui::SameLine();
    ImGui::Checkbox("Snap to Grid", &snapToGrid);

    ImGui::SameLine();
    ImGui::Checkbox("Show Note IDs", &showNoteIds);

    ImGui::SameLine();
    ImGui::Checkbox("Show milliseconds", &showMilliseconds);

    ImGui::Text("BPM:");
    ImGui::SameLine();
    if (ImGui::InputFloat("##bpm", &bpm, 1.0f, 10.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal)) {
        bpm = std::max(1.0f, bpm);
        calculateGridSpacing();
    }

    ImGui::SameLine();
    ImGui::Text("Grid: %.2fs", gridSpacing);

    ImGui::Text("Note Radius:");
    ImGui::SameLine();
    ImGui::SliderFloat("##noteRadius", &noteRadius, 2.0f, 20.0f, "%.1f");

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

    ImGui::Text("Marker Interval:");
    ImGui::SameLine();
    if (ImGui::SliderFloat("##markerInterval", &markerInterval, 0.250f, 10.0f, "%.2fs")) {
        markerInterval = std::max(0.250f, markerInterval);
        calculateGridSpacing();
    }

    if (isSongLoaded) {
        ImGui::Text("Song Duration: %.2fs //", songDuration);
        ImGui::SameLine();
        ImGui::Text("Current Position: %.2fs //", currentPosition);
        ImGui::SameLine();
        ImGui::Text("Status: %s //", isPlaying ? "Playing" : "Paused");
        ImGui::SameLine();
        ImGui::Text("Notes: %zu | Selected: %s", nodeManager.getNotes().size(), selectedNoteId == -1 ? "None" : std::to_string(selectedNoteId).c_str());
    }

    ImGui::EndGroup();

    ImGui::End();

    ImGui::Begin("Timeline", nullptr, ImGuiWindowFlags_NoCollapse);

    ImVec2 window_size = ImGui::GetWindowSize();
    timelineWidth = window_size.x - 20.0f;

    ImVec2 content_pos = ImGui::GetCursorScreenPos();

    drawTimelineRuler();

    ImGui::InvisibleButton("timeline_area", ImVec2(timelineWidth, timelineHeight));

    if (ImGui::IsItemHovered()) { // Handle Mouse zoom
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            float zoom_factor = (wheel > 0.0f) ? 1.1f : 0.9f;
            float new_zoom = zoomLevel * zoom_factor;
            new_zoom = std::clamp(new_zoom, minZoomLevel, maxZoomLevel);

            if (new_zoom != zoomLevel) {
                float mouse_x = ImGui::GetIO().MousePos.x - content_pos.x;
                float visible_duration = songDuration / zoomLevel;
                float mouse_time = scrollOffset + (mouse_x / (timelineWidth / visible_duration));

                zoomLevel = new_zoom;

                float new_visible_duration = songDuration / zoomLevel;
                float new_mouse_time = scrollOffset + (mouse_x / (timelineWidth / new_visible_duration));
                scrollOffset += (mouse_time - new_mouse_time);

                float max_scroll = std::max(0.0f, static_cast<float>(songDuration - new_visible_duration));
                scrollOffset = std::clamp(scrollOffset, 0.0f, max_scroll);
                targetScrollOffset = scrollOffset;
            }
        }
    }

    drawTimelineGrid();
    drawTimelineLanes();
    handleNotePlacementAndInteraction();
    drawPlaybackCursor();

    if (isSongLoaded) {
        ImGui::SetCursorScreenPos(ImVec2(content_pos.x + 10, content_pos.y + 40));
        ImGui::BeginGroup();

        int current_min = (int)currentPosition / 60;
        int current_sec = (int)currentPosition % 60;
        int total_min = (int)songDuration / 60;
        int total_sec = (int)songDuration % 60;

        ImGui::Text("Position: %d:%02d / %d:%02d", current_min, current_sec, total_min, total_sec);
        ImGui::Text("Zoom: %.2fx | Controls: Space=Play/Pause, <-/->=Seek, Enter=Move to song start, Scroll=Zoom", zoomLevel);
        ImGui::Text("Editor: Click=Place Note, Double-click=Delete, Drag=Move, Ctrl+Arrows=Fine Adjust");
        ImGui::Text("Multi-select: Ctrl+Click, Delete=Remove Selected, Notes List for bulk operations");

        ImGui::EndGroup();
    }

    ImGui::End();

    if (showFileDialog) {
        ImGui::OpenPopup("File Browser");
        showFileDialog = false;
    }

    if (ImGui::BeginPopupModal("File Browser", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

        ImGui::Text("Current Directory:");
        ImGui::SameLine();

        static char manualPath[512] = "";
        static bool pathUpdated = false;

        if (ImGui::IsWindowAppearing() || pathUpdated) {
            strncpy(manualPath, currentDirectory.c_str(), sizeof(manualPath) - 1);
            pathUpdated = false;
        }

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

        ImGui::BeginChild("FileList", ImVec2(0, 450), ImGuiChildFlags_Borders);

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

        if (!files.empty()) {
            ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.8f, 1.0f), "Files:");

            ImGui::BeginChild("FilesList", ImVec2(0, 0), ImGuiChildFlags_Borders);

            for (size_t i = 0; i < files.size(); i++) {
                std::string extension = std::filesystem::path(files[i]).extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                std::string prefix = "[AUDIO] ";
                if (extension == ".chart") {
                    prefix = "[CHART] ";
                }

                std::string fileName = prefix + files[i];
                bool isSelected = (selectedFileIndex == static_cast<int>(i));

                if (ImGui::Selectable(fileName.c_str(), isSelected)) {
                    selectedFileIndex = static_cast<int>(i);
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    std::string fullPath = currentDirectory + "/" + files[i];
                    if (extension == ".chart") {
                        loadChartFile(fullPath);
                    } else {
                        loadSong(fullPath);
                    }
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndChild();
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No audio or chart files found in this directory");
        }

        ImGui::EndChild();

        if (selectedFileIndex >= 0 && selectedFileIndex < static_cast<int>(files.size())) {
            ImGui::Separator();
            ImGui::Text("Selected: %s", files[selectedFileIndex].c_str());

            if (ImGui::Button("Load Selected File", ImVec2(150, 25))) {
                std::string fullPath = currentDirectory + "/" + files[selectedFileIndex];
                std::string extension = std::filesystem::path(files[selectedFileIndex]).extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                if (extension == ".chart") {
                    loadChartFile(fullPath);
                } else {
                    loadSong(fullPath);
                }
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::Separator();
        ImGui::Text("Directories: %zu | Files: %zu | Double-click to load", directories.size(), files.size());

        ImGui::EndPopup();
    }

    if (showSaveDialog) {
        ImGui::OpenPopup("Save Chart");
        showSaveDialog = false;
    }

    if (ImGui::BeginPopupModal("Save Chart", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Save Chart File");
        ImGui::Separator();

        static char chartName[256] = "";
        static char chartPath[512] = "";

        if (ImGui::IsWindowAppearing()) {
            if (chartTitle.empty()) {
                strncpy(chartName, currentSongName.c_str(), sizeof(chartName) - 1);
            } else {
                strncpy(chartName, chartTitle.c_str(), sizeof(chartName) - 1);
            }
            strncpy(chartPath, currentDirectory.c_str(), sizeof(chartPath) - 1);
        }

        ImGui::Text("Chart Title:");
        ImGui::InputText("##chartTitle", chartName, sizeof(chartName));
        chartTitle = chartName;

        ImGui::Text("Artist:");
        static char artistBuffer[256] = "";
        if (ImGui::IsWindowAppearing()) {
            strncpy(artistBuffer, chartArtist.c_str(), sizeof(artistBuffer) - 1);
        }
        if (ImGui::InputText("##chartArtist", artistBuffer, sizeof(artistBuffer))) {
            chartArtist = artistBuffer;
        }

        ImGui::Text("Save Path:");
        ImGui::InputText("##chartPath", chartPath, sizeof(chartPath));

        ImGui::Separator();

        if (ImGui::Button("Save", ImVec2(80, 25))) {
            std::string fullPath = std::string(chartPath) + "/" + std::string(chartName) + ".chart";
            if (saveChartFile(fullPath)) {
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 25))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (showLoadDialog) {
        ImGui::OpenPopup("Load Chart");
        showLoadDialog = false;
    }

    if (ImGui::BeginPopupModal("Load Chart", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Load Chart File");
        ImGui::Separator();

        static char chartPath[512] = "";

        if (ImGui::IsWindowAppearing()) {
            strncpy(chartPath, currentDirectory.c_str(), sizeof(chartPath) - 1);
        }

        ImGui::Text("Chart File Path:");
        ImGui::InputText("##loadChartPath", chartPath, sizeof(chartPath));

        ImGui::Separator();

        if (ImGui::Button("Load", ImVec2(80, 25))) {
            if (loadChartFile(chartPath)) {
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 25))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("No Song Loaded", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("No song loaded!");
        ImGui::Text("Please load a song first before saving a chart.");
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(80, 25))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    drawNotesList();

    drawPropertiesPanel();
}

void Editor::refreshFileList() {
    files.clear();
    directories.clear();

    try {
        std::filesystem::path current_path(currentDirectory);

        if (current_path != current_path.root_path()) {
            directories.push_back("..");
        }

        for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
            if (entry.is_directory()) {
                directories.push_back(entry.path().filename().string());
            } else {
                std::string extension = entry.path().extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                if (extension == ".mp3" || extension == ".wav" || extension == ".ogg" ||
                    extension == ".flac" || extension == ".m4a" || extension == ".aac") {
                    files.push_back(entry.path().filename().string());
                } else if (extension == ".chart") {
                    files.push_back(entry.path().filename().string());
                }
            }
        }

        std::sort(directories.begin(), directories.end());
        std::sort(files.begin(), files.end());

    } catch (const std::exception& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
    }
}

void Editor::navigateToDirectory(const std::string& dirName) {
    if (dirName == "..") {
        std::filesystem::path current_path(currentDirectory);
        currentDirectory = current_path.parent_path().string();
    } else {
        std::filesystem::path new_path = std::filesystem::path(currentDirectory) / dirName;
        if (std::filesystem::is_directory(new_path)) {
            currentDirectory = new_path.string();
        }
    }

    selectedFileIndex = -1;
    refreshFileList();
}

void Editor::drawNotesList() {
    if (!showNotesList) return;

    ImGui::Begin("Notes List", &showNotesList, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Notes (%zu total)", nodeManager.getNotes().size());
    ImGui::Separator();

    ImGui::Text("Sort by:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Time", sortOrder == SortOrder::TIME)) {
        sortOrder = SortOrder::TIME;
        sortNotes();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Lane", sortOrder == SortOrder::LANE)) {
        sortOrder = SortOrder::LANE;
        sortNotes();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("ID", sortOrder == SortOrder::ID)) {
        sortOrder = SortOrder::ID;
        sortNotes();
    }

    ImGui::Separator();

    if (ImGui::Button("Select All")) {
        selectedNoteIds.clear();
        for (const auto& note : nodeManager.getNotes()) {
            selectedNoteIds.push_back(note.id);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Selection")) {
        selectedNoteIds.clear();
    }

    if (!selectedNoteIds.empty()) {
        if (ImGui::Button(("Delete Selected (" + std::to_string(selectedNoteIds.size()) + ")").c_str())) {
            for (int id : selectedNoteIds) {
                nodeManager.removeNote(id);
            }
            selectedNoteIds.clear();
            if (std::find(selectedNoteIds.begin(), selectedNoteIds.end(), selectedNoteId) != selectedNoteIds.end()) {
                selectedNoteId = -1;
            }
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("Delete Selected (0)");
        ImGui::EndDisabled();
    }

    ImGui::Separator();

    if (ImGui::BeginTable("NotesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Select");
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Lane");
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        for (const auto& note : nodeManager.getNotes()) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            bool isSelected = std::find(selectedNoteIds.begin(), selectedNoteIds.end(), note.id) != selectedNoteIds.end();
            if (ImGui::Checkbox(("##select" + std::to_string(note.id)).c_str(), &isSelected)) {
                if (isSelected) {
                    selectedNoteIds.push_back(note.id);
                } else {
                    selectedNoteIds.erase(std::remove(selectedNoteIds.begin(), selectedNoteIds.end(), note.id), selectedNoteIds.end());
                }
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", note.id);

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", note.lane == Core::Lane::TOP ? "Top" : "Bottom");

            ImGui::TableSetColumnIndex(3);
            int minutes = (int)note.timestamp / 60;
            int seconds = (int)note.timestamp % 60;
            int centiseconds = (int)((note.timestamp - (int)note.timestamp) * 100);
            ImGui::Text("%d:%02d.%02d", minutes, seconds, centiseconds);

            ImGui::TableSetColumnIndex(4);
            if (ImGui::Button(("Select##" + std::to_string(note.id)).c_str())) {
                selectedNoteId = note.id;
            }
            ImGui::SameLine();
            if (ImGui::Button(("Delete##" + std::to_string(note.id)).c_str())) {
                nodeManager.removeNote(note.id);
                selectedNoteIds.erase(std::remove(selectedNoteIds.begin(), selectedNoteIds.end(), note.id), selectedNoteIds.end());
                if (selectedNoteId == note.id) selectedNoteId = -1;
                break;
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void Editor::jumpToPosition(Core::Note* note) {
    currentPosition = note->timestamp;
    soundManager->seekTo("timeline_song", currentPosition);

    float visible_duration = songDuration / zoomLevel;
    float visible_start = scrollOffset;
    float visible_end = visible_start + visible_duration;
    float margin = visible_duration * 0.1f;

    if (note->timestamp < visible_start + margin || note->timestamp > visible_end - margin) {
        float new_scroll = note->timestamp - (visible_duration * 0.5f);
        new_scroll = std::clamp(new_scroll, 0.0f, std::max(0.0f, static_cast<float>(songDuration - visible_duration)));
        scrollOffset = new_scroll;
        targetScrollOffset = new_scroll;
    }
}

void Editor::drawPropertiesPanel() {
    if (!showProperties) return;

    ImGui::Begin("Properties", &showProperties, ImGuiWindowFlags_AlwaysAutoResize);

    if (selectedNoteId != -1) {
        Core::Note* selectedNote = nodeManager.getNoteById(selectedNoteId);
        if (selectedNote) {
            ImGui::Text("Selected Note #%d", selectedNote->id);
            ImGui::Separator();

            ImGui::Text("Lane:");
            if (ImGui::RadioButton("Top", selectedNote->lane == Core::Lane::TOP)) {
                nodeManager.moveNote(selectedNote->id, 0, selectedNote->timestamp);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Bottom", selectedNote->lane == Core::Lane::BOTTOM)) {
                nodeManager.moveNote(selectedNote->id, 1, selectedNote->timestamp);
            }

            ImGui::Text("Time (seconds):");
            float timeValue = static_cast<float>(selectedNote->timestamp);
            if (ImGui::InputFloat("##time", &timeValue, 0.1f, 1.0f, "%.3f")) {
                double newTime = std::clamp(static_cast<double>(timeValue), 0.0, songDuration);
                if (snapToGrid) {
                    newTime = std::round(newTime / gridSpacing) * gridSpacing;
                }
                nodeManager.moveNote(selectedNote->id, selectedNote->lane, newTime);
            }

            int minutes = (int)selectedNote->timestamp / 60;
            int seconds = (int)selectedNote->timestamp % 60;
            int centiseconds = (int)((selectedNote->timestamp - (int)selectedNote->timestamp) * 100);
            ImGui::Text("Formatted: %d:%02d.%02d", minutes, seconds, centiseconds);

            if (selectedNote->timestamp > 0) {
                float bpmAtNote = 60.0f / selectedNote->timestamp;
                ImGui::Text("BPM at note: %.1f", bpmAtNote);
            }

            ImGui::Separator();

            if (ImGui::Button("Delete Note")) {
                nodeManager.removeNote(selectedNote->id);
                selectedNoteId = -1;
            }

            ImGui::SameLine();
            if (ImGui::Button("Jump to Note")) {
                jumpToPosition(selectedNote);
            }
        }
    } else {
        ImGui::Text("No note selected");
        ImGui::Text("Click on a note in the timeline to view its properties");
    }

    ImGui::End();
}

void Editor::sortNotes() {
    std::vector<Core::Note>& notes = nodeManager.getNotes();

    switch (sortOrder) {
        case SortOrder::TIME:
            std::sort(notes.begin(), notes.end(), [](const Core::Note& a, const Core::Note& b) {
                return a.timestamp < b.timestamp;
            });
            break;
        case SortOrder::LANE:
            std::sort(notes.begin(), notes.end(), [](const Core::Note& a, const Core::Note& b) {
                if (a.lane != b.lane) return a.lane < b.lane;
                return a.timestamp < b.timestamp;
            });
            break;
        case SortOrder::ID:
            std::sort(notes.begin(), notes.end(), [](const Core::Note& a, const Core::Note& b) {
                return a.id < b.id;
            });
            break;
    }
}

bool Editor::saveChartFile(const std::string& filepath) {
    if (!isSongLoaded || currentSongPath.empty()) {
        std::cerr << "No song loaded to save" << std::endl;
        return false;
    }

    std::vector<char> audioData = readAudioFile(currentSongPath);
    if (audioData.empty()) {
        std::cerr << "Failed to read audio file: " << currentSongPath << std::endl;
        return false;
    }

    ChartHeader header;
    memset(&header, 0, sizeof(ChartHeader));
    strcpy(header.magic, "NOTARHYTHM");
    header.version = 1;
    header.headerSize = sizeof(ChartHeader);
    header.audioSize = static_cast<uint32_t>(audioData.size());
    header.notesCount = static_cast<uint32_t>(nodeManager.getNotes().size());
    header.bpm = bpm;
    header.duration = songDuration;

    strncpy(header.title, chartTitle.c_str(), sizeof(header.title) - 1);
    strncpy(header.artist, chartArtist.c_str(), sizeof(header.artist) - 1);

    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to create chart file: " << filepath << std::endl;
        return false;
    }

    file.write(reinterpret_cast<const char*>(&header), sizeof(ChartHeader));
    if (!file.good()) {
        std::cerr << "Failed to write header" << std::endl;
        return false;
    }

    file.write(audioData.data(), audioData.size());
    if (!file.good()) {
        std::cerr << "Failed to write audio data" << std::endl;
        return false;
    }

    for (const auto& note : nodeManager.getNotes()) {
        file.write(reinterpret_cast<const char*>(&note.id), sizeof(note.id));
        file.write(reinterpret_cast<const char*>(&note.lane), sizeof(note.lane));
        file.write(reinterpret_cast<const char*>(&note.timestamp), sizeof(note.timestamp));
    }

    file.close();
    return true;
}

bool Editor::loadChartFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open chart file: " << filepath << std::endl;
        return false;
    }

    ChartHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(ChartHeader));
    if (!file.good()) {
        std::cerr << "Failed to read chart header" << std::endl;
        return false;
    }

    if (strcmp(header.magic, "NOTARHYTHM") != 0) {
        std::cerr << "Invalid chart file format - wrong magic number: " << header.magic << std::endl;
        return false;
    }

    if (header.version != 1) {
        std::cerr << "Unsupported chart file version: " << header.version << std::endl;
        return false;
    }

    std::vector<char> audioData(header.audioSize);
    file.read(audioData.data(), header.audioSize);
    if (!file.good()) {
        std::cerr << "Failed to read audio data" << std::endl;
        return false;
    }

    std::filesystem::path chartPath(filepath);
    std::string tempAudioPath = chartPath.parent_path().string() + "/temp_audio_" + chartPath.stem().string() + ".wav";
    if (!writeAudioFile(tempAudioPath, audioData)) {
        std::cerr << "Failed to write temporary audio file: " << tempAudioPath << std::endl;
        return false;
    }

    if (!soundManager->loadSound("timeline_song", tempAudioPath)) {
        std::cerr << "Failed to load audio from chart" << std::endl;
        std::filesystem::remove(tempAudioPath);
        return false;
    }

    currentSongPath = tempAudioPath;
    currentSongName = header.title;
    isSongLoaded = true;
    currentPosition = 0.0;
    isPlaying = false;
    songDuration = header.duration;
    bpm = header.bpm;
    chartTitle = header.title;
    chartArtist = header.artist;

    nodeManager.clear();
    selectedNoteId = -1;
    hoveredNoteId = -1;
    selectedNoteIds.clear();

    for (uint32_t i = 0; i < header.notesCount; ++i) {
        int id;
        Core::Lane lane;
        double timestamp;

        file.read(reinterpret_cast<char*>(&id), sizeof(id));
        file.read(reinterpret_cast<char*>(&lane), sizeof(lane));
        file.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));

        if (!file.good()) {
            std::cerr << "Failed to read note " << i << std::endl;
            break;
        }

        nodeManager.addNoteWithId(id, static_cast<int>(lane), timestamp);
    }

    file.close();
    calculateGridSpacing();

    return true;
}

std::vector<char> Editor::readAudioFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open audio file: " << filepath << std::endl;
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

bool Editor::writeAudioFile(const std::string& filepath, const std::vector<char>& audioData) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to create audio file: " << filepath << std::endl;
        return false;
    }

    file.write(audioData.data(), audioData.size());
    file.close();
    return true;
}

void Editor::extractAudioFromChart(const std::string& chartPath, const std::string& outputPath) {
    std::ifstream file(chartPath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open chart file: " << chartPath << std::endl;
        return;
    }

    ChartHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(ChartHeader));
    if (!file.good() || strcmp(header.magic, "NOTARHYTHM") != 0) {
        std::cerr << "Invalid chart file format" << std::endl;
        return;
    }

    std::vector<char> audioData(header.audioSize);
    file.read(audioData.data(), header.audioSize);
    file.close();

    writeAudioFile(outputPath, audioData);
}

} // Windows
} // App
