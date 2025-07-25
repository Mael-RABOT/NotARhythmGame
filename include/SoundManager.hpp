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

class SoundManager {
private:
    std::map<std::string, HSTREAM> streams;
    bool initialized;

#ifdef __EMSCRIPTEN__
    // WebAssembly stub implementations
    static const int BASS_DEFAULT_DEVICE = -1;
    static const int BASS_MAX_FREQUENCY = 44100;
    static const int BASS_MIN_FREQUENCY = 100;
    static const int BASS_SAMPLE_LOOP = 4;
    static const int BASS_ATTRIB_VOL = 2;
    static const int BASS_POS_BYTE = 0;
    static const int BASS_ACTIVE_PLAYING = 3;
    static const int BASS_ACTIVE_PAUSED = 2;
    static const int BASS_ATTRIB_FREQ = 1;
    static const int BASS_OK = 0;
    static const int BASS_ERROR_MEM = 1;
    static const int BASS_ERROR_FILEOPEN = 2;
    static const int BASS_ERROR_DRIVER = 3;
    static const int BASS_ERROR_BUFLOST = 4;
    static const int BASS_ERROR_HANDLE = 5;
    static const int BASS_ERROR_FORMAT = 6;
    static const int BASS_ERROR_POSITION = 7;
    static const int BASS_ERROR_INIT = 8;
    static const int BASS_ERROR_START = 9;
    static const int BASS_ERROR_ALREADY = 10;
    static const int BASS_ERROR_NOCHAN = 11;
    static const int BASS_ERROR_ILLTYPE = 12;
    static const int BASS_ERROR_ILLPARAM = 13;
    static const int BASS_ERROR_NO3D = 14;
    static const int BASS_ERROR_NOEAX = 15;
    static const int BASS_ERROR_DEVICE = 16;
    static const int BASS_ERROR_NOPLAY = 17;
    static const int BASS_ERROR_FREQ = 18;
    static const int BASS_ERROR_NOTFILE = 19;
    static const int BASS_ERROR_NOHW = 20;
    static const int BASS_ERROR_EMPTY = 21;
    static const int BASS_ERROR_NONET = 22;
    static const int BASS_ERROR_CREATE = 23;
    static const int BASS_ERROR_NOFX = 24;
    static const int BASS_ERROR_NOTAVAIL = 25;
    static const int BASS_ERROR_DECODE = 26;
    static const int BASS_ERROR_DX = 27;
    static const int BASS_ERROR_TIMEOUT = 28;
    static const int BASS_ERROR_FILEFORM = 29;
    static const int BASS_ERROR_SPEAKER = 30;
    static const int BASS_ERROR_VERSION = 31;
    static const int BASS_ERROR_CODEC = 32;
    static const int BASS_ERROR_ENDED = 33;
    static const int BASS_ERROR_BUSY = 34;
#endif

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
