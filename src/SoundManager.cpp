#include "SoundManager.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>
using namespace emscripten;
#endif

SoundManager::SoundManager() : initialized(false), globalVolume(1.0f) {
}

SoundManager::~SoundManager() {
    cleanup();
}

bool SoundManager::initialize(int device, int freq, int flags) {
    if (initialized) {
        return true;
    }

#ifdef __EMSCRIPTEN__
    // For WebAssembly, we'll use Web Audio API
    // This is a stub implementation - in a real scenario, you'd need to
    // implement Web Audio API integration
    initialized = true;
    return true;
#else
    if (!BASS_Init(device, freq, flags, 0, NULL)) {
        std::cerr << "Failed to initialize BASS: " << getLastError() << std::endl;
        return false;
    }
    initialized = true;
    return true;
#endif
}

void SoundManager::cleanup() {
    if (initialized) {
        stopAllSounds();
        streams.clear();
#ifdef __EMSCRIPTEN__
        // Web Audio API cleanup would go here
#else
        BASS_Free();
#endif
        initialized = false;
    }
}

bool SoundManager::loadSound(const std::string& name, const std::string& filepath) {
    if (!initialized) {
        std::cerr << "SoundManager not initialized!" << std::endl;
        return false;
    }

    if (streams.find(name) != streams.end()) {
        // Sound already loaded
        return true;
    }

#ifdef __EMSCRIPTEN__
    // For WebAssembly, we'll use a simple ID system
    // In a real implementation, you'd load the audio file using Web Audio API
    static int nextId = 1;
    streams[name] = nextId++;
    return true;
#else
    HSTREAM stream = BASS_StreamCreateFile(FALSE, filepath.c_str(), 0, 0, 0);
    if (!stream) {
        std::cerr << "Failed to load sound '" << name << "' from '" << filepath << "': " << getLastError() << std::endl;
        return false;
    }
    streams[name] = stream;
    return true;
#endif
}

bool SoundManager::playSound(const std::string& name, bool loop) {
    if (!initialized) {
        std::cerr << "SoundManager not initialized!" << std::endl;
        return false;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        std::cerr << "Sound '" << name << "' not found!" << std::endl;
        return false;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Playing sound: " << name << (loop ? " (looped)" : "") << std::endl;
    return true;
#else
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
#endif
}

bool SoundManager::stopSound(const std::string& name) {
    if (!initialized) {
        return false;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return false;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Stopping sound: " << name << std::endl;
    return true;
#else
    return BASS_ChannelStop(it->second);
#endif
}

bool SoundManager::pauseSound(const std::string& name) {
    if (!initialized) {
        return false;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return false;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Pausing sound: " << name << std::endl;
    return true;
#else
    return BASS_ChannelPause(it->second);
#endif
}

bool SoundManager::resumeSound(const std::string& name) {
    if (!initialized) {
        return false;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return false;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Resuming sound: " << name << std::endl;
    return true;
#else
    return BASS_ChannelPlay(it->second, FALSE);
#endif
}

bool SoundManager::setVolume(const std::string& name, float volume) {
    if (!initialized) {
        return false;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return false;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Setting volume for " << name << " to " << volume << std::endl;
    return true;
#else
    return BASS_ChannelSetAttribute(it->second, BASS_ATTRIB_VOL, volume);
#endif
}

bool SoundManager::isPlaying(const std::string& name) {
    if (!initialized) {
        return false;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return false;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    return false; // Stub implementation
#else
    DWORD state = BASS_ChannelIsActive(it->second);
    return (state == BASS_ACTIVE_PLAYING);
#endif
}

double SoundManager::getCurrentTime(const std::string& name) {
    if (!initialized) {
        return 0.0;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return 0.0;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    return 0.0; // Stub implementation
#else
    QWORD position = BASS_ChannelGetPosition(it->second, BASS_POS_BYTE);
    if (static_cast<int>(position) == -1) {
        return 0.0;
    }
    double time = BASS_ChannelBytes2Seconds(it->second, position);
    return time;
#endif
}

double SoundManager::getDuration(const std::string& name) {
    if (!initialized) {
        return 0.0;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return 0.0;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    return 0.0; // Stub implementation
#else
    QWORD length = BASS_ChannelGetLength(it->second, BASS_POS_BYTE);
    if (static_cast<int>(length) == -1) {
        return 0.0;
    }
    double duration = BASS_ChannelBytes2Seconds(it->second, length);
    return duration;
#endif
}

bool SoundManager::setPosition(const std::string& name, double time) {
    if (!initialized) {
        return false;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return false;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Setting position for " << name << " to " << time << std::endl;
    return true;
#else
    QWORD bytes = BASS_ChannelSeconds2Bytes(it->second, time);
    return BASS_ChannelSetPosition(it->second, bytes, BASS_POS_BYTE);
#endif
}

bool SoundManager::setFrequency(const std::string& name, int frequency) {
    if (!initialized) {
        return false;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return false;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Setting frequency for " << name << " to " << frequency << std::endl;
    return true;
#else
    DWORD state = BASS_ChannelIsActive(it->second);
    bool wasPlaying = (state == BASS_ACTIVE_PLAYING);
    bool wasPaused = (state == BASS_ACTIVE_PAUSED);

    if (wasPlaying || wasPaused) {
        QWORD currentPos = BASS_ChannelGetPosition(it->second, BASS_POS_BYTE);
        bool success = BASS_ChannelSetAttribute(it->second, BASS_ATTRIB_FREQ, frequency);
        if (success) {
            BASS_ChannelSetPosition(it->second, currentPos, BASS_POS_BYTE);
            if (wasPlaying) {
                BASS_ChannelPlay(it->second, FALSE);
            } else if (wasPaused) {
                BASS_ChannelPause(it->second);
            }
        }
        return success;
    } else {
        return BASS_ChannelSetAttribute(it->second, BASS_ATTRIB_FREQ, frequency);
    }
#endif
}

int SoundManager::getFrequency(const std::string& name) {
    if (!initialized) {
        return 0;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return 0;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    return 44100; // Default frequency
#else
    float frequency;
    if (!BASS_ChannelGetAttribute(it->second, BASS_ATTRIB_FREQ, &frequency)) {
        return 0;
    }
    return static_cast<int>(frequency);
#endif
}

void SoundManager::stopAllSounds() {
    if (!initialized) {
        return;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Stopping all sounds" << std::endl;
#else
    for (auto& pair : streams) {
        BASS_ChannelStop(pair.second);
    }
#endif
}

void SoundManager::pauseAllSounds() {
    if (!initialized) {
        return;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Pausing all sounds" << std::endl;
#else
    for (auto& pair : streams) {
        BASS_ChannelPause(pair.second);
    }
#endif
}

void SoundManager::resumeAllSounds() {
    if (!initialized) {
        return;
    }

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Resuming all sounds" << std::endl;
#else
    for (auto& pair : streams) {
        DWORD state = BASS_ChannelIsActive(pair.second);
        if (state == BASS_ACTIVE_PAUSED) {
            BASS_ChannelPlay(pair.second, FALSE);
        }
    }
#endif
}

void SoundManager::setGlobalVolume(float volume) {
    globalVolume = volume;
    // Apply to all sounds
    for (auto& pair : streams) {
        setVolume(pair.first, volume);
    }
}

std::string SoundManager::getLastError() const {
#ifdef __EMSCRIPTEN__
    return "WebAssembly stub implementation - no errors available";
#else
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
#endif
}

// Additional methods needed by Editor
double SoundManager::getPosition(const std::string& name) {
    return getCurrentTime(name);
}

bool SoundManager::seekTo(const std::string& name, double position) {
    return setPosition(name, position);
}

bool SoundManager::isSoundLoaded(const std::string& name) {
    return streams.find(name) != streams.end();
}

float SoundManager::getPlaybackSpeed(const std::string& name) {
#ifdef __EMSCRIPTEN__
    return 1.0f; // Stub implementation
#else
    if (!initialized) {
        return 1.0f;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return 1.0f;
    }

    float frequency = 44100.0f;
    if (!BASS_ChannelGetAttribute(it->second, BASS_ATTRIB_FREQ, &frequency)) {
        return 1.0f;
    }

    return frequency / 44100.0f;
#endif
}

float SoundManager::getCurrentPlaybackSpeed(const std::string& name) {
    return getPlaybackSpeed(name);
}

bool SoundManager::setPlaybackSpeed(const std::string& name, float speed) {
    if (!initialized) {
        return false;
    }

    auto it = streams.find(name);
    if (it == streams.end()) {
        return false;
    }

    speed = std::max(0.1f, std::min(4.0f, speed));

#ifdef __EMSCRIPTEN__
    // Web Audio API implementation would go here
    std::cout << "Setting playback speed for " << name << " to " << speed << std::endl;
    return true;
#else
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
    }

    return success;
#endif
}

void SoundManager::unloadSound(const std::string& name) {
    auto it = streams.find(name);
    if (it != streams.end()) {
#ifdef __EMSCRIPTEN__
        // Web Audio API cleanup would go here
        std::cout << "Unloading sound: " << name << std::endl;
#else
        BASS_StreamFree(it->second);
#endif
        streams.erase(it);
    }
}

void SoundManager::unloadAllSounds() {
#ifdef __EMSCRIPTEN__
    // Web Audio API cleanup would go here
    std::cout << "Unloading all sounds" << std::endl;
#else
    for (auto& pair : streams) {
        BASS_StreamFree(pair.second);
    }
#endif
    streams.clear();
}
