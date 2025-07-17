#include "App.hpp"

#define MUSIC_PATH "/home/mrabot/Music/INTERWORLD.mp3"

namespace App
{
    // Config class
    Config::Config() : io(ImGui::GetIO()) {}

    void Config::configure() {
        this->io.ConfigDockingWithShift = true;
    }

    ImGuiIO& Config::getIO() {
        return io;
    }

    // Play sound
    // void playSound(std::string filePath) {
    //     static Core::SoundManager sound_manager;

    //     static bool initialized = false;
    //     if (!initialized) {
    //         if (!sound_manager.initialize(-1, 44100, 0)) {
    //             std::cerr << "Failed to initialize sound manager" << std::endl;
    //             return;
    //         }

    //         if (sound_manager.loadSound("bgm", filePath)) {
    //             sound_manager.setVolume("bgm", 0.5f);
    //             sound_manager.playSound("bgm", true);
    //         }

    //         initialized = true;
    //     }
    // }

    // Entrypoint
    void run() {
        static Config config;
        static Windows::Timeline timeline;

        // Configure ImGui once
        static bool configured = false;
        if (!configured) {
        config.configure();
            configured = true;
        }

        // Update and render timeline
        timeline.update();
        timeline.render();
    }
} // namespace App
