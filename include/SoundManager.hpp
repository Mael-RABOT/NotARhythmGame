#pragma once

#include "bass.h"
#include <string>
#include <unordered_map>
#include <iostream>

namespace App {
namespace Core {

class SoundManager {
private:
    std::unordered_map<std::string, HSTREAM> loadedSounds;
    bool initialized;

public:
    SoundManager();
    ~SoundManager();

    bool initialize(int device = -1, DWORD freq = 44100, DWORD flags = 0);

    void cleanup();

    bool loadSound(const std::string& name, const std::string& filepath);
    bool playSound(const std::string& name, bool loop = false);
    bool stopSound(const std::string& name);
    bool pauseSound(const std::string& name);
    bool resumeSound(const std::string& name);

    bool setVolume(const std::string& name, float volume);

    bool isPlaying(const std::string& name);

    // Audio position and duration methods
    double getPosition(const std::string& name);
    bool setPosition(const std::string& name, double position);
    double getDuration(const std::string& name);
    bool seekTo(const std::string& name, double position);

    void unloadSound(const std::string& name);
    void unloadAllSounds();

    std::string getLastError();
};

} // Core
} // App
