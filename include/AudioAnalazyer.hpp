#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <map>
#include <functional>
#include <memory>
#include <thread>
#include <chrono>
#include <cstring>
#include <sndfile.h>

struct LoudSection {
    double start;
    double end;
    double intensity;
};

struct AudioStats {
    double peakAmplitude;
    double averageAmplitude;
    double dynamicRange;
    double silenceThreshold;
    std::vector<LoudSection> loudSections;
};

struct BeatFeatures {
    std::vector<double> energy;
    std::vector<double> zeroCrossings;
    std::vector<double> spectralCentroid;
    std::vector<double> bassEnergy;
    std::vector<double> rhythmIntensity;
};

struct WaveformLevel {
    std::vector<double> peaks;
    std::vector<double> rms;
    int samplesPerPixel;
};

struct AnalysisProgress {
    double progress;
    std::string stage;
};

struct AudioWaveform {
    std::vector<double> data;
    std::map<std::string, WaveformLevel> levels;
    std::vector<double> frequencyData;
    BeatFeatures beatFeatures;
    AudioStats audioStats;
    double sampleRate;
    double duration;
    int totalSamples;
    double originalSampleRate;
};

/**
 * AudioAnalyzer - Optimized for rhythm game chart creation
 *
 * The waveform generation is specifically tuned for rhythm games, emphasizing:
 * - Bass frequencies (below ~200Hz) with 2.5x weight
 * - Rhythmic patterns and onset detection with 2.0x weight
 * - Onset energy (sudden changes) with 1.8x weight
 * - Volume/amplitude with reduced 0.3x weight
 *
 * This ensures that bass drops, drum hits, and rhythmic elements are prominently
 * displayed in the waveform, making it easier to create accurate charts.
 */
class AudioAnalyzer {
private:
    std::function<void(const AnalysisProgress&)> progressCallback;
    size_t maxFileSize;

    void updateProgress(double progress, const std::string& stage);
    std::vector<double> loadAudioFile(const std::string& filename, double& sampleRate, double& duration);
    WaveformLevel generateWaveformLevel(const std::vector<double>& channelData, int samplesPerPixel);
    std::vector<double> analyzeFrequencyContent(const std::vector<double>& channelData, double sampleRate);
    BeatFeatures analyzeBeatFeatures(const std::vector<double>& channelData, double sampleRate);
    AudioStats calculateAudioStats(const std::vector<double>& channelData, double sampleRate);

public:
    AudioAnalyzer(size_t maxFileSize = 50 * 1024 * 1024);
    void setProgressCallback(std::function<void(const AnalysisProgress&)> callback);
    AudioWaveform analyzeAudio(const std::string& filename);
};

