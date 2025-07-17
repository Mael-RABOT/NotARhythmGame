#include "App.hpp"

#define MUSIC_PATH "/home/mrabot/Music/INTERWORLD.mp3"

namespace App
{
    // Config class
    Config::Config() : io(ImGui::GetIO()) {}

    void Config::configure() {
        this->io.ConfigDockingWithShift = true;
    }

    // Entrypoint
    void run() {
        static Config config;
        static Windows::Editor editor;

        // Configure ImGui once
        static bool configured = false;
        if (!configured) {
        config.configure();
            configured = true;
        }

        editor.update();
        editor.render();
    }
} // namespace App
