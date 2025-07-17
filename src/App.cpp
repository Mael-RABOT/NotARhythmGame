#include "App.hpp"

namespace App
{
    Config::Config() : io(ImGui::GetIO()) {}

    void Config::configure() {
        this->io.ConfigDockingWithShift = true;
    }

    void showMainMenu(AppMode& currentMode) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 200, ImGui::GetIO().DisplaySize.y * 0.5f - 150));
        ImGui::SetNextWindowSize(ImVec2(400, 300));

        ImGui::Begin("Not A Rhythm Game - Main Menu", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.15f, 0.95f));

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "NOT A RHYTHM GAME");
        ImGui::Separator();

        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::Button("EDITOR", ImVec2(300, 50))) {
            currentMode = AppMode::EDITOR;
        }

        ImGui::Spacing();

        if (ImGui::Button("PLAYER", ImVec2(300, 50))) {
            currentMode = AppMode::PLAYER;
        }

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Editor: Create and edit rhythm charts");
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Player: Listen to charts");

        ImGui::PopStyleColor();
        ImGui::End();
    }

    void run() {
        static Config config;
        static std::unique_ptr<Core::SoundManager> soundManager = std::make_unique<Core::SoundManager>();
        static Windows::Editor editor(soundManager.get());
        static Windows::Player player(soundManager.get());
        static AppMode currentMode = AppMode::MAIN_MENU;

        static bool configured = false;
        if (!configured) {
            config.configure();

            if (!soundManager->initialize(BASS_DEFAULT_DEVICE, BASS_MAX_FREQUENCY, BASS_MIN_FREQUENCY)) {
                std::cerr << "Failed to initialize sound manager" << std::endl;
            }

            configured = true;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape) && currentMode == AppMode::EDITOR) {
            currentMode = AppMode::MAIN_MENU;
        }

        switch (currentMode) {
            case AppMode::MAIN_MENU:
                showMainMenu(currentMode);
                break;

            case AppMode::EDITOR:
                ImGui::DockSpaceOverViewport();
                editor.update();
                editor.render();
                break;

            case AppMode::PLAYER:
                ImGui::DockSpaceOverViewport();
                player.update();
                player.render();
                break;
        }
    }
} // namespace App
