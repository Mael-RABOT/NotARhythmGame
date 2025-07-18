#pragma once

#include "imgui.h"

#include "Editor.hpp"
#include "Player.hpp"

namespace App
{
    enum class AppMode {
        MAIN_MENU = 0,
        EDITOR = 1,
        PLAYER = 2
    };

    class Config {
        private:
            ImGuiIO& io;

        public:
            Config();
            ~Config() = default;

            void configure();
    };

    extern bool shouldShutdown;
    void requestShutdown();
    bool isShutdownRequested();

    void run();
    void showMainMenu(AppMode& currentMode);
}
