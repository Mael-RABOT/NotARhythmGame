#include "SoundManager.hpp"
#include "bass.h"

namespace App {
namespace Core {

SoundManager::SoundManager() : initialized(false) {
}

SoundManager::~SoundManager() {
    cleanup();
}

bool SoundManager::initialize(int device, DWORD freq, DWORD flags) {
    if (initialized) {
        return true;
    }

    if (!BASS_Init(device, freq, flags, 0, NULL)) {
        std::cerr << "Failed to initialize BASS: " << getLastError() << std::endl;
        return false;
    }

    initialized = true;
    return true;
}

void SoundManager::cleanup() {
    if (initialized) {
        unloadAllSounds();
        BASS_Free();
        initialized = false;
    }
}

bool SoundManager::loadSound(const std::string& name, const std::string& filepath) {
    if (!initialized) {
        std::cerr << "SoundManager not initialized!" << std::endl;
        return false;
    }

    if (loadedSounds.find(name) != loadedSounds.end()) {
        unloadSound(name);
    }

    HSTREAM stream = BASS_StreamCreateFile(FALSE, filepath.c_str(), 0, 0, 0);
    if (!stream) {
        std::cerr << "Failed to load sound '" << name << "' from '" << filepath << "': " << getLastError() << std::endl;
        return false;
    }

    QWORD length = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
    if (static_cast<int>(length) != -1) {
        double duration = BASS_ChannelBytes2Seconds(stream, length);
        originalDurations[name] = duration;
    } else {
        originalDurations[name] = 0.0;
    }

    originalSpeeds[name] = 1.0f;

    loadedSounds[name] = stream;
    return true;
}

bool SoundManager::isSoundLoaded(const std::string& name) {
    return loadedSounds.find(name) != loadedSounds.end();
}

bool SoundManager::playSound(const std::string& name, bool loop) {
    if (!initialized) {
        std::cerr << "SoundManager not initialized!" << std::endl;
        return false;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        std::cerr << "Sound '" << name << "' not found!" << std::endl;
        return false;
    }

    HSTREAM stream = it->second;

    if (loop) {
        BASS_ChannelFlags(stream, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
    } else {
        BASS_ChannelFlags(stream, 0, BASS_SAMPLE_LOOP);
    }

    if (!BASS_ChannelPlay(stream, TRUE)) {
        std::cerr << "Failed to play sound '" << name << "': " << getLastError() << std::endl;
        return false;
    }

    return true;
}

bool SoundManager::stopSound(const std::string& name) {
    if (!initialized) {
        return false;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        return false;
    }

    return BASS_ChannelStop(it->second);
}

bool SoundManager::pauseSound(const std::string& name) {
    if (!initialized) {
        return false;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        return false;
    }

    return BASS_ChannelPause(it->second);
}

bool SoundManager::resumeSound(const std::string& name) {
    if (!initialized) {
        return false;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        return false;
    }

    return BASS_ChannelPlay(it->second, FALSE);
}

bool SoundManager::setVolume(const std::string& name, float volume) {
    if (!initialized) {
        return false;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        return false;
    }

    volume = std::max(0.0f, std::min(1.0f, volume));

    return BASS_ChannelSetAttribute(it->second, BASS_ATTRIB_VOL, volume);
}

bool SoundManager::isPlaying(const std::string& name) {
    if (!initialized) {
        return false;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        return false;
    }

    DWORD state = BASS_ChannelIsActive(it->second);
    return (state == BASS_ACTIVE_PLAYING);
}

double SoundManager::getPosition(const std::string& name) {
    if (!initialized) {
        return 0.0;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        return 0.0;
    }

    QWORD position = BASS_ChannelGetPosition(it->second, BASS_POS_BYTE);
    if (static_cast<int>(position) == -1) {
        return 0.0;
    }

    // Convert bytes to seconds
    double time = BASS_ChannelBytes2Seconds(it->second, position);
    return time;
}

bool SoundManager::setPosition(const std::string& name, double position) {
    if (!initialized) {
        return false;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        return false;
    }

    // Convert seconds to bytes
    QWORD bytes = BASS_ChannelSeconds2Bytes(it->second, position);
    if (static_cast<int>(bytes) == -1) {
        return false;
    }

    return BASS_ChannelSetPosition(it->second, bytes, BASS_POS_BYTE);
}

double SoundManager::getDuration(const std::string& name) {
    if (!initialized) {
        return 0.0;
    }

    auto it = originalDurations.find(name);
    if (it == originalDurations.end()) {
        return 0.0;
    }

    return it->second;
}

bool SoundManager::seekTo(const std::string& name, double position) {
    if (!initialized) {
        return false;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        return false;
    }

    // Get current playback state
    DWORD state = BASS_ChannelIsActive(it->second);
    bool wasPlaying = (state == BASS_ACTIVE_PLAYING);
    bool wasPaused = (state == BASS_ACTIVE_PAUSED);

    // Set the new position
    if (!setPosition(name, position)) {
        return false;
    }

    // Restore playback state
    if (wasPlaying) {
        BASS_ChannelPlay(it->second, FALSE);
    } else if (wasPaused) {
        BASS_ChannelPause(it->second);
    }

    return true;
}

void SoundManager::unloadSound(const std::string& name) {
    auto it = loadedSounds.find(name);
    if (it != loadedSounds.end()) {
        BASS_StreamFree(it->second);
        loadedSounds.erase(it);
    }

    originalDurations.erase(name);
    originalSpeeds.erase(name);
}

void SoundManager::unloadAllSounds() {
    for (auto& pair : loadedSounds) {
        BASS_StreamFree(pair.second);
    }
    loadedSounds.clear();
    originalDurations.clear();
    originalSpeeds.clear();
}

std::string SoundManager::getLastError() {
    int error = BASS_ErrorGetCode();
    switch (error) {
        case BASS_OK: return "No error";
        case BASS_ERROR_MEM: return "Memory error";
        case BASS_ERROR_FILEOPEN: return "Can't open the file";
        case BASS_ERROR_DRIVER: return "Can't find a free/valid driver";
        case BASS_ERROR_BUFLOST: return "The sample buffer was lost";
        case BASS_ERROR_HANDLE: return "Invalid handle";
        case BASS_ERROR_FORMAT: return "Unsupported sample format";
        case BASS_ERROR_POSITION: return "Invalid position";
        case BASS_ERROR_INIT: return "BASS_Init has not been successfully called";
        case BASS_ERROR_START: return "BASS_Start has not been successfully called";
        case BASS_ERROR_ALREADY: return "Already initialized/paused/whatever";
        case BASS_ERROR_NOCHAN: return "Can't get a free channel";
        case BASS_ERROR_ILLTYPE: return "An illegal type was specified";
        case BASS_ERROR_ILLPARAM: return "An illegal parameter was specified";
        case BASS_ERROR_NO3D: return "No 3D support";
        case BASS_ERROR_NOEAX: return "No EAX support";
        case BASS_ERROR_DEVICE: return "Illegal device number";
        case BASS_ERROR_NOPLAY: return "Not playing";
        case BASS_ERROR_FREQ: return "Illegal sample rate";
        case BASS_ERROR_NOTFILE: return "The stream is not a file stream";
        case BASS_ERROR_NOHW: return "No hardware voices available";
        case BASS_ERROR_EMPTY: return "The MOD music has no sequence data";
        case BASS_ERROR_NONET: return "No internet connection could be opened";
        case BASS_ERROR_CREATE: return "Couldn't create the file";
        case BASS_ERROR_NOFX: return "Effects are not available";
        case BASS_ERROR_NOTAVAIL: return "Requested data is not available";
        case BASS_ERROR_DECODE: return "The channel is a 'decoding channel'";
        case BASS_ERROR_DX: return "A sufficient DirectX version is not installed";
        case BASS_ERROR_TIMEOUT: return "Connection timedout";
        case BASS_ERROR_FILEFORM: return "Unsupported file format";
        case BASS_ERROR_SPEAKER: return "Unavailable speaker";
        case BASS_ERROR_VERSION: return "Invalid BASS version (used by add-ons)";
        case BASS_ERROR_CODEC: return "Codec is not available/supported";
        case BASS_ERROR_ENDED: return "The channel/file has ended";
        case BASS_ERROR_BUSY: return "The device is busy";
        default: return "Unknown error";
    }
}

float SoundManager::getPlaybackSpeed(const std::string& name) {
    if (!initialized) {
        return 1.0f;
    }

    auto it = originalSpeeds.find(name);
    if (it == originalSpeeds.end()) {
        return 1.0f;
    }

    return it->second;
}

float SoundManager::getCurrentPlaybackSpeed(const std::string& name) {
    if (!initialized) {
        return 1.0f;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        return 1.0f;
    }

    float frequency = 44100.0f;
    if (!BASS_ChannelGetAttribute(it->second, BASS_ATTRIB_FREQ, &frequency)) {
        return 1.0f;
    }

    return frequency / 44100.0f;
}

bool SoundManager::setPlaybackSpeed(const std::string& name, float speed) {
    if (!initialized) {
        return false;
    }

    auto it = loadedSounds.find(name);
    if (it == loadedSounds.end()) {
        return false;
    }

    speed = std::max(0.1f, std::min(4.0f, speed));

    DWORD state = BASS_ChannelIsActive(it->second);
    bool wasPlaying = (state == BASS_ACTIVE_PLAYING);
    bool wasPaused = (state == BASS_ACTIVE_PAUSED);

    QWORD currentPos = BASS_ChannelGetPosition(it->second, BASS_POS_BYTE);

    float frequency = 44100.0f * speed;

    bool success = BASS_ChannelSetAttribute(it->second, BASS_ATTRIB_FREQ, frequency);

    if (success) {
        BASS_ChannelSetPosition(it->second, currentPos, BASS_POS_BYTE);

        if (wasPlaying) {
            BASS_ChannelPlay(it->second, FALSE);
        } else if (wasPaused) {
            BASS_ChannelPause(it->second);
        }

        originalSpeeds[name] = speed;
    }

    return success;
}

} // Core
} // App
