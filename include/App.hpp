#pragma once

#include "imgui.h"

#include "Editor.hpp"
#include "SoundManager.hpp"

namespace App
{
    class Config {
        private:
            ImGuiIO& io;

        public:
            Config();
            ~Config() = default;

            void configure();
    };

    void run();
}
