#include "Editor.hpp"
#include <thread>

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
        if (note.type == Core::NoteType::HOLD) {
            if ((note.timestamp < visible_start && note.endTimestamp < visible_start) ||
                (note.timestamp > visible_start + visible_duration && note.endTimestamp > visible_start + visible_duration)) {
                continue;
            }
        } else {
            if (note.timestamp < visible_start || note.timestamp > visible_start + visible_duration) continue;
        }

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
            if (note.type == Core::NoteType::HOLD) {
                if (note.lane == Core::Lane::TOP) {
                    noteColor = IM_COL32(100, 150, 255, 180);
                    borderColor = IM_COL32(80, 120, 200, 255);
                } else {
                    noteColor = IM_COL32(255, 100, 150, 180);
                    borderColor = IM_COL32(200, 80, 120, 255);
                }
            } else {
                if (note.lane == Core::Lane::TOP) {
                    noteColor = IM_COL32(100, 150, 255, 200);
                    borderColor = IM_COL32(80, 120, 200, 255);
                } else {
                    noteColor = IM_COL32(255, 100, 150, 200);
                    borderColor = IM_COL32(200, 80, 120, 255);
                }
            }
        }

        if (note.type == Core::NoteType::HOLD) {
            float endX = content_pos.x + (note.endTimestamp - visible_start) * pixels_per_second;

            float drawStartX = std::max(x, content_pos.x);
            float drawEndX = std::min(endX, content_pos.x + timelineWidth);

            if (drawEndX > drawStartX) {
                float holdHeight = radius * 1.5f;

                draw_list->AddRectFilled(
                    ImVec2(drawStartX, y - holdHeight * 0.5f),
                    ImVec2(drawEndX, y + holdHeight * 0.5f),
                    noteColor
                );

                draw_list->AddRect(
                    ImVec2(drawStartX, y - holdHeight * 0.5f),
                    ImVec2(drawEndX, y + holdHeight * 0.5f),
                    borderColor,
                    0.0f,
                    0,
                    2.0f
                );
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

        if (note.type == Core::NoteType::HOLD) {
            float endX = content_pos.x + (note.endTimestamp - visible_start) * pixels_per_second;
            if (endX >= content_pos.x && endX <= content_pos.x + timelineWidth) {
                draw_list->AddCircleFilled( // End shadow
                    ImVec2(endX + 2, y + 2),
                    radius * 0.7f,
                    IM_COL32(0, 0, 0, 50),
                    24
                );

                draw_list->AddCircleFilled( // End body
                    ImVec2(endX, y),
                    radius * 0.7f,
                    noteColor,
                    24
                );

                draw_list->AddCircle( // End border
                    ImVec2(endX, y),
                    radius * 0.7f,
                    borderColor,
                    24,
                    2.0f
                );
            }
        }

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

            const char* noteTypeStr = (note.type == Core::NoteType::HOLD) ? "HOLD" : "TAP";

            if (note.type == Core::NoteType::HOLD) {
                ImGui::SetTooltip(
                    "Note #%d%s\n"
                    "Type: %s\n"
                    "Lane: %s\n"
                    "Start: %.2fs\n"
                    "End: %.2fs\n"
                    "Duration: %.2fs\n"
                    "Click to select\n"
                    "Ctrl+Click for multi-select\n"
                    "Double-click to delete\n"
                    "Drag to move",
                    note.id,
                    selectionStatus,
                    noteTypeStr,
                    note.lane == Core::Lane::TOP ? "Top" : "Bottom",
                    note.timestamp,
                    note.endTimestamp,
                    note.endTimestamp - note.timestamp
                );
            } else {
                ImGui::SetTooltip(
                    "Note #%d%s\n"
                    "Type: %s\n"
                    "Lane: %s\n"
                    "Time: %.2fs (%.1f BPM)\n"
                    "Click to select\n"
                    "Ctrl+Click for multi-select\n"
                    "Double-click to delete\n"
                    "Drag to move",
                    note.id,
                    selectionStatus,
                    noteTypeStr,
                    note.lane == Core::Lane::TOP ? "Top" : "Bottom",
                    note.timestamp,
                    note.timestamp > 0 ? 60.0 / note.timestamp : 0.0
                );
            }
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
        bool clickingOnOtherWindow = false;
        if (showNotesList && ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
            clickingOnOtherWindow = true;
        }
        if (showProperties && ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
            clickingOnOtherWindow = true;
        }

        if (!clickingOnOtherWindow) {
            selectedNoteId = -1;
            selectedNoteIds.clear();
            hoveredNoteId = -1;
        }
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

            bool noteClicked = false;
            if (distance <= noteRadius) {
                noteClicked = true;
            } else if (note.type == Core::NoteType::HOLD) {
                float end_x = content_pos.x + (note.endTimestamp - visible_start) * pixels_per_second;
                float end_distance = sqrtf((mouse.x - end_x) * (mouse.x - end_x) + (mouse.y - note_y) * (mouse.y - note_y));
                if (end_distance <= noteRadius * 0.7f) {
                    noteClicked = true;
                }
            }

            if (noteClicked) {
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

                // Check if Shift is held to create a hold note
                bool shiftPressed = ImGui::GetIO().KeyShift;
                if (shiftPressed) {
                    // Create a hold note with default duration (1 beat)
                    double endTime = snapped + gridSpacing;
                    endTime = std::clamp(endTime, snapped, songDuration);
                    int newNoteId = nodeManager.addHoldNote(lane, snapped, endTime);
                    selectedNoteId = newNoteId;
                    hoveredNoteId = newNoteId;
                } else {
                    int newNoteId = nodeManager.addNote(lane, snapped);
                    selectedNoteId = newNoteId;
                    hoveredNoteId = newNoteId;
                }
            }
        }

        if (ImGui::IsMouseDoubleClicked(0)) { // Delete note on double click
            for (const auto& note : nodeManager.getNotes()) {
                float note_x = content_pos.x + (note.timestamp - visible_start) * pixels_per_second;
                float note_y = timeline_y + note.lane * laneHeight + laneHeight * 0.5f;
                float distance = sqrtf((mouse.x - note_x) * (mouse.x - note_x) + (mouse.y - note_y) * (mouse.y - note_y));

                bool noteClicked = false;
                if (distance <= noteRadius) {
                    noteClicked = true;
                } else if (note.type == Core::NoteType::HOLD) {
                    float end_x = content_pos.x + (note.endTimestamp - visible_start) * pixels_per_second;
                    float end_distance = sqrtf((mouse.x - end_x) * (mouse.x - end_x) + (mouse.y - note_y) * (mouse.y - note_y));
                    if (end_distance <= noteRadius * 0.7f) {
                        noteClicked = true;
                    }
                }

                if (noteClicked) {
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

                bool noteClicked = false;
                if (distance <= noteRadius) {
                    noteClicked = true;
                } else if (selectedNote->type == Core::NoteType::HOLD) {
                    float end_x = content_pos.x + (selectedNote->endTimestamp - visible_start) * pixels_per_second;
                    float end_distance = sqrtf((mouse.x - end_x) * (mouse.x - end_x) + (mouse.y - note_y) * (mouse.y - note_y));
                    if (end_distance <= noteRadius * 0.7f) {
                        noteClicked = true;
                    }
                }

                if (noteClicked) {
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

            Core::Note* draggedNote = nodeManager.getNoteById(draggedNoteId);
            if (draggedNote && draggedNote->type == Core::NoteType::HOLD) {
                // For hold notes, maintain the duration when dragging
                double duration = draggedNote->endTimestamp - draggedNote->timestamp;
                double newEndTimestamp = newTimestamp + duration;
                newEndTimestamp = std::clamp(newEndTimestamp, newTimestamp, songDuration);
                nodeManager.moveHoldNote(draggedNoteId, newLane, newTimestamp, newEndTimestamp);
            } else {
                nodeManager.moveNote(draggedNoteId, newLane, newTimestamp);
            }
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

    static bool isRightDragging = false;
    static ImVec2 rightDragStartPos;

    if (ImGui::IsMouseClicked(1) && ImGui::IsItemHovered()) {
        isRightDragging = true;
        rightDragStartPos = ImGui::GetIO().MousePos;
    }

    if (isRightDragging && ImGui::IsMouseDown(1)) {
        ImVec2 mouse = ImGui::GetIO().MousePos;
        float rel_x = mouse.x - content_pos.x;

        if (rel_x >= 0 && rel_x <= timelineWidth) {
            double newPosition = visible_start + (rel_x / pixels_per_second);
            newPosition = std::clamp(newPosition, 0.0, songDuration);

            if (soundManager) {
                soundManager->setPosition("timeline_song", newPosition);
                currentPosition = newPosition;

                if (enableAutoscroll) {
                    float visible_duration = songDuration / zoomLevel;
                    float border_threshold = visible_duration * 0.2f;

                    if (newPosition < scrollOffset + border_threshold) {
                        float target_scroll = newPosition - (visible_duration * 0.3f);
                        target_scroll = std::max(0.0f, target_scroll);
                        scrollOffset = target_scroll;
                        targetScrollOffset = target_scroll;
                    } else if (newPosition > scrollOffset + visible_duration - border_threshold) {
                        float target_scroll = newPosition - (visible_duration * 0.7f);
                        target_scroll = std::min(target_scroll, std::max(0.0f, static_cast<float>(songDuration - visible_duration)));
                        scrollOffset = target_scroll;
                        targetScrollOffset = target_scroll;
                    }
                }
            }
        }
    } else if (isRightDragging && !ImGui::IsMouseDown(1)) {
        isRightDragging = false;
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
      audioAnalyzer(std::make_unique<AudioAnalyzer>()),
      showWaveform(true),
      waveformLoaded(false),
      isAnalyzing(false),
      analysisProgress(""),
      analysisProgressPercent(0.0f),
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
      minZoomLevel(MIN_ZOOM_LEVEL),
      maxZoomLevel(MAX_ZOOM_LEVEL),
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
      showHelpWindow(false),
      chartTitle(""),
      chartArtist(""),
      speedOverrideEnabled(false),
      playbackSpeed(0.5f),
      originalPlaybackSpeed(1.0f) {

    calculateGridSpacing();

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
      audioAnalyzer(std::make_unique<AudioAnalyzer>()),
      showWaveform(true),
      waveformLoaded(false),
      isAnalyzing(false),
      analysisProgress(""),
      analysisProgressPercent(0.0f),
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
      minZoomLevel(MIN_ZOOM_LEVEL),
      maxZoomLevel(MAX_ZOOM_LEVEL),
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
      showHelpWindow(false),
      chartTitle(""),
      chartArtist(""),
      speedOverrideEnabled(false),
      playbackSpeed(0.5f),
      originalPlaybackSpeed(1.0f) {

    calculateGridSpacing();

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
    if (showFileDialog || showSaveDialog || showLoadDialog) return;

    if (ImGui::IsPopupOpen("File Browser") || ImGui::IsPopupOpen("Save Chart") ||
        ImGui::IsPopupOpen("Load Chart") || ImGui::IsPopupOpen("No Song Loaded")) return;

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

    if (ImGui::IsKeyPressed(ImGuiKey_S) && !ImGui::GetIO().KeyCtrl) {
        speedOverrideEnabled = !speedOverrideEnabled;
        if (soundManager && isSongLoaded) {
            if (speedOverrideEnabled) {
                originalPlaybackSpeed = soundManager->getCurrentPlaybackSpeed("timeline_song");
                soundManager->setPlaybackSpeed("timeline_song", playbackSpeed);
            } else {
                soundManager->setPlaybackSpeed("timeline_song", originalPlaybackSpeed);
            }
        }
    }

    if (speedOverrideEnabled) {
        if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) {
            playbackSpeed = std::min(2.0f, playbackSpeed + 0.1f);
            if (soundManager && isSongLoaded) {
                soundManager->setPlaybackSpeed("timeline_song", playbackSpeed);
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
            playbackSpeed = std::max(0.1f, playbackSpeed - 0.1f);
            if (soundManager && isSongLoaded) {
                soundManager->setPlaybackSpeed("timeline_song", playbackSpeed);
            }
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_H) && ImGui::GetIO().KeyCtrl) {
        showHelpWindow = !showHelpWindow;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_N) && ImGui::GetIO().KeyCtrl) {
        showNotesList = !showNotesList;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_P) && ImGui::GetIO().KeyCtrl) {
        showProperties = !showProperties;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_O) && ImGui::GetIO().KeyCtrl) {
        showFileDialog = true;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::GetIO().KeyCtrl) {
        if (isSongLoaded) {
            showSaveDialog = true;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_L) && ImGui::GetIO().KeyCtrl) {
        showLoadDialog = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && selectedNoteId != -1) {
        nodeManager.removeNote(selectedNoteId);
        selectedNoteId = -1;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::GetIO().KeyCtrl) {
        nodeManager.clear();
        selectedNoteId = -1;
        hoveredNoteId = -1;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_G)) {
        showGrid = !showGrid;
        calculateGridSpacing();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_A)) {
        enableAutoscroll = !enableAutoscroll;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_T)) {
        snapToGrid = !snapToGrid;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_I)) {
        showNoteIds = !showNoteIds;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_M)) {
        showMilliseconds = !showMilliseconds;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_W)) {
        showWaveform = !showWaveform;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) && ImGui::GetIO().KeyCtrl) {
        zoomLevel = std::min(maxZoomLevel, zoomLevel * 1.2f);
        float visible_duration = songDuration / zoomLevel;
        float max_scroll = std::max(0.0f, static_cast<float>(songDuration - visible_duration));
        scrollOffset = std::min(scrollOffset, max_scroll);
        targetScrollOffset = scrollOffset;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) && ImGui::GetIO().KeyCtrl) {
        zoomLevel = std::max(minZoomLevel, zoomLevel / 1.2f);
        float visible_duration = songDuration / zoomLevel;
        float max_scroll = std::max(0.0f, static_cast<float>(songDuration - visible_duration));
        scrollOffset = std::min(scrollOffset, max_scroll);
        targetScrollOffset = scrollOffset;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_0) && ImGui::GetIO().KeyCtrl) {
        zoomLevel = 1.0f;
        scrollOffset = 0.0f;
        targetScrollOffset = 0.0f;
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

        bool shiftPressed = ImGui::GetIO().KeyShift;
        if (shiftPressed) {
            double endTime = snapped + gridSpacing;
            endTime = std::clamp(endTime, snapped, songDuration);
            int newNoteId = nodeManager.addHoldNote(Core::Lane::TOP, snapped, endTime);
            selectedNoteId = newNoteId;
            hoveredNoteId = newNoteId;
        } else {
            int newNoteId = nodeManager.addNote(Core::Lane::TOP, snapped);
            selectedNoteId = newNoteId;
            hoveredNoteId = newNoteId;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_J)) {
        double snapped = currentPosition;
        if (snapToGrid) {
            snapped = std::round(currentPosition / gridSpacing) * gridSpacing;
        }

        snapped = std::clamp(snapped, 0.0, songDuration);

        bool shiftPressed = ImGui::GetIO().KeyShift;
        if (shiftPressed) {
            double endTime = snapped + gridSpacing;
            endTime = std::clamp(endTime, snapped, songDuration);
            int newNoteId = nodeManager.addHoldNote(Core::Lane::BOTTOM, snapped, endTime);
            selectedNoteId = newNoteId;
            hoveredNoteId = newNoteId;
        } else {
            int newNoteId = nodeManager.addNote(Core::Lane::BOTTOM, snapped);
            selectedNoteId = newNoteId;
            hoveredNoteId = newNoteId;
        }
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

    ImGui::Text("Speed Control");
    ImGui::Separator();

    if (ImGui::Button(speedOverrideEnabled ? "Speed Override: ON" : "Speed Override: OFF", ImVec2(150, 25))) {
        speedOverrideEnabled = !speedOverrideEnabled;
        if (soundManager && isSongLoaded) {
            if (speedOverrideEnabled) {
                originalPlaybackSpeed = soundManager->getCurrentPlaybackSpeed("timeline_song");
                soundManager->setPlaybackSpeed("timeline_song", playbackSpeed);
            } else {
                soundManager->setPlaybackSpeed("timeline_song", originalPlaybackSpeed);
            }
        }
    }

    ImGui::SameLine();
    ImGui::Text("(S to toggle)");

    if (speedOverrideEnabled) {
        float currentSpeed = soundManager ? soundManager->getCurrentPlaybackSpeed("timeline_song") : playbackSpeed;
        ImGui::Text("Playback Speed: %.2fx", currentSpeed);
        if (ImGui::SliderFloat("##playbackSpeed", &playbackSpeed, 0.1f, 2.0f, "%.2fx")) {
            if (soundManager && isSongLoaded) {
                soundManager->setPlaybackSpeed("timeline_song", playbackSpeed);
            }
        }
        ImGui::Text("(+/- keys to adjust)");
    } else {
        ImGui::BeginDisabled();
        ImGui::Text("Playback Speed: %.2fx", playbackSpeed);
        ImGui::SliderFloat("##playbackSpeed", &playbackSpeed, 0.1f, 2.0f, "%.2fx");
        ImGui::EndDisabled();
    }

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::BeginGroup();

    ImGui::Text("Level Editor");
    ImGui::Separator();

    if (ImGui::Button("Clear All Notes (Ctrl+C)", ImVec2(120, 25))) {
        nodeManager.clear();
        selectedNoteId = -1;
        hoveredNoteId = -1;
    }

    ImGui::SameLine();
    if (ImGui::Button("Save Chart (Ctrl+S)", ImVec2(100, 25))) {
        if (isSongLoaded) {
            showSaveDialog = true;
        } else {
            ImGui::OpenPopup("No Song Loaded");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Load Chart (Ctrl+L)", ImVec2(100, 25))) {
        showLoadDialog = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Browse Charts (Ctrl+O)", ImVec2(100, 25))) {
        showFileDialog = true;
    }

    ImGui::SameLine();
    if (selectedNoteId != -1) {
        if (ImGui::Button("Delete Note (Del)", ImVec2(100, 25))) {
            nodeManager.removeNote(selectedNoteId);
            selectedNoteId = -1;
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("Delete Note (Del)", ImVec2(100, 25));
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    if (ImGui::Button("Notes List (Ctrl+N)", ImVec2(100, 25))) {
        showNotesList = !showNotesList;
    }

    ImGui::SameLine();
    if (ImGui::Button("Properties (Ctrl+P)", ImVec2(100, 25))) {
        showProperties = !showProperties;
    }

    ImGui::SameLine();
    if (ImGui::Button("Help (Ctrl+H)", ImVec2(100, 25))) {
        showHelpWindow = !showHelpWindow;
    }

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::BeginGroup();

    ImGui::Text("Timeline Configuration");
    ImGui::Separator();

    if (ImGui::Checkbox("Show Grid (G)", &showGrid)) {
        calculateGridSpacing();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll (A)", &enableAutoscroll);

    ImGui::SameLine();
    ImGui::Checkbox("Snap to Grid (T)", &snapToGrid);

    ImGui::SameLine();
    ImGui::Checkbox("Show Note IDs (I)", &showNoteIds);

    ImGui::SameLine();
    ImGui::Checkbox("Show milliseconds (M)", &showMilliseconds);

    ImGui::SameLine();
    ImGui::Checkbox("Show Waveform (W)", &showWaveform);

    ImGui::SameLine();
    if (isSongLoaded && !isAnalyzing && !waveformLoaded) {
        if (ImGui::Button("Generate Waveform", ImVec2(140, 20))) {
            analyzeAudioFile(currentSongPath);
        }
    } else if (isAnalyzing) {
        ImGui::BeginDisabled();
        ImGui::Button("Generating...", ImVec2(140, 20));
        ImGui::EndDisabled();
    } else if (waveformLoaded) {
        if (ImGui::Button("Regenerate Waveform", ImVec2(140, 20))) {
            analyzeAudioFile(currentSongPath);
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("Generate Waveform", ImVec2(140, 20));
        ImGui::EndDisabled();
    }

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
    if (ImGui::Button("Reset Zoom (Ctrl+0)", ImVec2(80, 20))) {
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
    drawWaveform();
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
        ImGui::Text("Editor: Click=Place Note, Shift+Click=Place Hold Note, Double-click=Delete, Drag=Move, Ctrl+Arrows=Fine Adjust, Right-drag=Move Playhead");
        ImGui::Text("Multi-select: Ctrl+Click, Delete=Remove Selected, Notes List for bulk operations");
        ImGui::Text("Note Types: TAP (press key), HOLD (hold key for duration)");

        if (isAnalyzing) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Analyzing waveform: %s (%.1f%%)", analysisProgress.c_str(), analysisProgressPercent);
            ImGui::ProgressBar(analysisProgressPercent / 100.0f, ImVec2(-1, 0), "");
        } else if (waveformLoaded && showWaveform) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Waveform loaded (%zu samples, %.1fs duration)",
                              waveformData.data.size(), waveformData.duration);
        } else if (showWaveform && isSongLoaded && !waveformLoaded) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Waveform not generated - click 'Generate Waveform' to analyze audio");
        }

        if (showWaveform && waveformLoaded) {
            ImGui::Separator();
            ImGui::Text("Waveform Colors:");

            ImGui::BeginGroup();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            ImGui::Text("Strong Bass/Rhythm");
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.4f, 1.0f));
            ImGui::Text("Moderate Intensity");
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
            ImGui::Text("Low Intensity");
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
            ImGui::Text("Very Low");
            ImGui::PopStyleColor();

            ImGui::EndGroup();
        }

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

    drawHelpWindow();
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

    if (ImGui::BeginTable("NotesTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("Select");
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Type");
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
            ImGui::Text("%s", note.type == Core::NoteType::HOLD ? "HOLD" : "TAP");

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", note.lane == Core::Lane::TOP ? "Top" : "Bottom");

            ImGui::TableSetColumnIndex(4);
            if (note.type == Core::NoteType::HOLD) {
                int start_min = (int)note.timestamp / 60;
                int start_sec = (int)note.timestamp % 60;
                int start_cs = (int)((note.timestamp - (int)note.timestamp) * 100);
                int end_min = (int)note.endTimestamp / 60;
                int end_sec = (int)note.endTimestamp % 60;
                int end_cs = (int)((note.endTimestamp - (int)note.endTimestamp) * 100);
                ImGui::Text("%d:%02d.%02d - %d:%02d.%02d", start_min, start_sec, start_cs, end_min, end_sec, end_cs);
            } else {
                int minutes = (int)note.timestamp / 60;
                int seconds = (int)note.timestamp % 60;
                int centiseconds = (int)((note.timestamp - (int)note.timestamp) * 100);
                ImGui::Text("%d:%02d.%02d", minutes, seconds, centiseconds);
            }

            ImGui::TableSetColumnIndex(5);
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

            ImGui::Text("Note Type:");
            bool isTap = (selectedNote->type == Core::NoteType::TAP);
            bool isHold = (selectedNote->type == Core::NoteType::HOLD);

            if (ImGui::RadioButton("TAP", isTap)) {
                if (!isTap) {
                    // Convert from HOLD to TAP note
                    nodeManager.moveNote(selectedNote->id, selectedNote->lane, selectedNote->timestamp);
                    // Refresh the note pointer after modification
                    selectedNote = nodeManager.getNoteById(selectedNoteId);
                }
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("HOLD", isHold)) {
                if (!isHold) {
                    // Convert from TAP to HOLD note with default duration
                    double endTime = selectedNote->timestamp + gridSpacing;
                    endTime = std::clamp(endTime, selectedNote->timestamp, songDuration);
                    nodeManager.moveHoldNote(selectedNote->id, selectedNote->lane, selectedNote->timestamp, endTime);
                    // Refresh the note pointer after modification
                    selectedNote = nodeManager.getNoteById(selectedNoteId);
                }
            }

            // Refresh the selected note pointer in case it was modified
            selectedNote = nodeManager.getNoteById(selectedNoteId);
            if (!selectedNote) return;

            if (selectedNote->type == Core::NoteType::HOLD) {
                ImGui::Text("Start Time (seconds):");
                float startTimeValue = static_cast<float>(selectedNote->timestamp);
                if (ImGui::InputFloat("##startTime", &startTimeValue, 0.1f, 1.0f, "%.3f")) {
                    double newStartTime = std::clamp(static_cast<double>(startTimeValue), 0.0, selectedNote->endTimestamp);
                    if (snapToGrid) {
                        newStartTime = std::round(newStartTime / gridSpacing) * gridSpacing;
                    }
                    nodeManager.moveHoldNote(selectedNote->id, selectedNote->lane, newStartTime, selectedNote->endTimestamp);
                    // Refresh the note pointer after modification
                    selectedNote = nodeManager.getNoteById(selectedNoteId);
                }

                ImGui::Text("End Time (seconds):");
                float endTimeValue = static_cast<float>(selectedNote->endTimestamp);
                if (ImGui::InputFloat("##endTime", &endTimeValue, 0.1f, 1.0f, "%.3f")) {
                    double newEndTime = std::clamp(static_cast<double>(endTimeValue), selectedNote->timestamp, songDuration);
                    if (snapToGrid) {
                        newEndTime = std::round(newEndTime / gridSpacing) * gridSpacing;
                    }
                    nodeManager.moveHoldNote(selectedNote->id, selectedNote->lane, selectedNote->timestamp, newEndTime);
                    // Refresh the note pointer after modification
                    selectedNote = nodeManager.getNoteById(selectedNoteId);
                }

                ImGui::Text("Duration: %.3fs", selectedNote->endTimestamp - selectedNote->timestamp);

                // Duration slider for hold notes
                float duration = static_cast<float>(selectedNote->endTimestamp - selectedNote->timestamp);
                if (ImGui::SliderFloat("##duration", &duration, 0.1f, 10.0f, "%.3fs")) {
                    double newEndTime = selectedNote->timestamp + static_cast<double>(duration);
                    newEndTime = std::clamp(newEndTime, selectedNote->timestamp, songDuration);
                    if (snapToGrid) {
                        newEndTime = std::round(newEndTime / gridSpacing) * gridSpacing;
                    }
                    nodeManager.moveHoldNote(selectedNote->id, selectedNote->lane, selectedNote->timestamp, newEndTime);
                    // Refresh the note pointer after modification
                    selectedNote = nodeManager.getNoteById(selectedNoteId);
                }

                int start_min = (int)selectedNote->timestamp / 60;
                int start_sec = (int)selectedNote->timestamp % 60;
                int start_cs = (int)((selectedNote->timestamp - (int)selectedNote->timestamp) * 100);
                int end_min = (int)selectedNote->endTimestamp / 60;
                int end_sec = (int)selectedNote->endTimestamp % 60;
                int end_cs = (int)((selectedNote->endTimestamp - (int)selectedNote->endTimestamp) * 100);
                ImGui::Text("Formatted: %d:%02d.%02d - %d:%02d.%02d", start_min, start_sec, start_cs, end_min, end_sec, end_cs);

                if (ImGui::Button("Extend by 1 Beat")) {
                    double newEndTime = selectedNote->endTimestamp + gridSpacing;
                    newEndTime = std::clamp(newEndTime, selectedNote->timestamp, songDuration);
                    nodeManager.moveHoldNote(selectedNote->id, selectedNote->lane, selectedNote->timestamp, newEndTime);
                    // Refresh the note pointer after modification
                    selectedNote = nodeManager.getNoteById(selectedNoteId);
                }
            } else {
                ImGui::Text("Time (seconds):");
                float timeValue = static_cast<float>(selectedNote->timestamp);
                if (ImGui::InputFloat("##time", &timeValue, 0.1f, 1.0f, "%.3f")) {
                    double newTime = std::clamp(static_cast<double>(timeValue), 0.0, songDuration);
                    if (snapToGrid) {
                        newTime = std::round(newTime / gridSpacing) * gridSpacing;
                    }
                    nodeManager.moveNote(selectedNote->id, selectedNote->lane, newTime);
                    // Refresh the note pointer after modification
                    selectedNote = nodeManager.getNoteById(selectedNoteId);
                }

                int minutes = (int)selectedNote->timestamp / 60;
                int seconds = (int)selectedNote->timestamp % 60;
                int centiseconds = (int)((selectedNote->timestamp - (int)selectedNote->timestamp) * 100);
                ImGui::Text("Formatted: %d:%02d.%02d", minutes, seconds, centiseconds);

                if (selectedNote->timestamp > 0) {
                    float bpmAtNote = 60.0f / selectedNote->timestamp;
                    ImGui::Text("BPM at note: %.1f", bpmAtNote);
                }
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
    header.version = 2; // Updated version for TAP/HOLD support
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
        file.write(reinterpret_cast<const char*>(&note.type), sizeof(note.type));
        file.write(reinterpret_cast<const char*>(&note.timestamp), sizeof(note.timestamp));
        file.write(reinterpret_cast<const char*>(&note.endTimestamp), sizeof(note.endTimestamp));
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

    if (header.version < 1 || header.version > 2) {
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
        Core::NoteType type;
        double timestamp;
        double endTimestamp;

        file.read(reinterpret_cast<char*>(&id), sizeof(id));
        file.read(reinterpret_cast<char*>(&lane), sizeof(lane));

        if (header.version >= 2) {
            file.read(reinterpret_cast<char*>(&type), sizeof(type));
            file.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
            file.read(reinterpret_cast<char*>(&endTimestamp), sizeof(endTimestamp));
        } else {
            // Version 1 compatibility - all notes are TAP notes
            type = Core::NoteType::TAP;
            file.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
            endTimestamp = timestamp;
        }

        if (!file.good()) {
            std::cerr << "Failed to read note " << i << std::endl;
            break;
        }

        if (type == Core::NoteType::HOLD) {
            nodeManager.addHoldNoteWithId(id, static_cast<int>(lane), timestamp, endTimestamp);
        } else {
            nodeManager.addNoteWithId(id, static_cast<int>(lane), timestamp);
        }
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

void Editor::analyzeAudioFile(const std::string& filepath) {
    if (!audioAnalyzer) return;

    waveformLoaded = false;
    isAnalyzing = true;
    analysisProgress = "Starting analysis...";
    analysisProgressPercent = 0.0f;

    audioAnalyzer->setProgressCallback([this](const AnalysisProgress& progress) {
        this->onAnalysisProgress(progress);
    });

    std::thread analysisThread([this, filepath]() {
        try {
            waveformData = audioAnalyzer->analyzeAudio(filepath);
            waveformLoaded = true;
        } catch (const std::exception& e) {
            std::cerr << "Waveform analysis failed: " << e.what() << std::endl;
        }
        isAnalyzing = false;
    });

    analysisThread.detach();
}

void Editor::onAnalysisProgress(const AnalysisProgress& progress) {
    analysisProgress = progress.stage;
    analysisProgressPercent = static_cast<float>(progress.progress);
}

void Editor::drawWaveform() {
    if (!showWaveform || !waveformLoaded || waveformData.data.empty()) {
        if (showWaveform && isSongLoaded && !waveformLoaded && !isAnalyzing) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 content_pos = ImGui::GetCursorScreenPos();
            float timeline_y = content_pos.y + 30.0f;

            float waveformHeight = timelineHeight * 0.4f;
            float waveformY = timeline_y + (timelineHeight - waveformHeight) * 0.5f;

            draw_list->AddRectFilled(
                ImVec2(content_pos.x, waveformY),
                ImVec2(content_pos.x + timelineWidth, waveformY + waveformHeight),
                IM_COL32(40, 40, 40, 100)
            );

            const char* text = "\n\nWaveform not available";
            ImVec2 textSize = ImGui::CalcTextSize(text);
            ImVec2 textPos = ImVec2(
                content_pos.x + (timelineWidth - textSize.x) * 0.5f,
                waveformY + (waveformHeight - textSize.y) * 0.5f
            );

            draw_list->AddText(textPos, IM_COL32(255, 165, 0, 200), text);
        }
        return;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 content_pos = ImGui::GetCursorScreenPos();
    float timeline_y = content_pos.y + 30.0f;
    float visible_duration = songDuration / zoomLevel;
    float visible_start = scrollOffset;

    std::string resolutionLevel = "medium";
    if (zoomLevel >= 10.0f) {
        resolutionLevel = "high";
    } else if (zoomLevel <= 2.0f) {
        resolutionLevel = "low";
    }

    const std::vector<double>* waveform = &waveformData.data;
    if (waveformData.levels.find(resolutionLevel) != waveformData.levels.end()) {
        waveform = &waveformData.levels[resolutionLevel].peaks;
    }

    double timePerSample = waveformData.duration / waveform->size();
    size_t startIndex = static_cast<size_t>(visible_start / timePerSample);
    size_t endIndex = static_cast<size_t>((visible_start + visible_duration) / timePerSample);

    startIndex = std::min(startIndex, waveform->size() - 1);
    endIndex = std::min(endIndex, waveform->size());

    if (startIndex >= endIndex) return;

    float waveformHeight = timelineHeight * 0.4f;
    float waveformY = timeline_y + (timelineHeight - waveformHeight) * 0.5f;

    draw_list->AddRectFilled(
        ImVec2(content_pos.x, waveformY),
        ImVec2(content_pos.x + timelineWidth, waveformY + waveformHeight),
        IM_COL32(20, 20, 20, 80)
    );

    float stepX = timelineWidth / (endIndex - startIndex);

    for (size_t i = startIndex; i < endIndex - 1; ++i) {
        float x1 = content_pos.x + (i - startIndex) * stepX;
        float x2 = content_pos.x + (i + 1 - startIndex) * stepX;

        float amplitude1 = static_cast<float>((*waveform)[i]) * waveformHeight * 0.45f * WAVEFORM_HEIGHT_MULTIPLIER;
        float amplitude2 = static_cast<float>((*waveform)[i + 1]) * waveformHeight * 0.45f * WAVEFORM_HEIGHT_MULTIPLIER;

        float y1 = waveformY + waveformHeight * 0.5f - amplitude1;
        float y2 = waveformY + waveformHeight * 0.5f - amplitude2;

        ImU32 waveformColor;
        float intensity = (static_cast<float>((*waveform)[i]) + static_cast<float>((*waveform)[i + 1])) * 0.5f;
        if (intensity > 0.8f) {
            waveformColor = IM_COL32(255, 100, 100, 220); // Red for strong bass/rhythm
        } else if (intensity > 0.6f) {
            waveformColor = IM_COL32(255, 150, 100, 200); // Orange for medium intensity
        } else if (intensity > 0.4f) {
            waveformColor = IM_COL32(100, 200, 255, 180); // Blue for moderate intensity
        } else {
            waveformColor = IM_COL32(100, 150, 255, 150); // Light blue for low intensity
        }

        draw_list->AddLine(
            ImVec2(x1, y1),
            ImVec2(x2, y2),
            waveformColor,
            1.0f
        );

        draw_list->AddLine(
            ImVec2(x1, waveformY + waveformHeight * 0.5f + amplitude1),
            ImVec2(x2, waveformY + waveformHeight * 0.5f + amplitude2),
            waveformColor,
            1.0f
        );
    }

    draw_list->AddLine(
        ImVec2(content_pos.x, waveformY + waveformHeight * 0.5f),
        ImVec2(content_pos.x + timelineWidth, waveformY + waveformHeight * 0.5f),
        IM_COL32(255, 255, 255, 30),
        1.0f
    );
}

void Editor::drawHelpWindow() {
    if (!showHelpWindow) return;

    ImGui::Begin("Keyboard Shortcuts & Help", &showHelpWindow, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "PLAYBACK CONTROLS");
    ImGui::Separator();
    ImGui::Text("Space - Play/Pause");
    ImGui::Text("Enter - Jump to beginning");
    ImGui::Text("Left/Right Arrow - Seek backward/forward");
    ImGui::Text("S - Toggle speed override");
    ImGui::Text("+/- (when speed override enabled) - Adjust speed");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "NOTE PLACEMENT");
    ImGui::Separator();
    ImGui::Text("F - Place tap note on top lane");
    ImGui::Text("Shift+F - Place hold note on top lane");
    ImGui::Text("J - Place tap note on bottom lane");
    ImGui::Text("Shift+J - Place hold note on bottom lane");
    ImGui::Text("Delete - Delete selected note");
    ImGui::Text("Ctrl+C - Clear all notes");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "TIMELINE CONTROLS");
    ImGui::Separator();
    ImGui::Text("G - Toggle grid");
    ImGui::Text("A - Toggle auto-scroll");
    ImGui::Text("T - Toggle snap to grid");
    ImGui::Text("I - Toggle note IDs");
    ImGui::Text("M - Toggle milliseconds");
    ImGui::Text("W - Toggle waveform");
    ImGui::Text("Ctrl++/Ctrl+- - Zoom in/out");
    ImGui::Text("Ctrl+0 - Reset zoom");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "WINDOW TOGGLES");
    ImGui::Separator();
    ImGui::Text("Ctrl+H - Toggle this help window");
    ImGui::Text("Ctrl+N - Toggle notes list");
    ImGui::Text("Ctrl+P - Toggle properties panel");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "FILE OPERATIONS");
    ImGui::Separator();
    ImGui::Text("Ctrl+O - Open file browser");
    ImGui::Text("Ctrl+S - Save chart");
    ImGui::Text("Ctrl+L - Load chart");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "MOUSE CONTROLS");
    ImGui::Separator();
    ImGui::Text("Left click - Select note");
    ImGui::Text("Right click - Place note at cursor position");
    ImGui::Text("Mouse wheel - Zoom timeline");
    ImGui::Text("Drag - Pan timeline");
    ImGui::Text("Double click note - Jump to note position");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "TIPS");
    ImGui::Separator();
    ImGui::Text("- Use ctrl+esc to exit the editor");
    ImGui::Text("- Use snap to grid for precise note placement");
    ImGui::Text("- Hold Shift while placing notes to create hold notes");
    ImGui::Text("- Speed override affects playback speed without changing pitch");
    ImGui::Text("- Use auto-scroll to keep the cursor in view while playing");
    ImGui::Text("- Zoom controls help with detailed editing");

    ImGui::End();
}

} // Windows
} // App
