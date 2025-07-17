#pragma once

#include "imgui.h"

#include "Timeline.hpp"
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

            ImGuiIO& getIO();
    };

    void run();
}
