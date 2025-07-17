#pragma once

#include <array>
#include <tuple>
#include <cstdint>

namespace App {
namespace Windows {

    struct SeekPerZoom {
        static constexpr std::array<std::tuple<float, float>, 4> data = {
            std::tuple<float, float>{1.0f, 5.0f},
            std::tuple<float, float>{5.0f, 2.5f},
            std::tuple<float, float>{10.0f, 1.0f},
            std::tuple<float, float>{15.0f, 0.250f},
        };
    };

    struct ChartHeader {
        char magic[12];        // "NOTARHYTHM" (11 chars + null terminator)
        uint32_t version;      // File format version
        uint32_t headerSize;   // Size of this header
        uint32_t audioSize;    // Size of embedded audio data
        uint32_t notesCount;   // Number of notes
        char title[256];       // Song title
        char artist[256];      // Artist name
        float bpm;             // Beats per minute
        double duration;       // Song duration in seconds
        uint32_t reserved[16]; // Reserved for future use
    };

} // Windows
} // App