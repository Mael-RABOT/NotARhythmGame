#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>

#ifdef __EMSCRIPTEN__
// WebAssembly version - no BASS library
typedef int HSTREAM;
typedef unsigned int DWORD;
typedef unsigned long long QWORD;
#else
#include "bass.h"
#endif

// Define BASS constants for both WebAssembly and normal compilation
#ifndef BASS_DEFAULT_DEVICE
#define BASS_DEFAULT_DEVICE -1
#endif
#ifndef BASS_MAX_FREQUENCY
#define BASS_MAX_FREQUENCY 44100
#endif
#ifndef BASS_MIN_FREQUENCY
#define BASS_MIN_FREQUENCY 100
#endif

class SoundManager {
private:
    std::map<std::string, HSTREAM> streams;
    bool initialized;

public:
    SoundManager();
    ~SoundManager();

    bool initialize(int device = BASS_DEFAULT_DEVICE, int freq = BASS_MAX_FREQUENCY, int flags = BASS_MIN_FREQUENCY);
    void cleanup();

    bool loadSound(const std::string& name, const std::string& filepath);
    bool playSound(const std::string& name, bool loop = false);
    bool stopSound(const std::string& name);
    bool pauseSound(const std::string& name);
    bool resumeSound(const std::string& name);
    bool setVolume(const std::string& name, float volume);
    bool isPlaying(const std::string& name);
    double getCurrentTime(const std::string& name);
    double getDuration(const std::string& name);
    bool setPosition(const std::string& name, double time);
    bool setFrequency(const std::string& name, int frequency);
    int getFrequency(const std::string& name);
    void stopAllSounds();
    void pauseAllSounds();
    void resumeAllSounds();
    void setGlobalVolume(float volume);
    std::string getLastError() const;

    // Additional methods needed by Editor
    double getPosition(const std::string& name);
    bool seekTo(const std::string& name, double position);
    bool isSoundLoaded(const std::string& name);
    float getPlaybackSpeed(const std::string& name);
    float getCurrentPlaybackSpeed(const std::string& name);
    bool setPlaybackSpeed(const std::string& name, float speed);
    void unloadSound(const std::string& name);
    void unloadAllSounds();

private:
    std::string lastError;
    float globalVolume;
};
