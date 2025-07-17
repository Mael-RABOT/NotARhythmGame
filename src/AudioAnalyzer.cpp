#include "AudioAnalazyer.hpp"
#include <limits>

AudioAnalyzer::AudioAnalyzer(size_t maxFileSize) : maxFileSize(maxFileSize) {}

void AudioAnalyzer::setProgressCallback(std::function<void(const AnalysisProgress&)> callback) {
    progressCallback = callback;
}

void AudioAnalyzer::updateProgress(double progress, const std::string& stage) {
    if (progressCallback) {
        progressCallback({progress, stage});
    }
}

std::vector<double> AudioAnalyzer::loadAudioFile(const std::string& filename, double& sampleRate, double& duration) {
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(sfInfo));

    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfInfo);
    if (!file) {
        throw std::runtime_error("Failed to open audio file: " + std::string(sf_strerror(nullptr)));
    }

    sampleRate = sfInfo.samplerate;
    duration = static_cast<double>(sfInfo.frames) / sampleRate;

    std::vector<double> audioData(sfInfo.frames * sfInfo.channels);
    sf_count_t samplesRead = sf_readf_double(file, audioData.data(), sfInfo.frames);

    if (samplesRead != sfInfo.frames) {
        sf_close(file);
        throw std::runtime_error("Failed to read complete audio file");
    }

    sf_close(file);

    // Convert to mono if stereo
    if (sfInfo.channels == 2) {
        std::vector<double> monoData(sfInfo.frames);
        for (int i = 0; i < sfInfo.frames; i++) {
            monoData[i] = (audioData[i * 2] + audioData[i * 2 + 1]) * 0.5;
        }
        return monoData;
    } else if (sfInfo.channels == 1) {
        return audioData;
    } else {
        throw std::runtime_error("Unsupported number of channels: " + std::to_string(sfInfo.channels));
    }
}

WaveformLevel AudioAnalyzer::generateWaveformLevel(const std::vector<double>& channelData, int samplesPerPixel) {
    std::vector<double> peaks;
    std::vector<double> rms;

    int segmentCount = static_cast<int>(std::ceil(static_cast<double>(channelData.size()) / samplesPerPixel));

    // Calculate global statistics
    double globalMax = 0.0;
    double globalSum = 0.0;
    for (double sample : channelData) {
        double absSample = std::abs(sample);
        globalMax = std::max(globalMax, absSample);
        globalSum += absSample;
    }

    double globalAverage = globalSum / channelData.size();
    double silenceThreshold = globalAverage * 0.15;

    for (int i = 0; i < segmentCount; i++) {
        int startSample = i * samplesPerPixel;
        int endSample = std::min(startSample + samplesPerPixel, static_cast<int>(channelData.size()));

        double maxPeak = -std::numeric_limits<double>::infinity();
        double minPeak = std::numeric_limits<double>::infinity();
        double sumSquares = 0.0;
        double bassSum = 0.0;
        int sampleCount = 0;
        int bassCount = 0;

        for (int j = startSample; j < endSample; j++) {
            double sample = channelData[j];
            maxPeak = std::max(maxPeak, sample);
            minPeak = std::min(minPeak, sample);
            sumSquares += sample * sample;
            sampleCount++;

            if (j > startSample) {
                double highFreq = sample - channelData[j - 1];
                double bassComponent = sample - highFreq * 0.3;
                bassSum += bassComponent * bassComponent;
                bassCount++;
            }
        }

        double rmsValue = std::sqrt(sumSquares / sampleCount);
        double bassEnergy = bassCount > 0 ? std::sqrt(bassSum / bassCount) : 0.0;
        double peakAmplitude = std::max(std::abs(maxPeak), std::abs(minPeak));
        double combinedAmplitude = std::max(peakAmplitude, bassEnergy * 1.5);

        double processedAmplitude;
        if (combinedAmplitude < silenceThreshold) {
            processedAmplitude = combinedAmplitude * 0.3;
        } else if (combinedAmplitude > globalAverage * 2) {
            processedAmplitude = combinedAmplitude * 1.2;
        } else {
            double normalized = (combinedAmplitude - silenceThreshold) / (globalAverage * 2 - silenceThreshold);
            processedAmplitude = combinedAmplitude * (0.5 + normalized * 0.7);
        }

        double compressionFactor = 0.8;
        double finalAmplitude = std::pow(processedAmplitude, compressionFactor);

        peaks.push_back(finalAmplitude);
        rms.push_back(rmsValue);
    }

    return {peaks, rms, samplesPerPixel};
}

std::vector<double> AudioAnalyzer::analyzeFrequencyContent(const std::vector<double>& channelData, double sampleRate) {
    const int windowSize = 2048;
    const int hopSize = windowSize / 4;
    std::vector<double> frequencyData;

    int maxWindows = std::min(1000, static_cast<int>((channelData.size() - windowSize) / hopSize));

    for (int i = 0; i < maxWindows; i++) {
        int startIndex = i * hopSize;

        double spectralSum = 0.0;
        double magnitudeSum = 0.0;

        // Simple DFT for spectral centroid calculation
        for (int k = 1; k < windowSize / 2; k++) {
            double real = 0.0;
            double imag = 0.0;

            for (int n = 0; n < windowSize; n++) {
                double sample = (startIndex + n < static_cast<int>(channelData.size())) ? channelData[startIndex + n] : 0.0;
                double angle = -2.0 * M_PI * k * n / windowSize;
                real += sample * std::cos(angle);
                imag += sample * std::sin(angle);
            }

            double magnitude = std::sqrt(real * real + imag * imag);
            double frequency = k * sampleRate / windowSize;

            spectralSum += frequency * magnitude;
            magnitudeSum += magnitude;
        }

        double spectralCentroid = magnitudeSum > 0 ? spectralSum / magnitudeSum : 0.0;
        frequencyData.push_back(spectralCentroid);

        if (i % 20 == 0) {
            double progress = 55.0 + (static_cast<double>(i) / maxWindows) * 10.0;
            updateProgress(progress, "Analyzing frequency content... (" + std::to_string(i) + "/" + std::to_string(maxWindows) + ")");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return frequencyData;
}

BeatFeatures AudioAnalyzer::analyzeBeatFeatures(const std::vector<double>& channelData, double sampleRate) {
    int windowSize = static_cast<int>(sampleRate * 0.1); // 100ms windows
    int hopSize = windowSize / 2;
    int maxWindows = std::min(2000, static_cast<int>((channelData.size() - windowSize) / hopSize));

    BeatFeatures beatFeatures;

    for (int i = 0; i < maxWindows; i++) {
        int startIndex = i * hopSize;
        double energy = 0.0;
        int zeroCrossings = 0;
        double bassEnergy = 0.0;

        for (int j = 0; j < windowSize; j++) {
            if (startIndex + j >= static_cast<int>(channelData.size())) break;

            double sample = channelData[startIndex + j];
            energy += sample * sample;

            // Count zero crossings
            if (j > 0 && startIndex + j - 1 < static_cast<int>(channelData.size())) {
                bool prevPositive = channelData[startIndex + j - 1] >= 0;
                bool currPositive = sample >= 0;
                if (prevPositive != currPositive) {
                    zeroCrossings++;
                }
            }

            // Simple bass energy calculation
            if (j > 0 && startIndex + j - 1 < static_cast<int>(channelData.size())) {
                double highFreq = sample - channelData[startIndex + j - 1];
                double bassComponent = sample - highFreq;
                bassEnergy += bassComponent * bassComponent;
            }
        }

        energy = energy / windowSize;
        bassEnergy = bassEnergy / windowSize;
        double rhythmIntensity = std::sqrt(energy * bassEnergy);

        beatFeatures.energy.push_back(energy);
        beatFeatures.zeroCrossings.push_back(static_cast<double>(zeroCrossings));
        beatFeatures.spectralCentroid.push_back(0.0); // Simplified
        beatFeatures.bassEnergy.push_back(bassEnergy);
        beatFeatures.rhythmIntensity.push_back(rhythmIntensity);

        if (i % 200 == 0) {
            double progress = 70.0 + (static_cast<double>(i) / maxWindows) * 10.0;
            updateProgress(progress, "Detecting beats and rhythm... (" + std::to_string(i) + "/" + std::to_string(maxWindows) + ")");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return beatFeatures;
}

AudioStats AudioAnalyzer::calculateAudioStats(const std::vector<double>& channelData, double sampleRate) {
    double maxAmplitude = 0.0;
    double sumAmplitude = 0.0;
    double sumSquares = 0.0;

    const size_t chunkSize = 100000;
    size_t totalChunks = (channelData.size() + chunkSize - 1) / chunkSize;

    for (size_t chunk = 0; chunk < totalChunks; chunk++) {
        size_t startIndex = chunk * chunkSize;
        size_t endIndex = std::min(startIndex + chunkSize, channelData.size());

        for (size_t i = startIndex; i < endIndex; i++) {
            double amplitude = std::abs(channelData[i]);
            maxAmplitude = std::max(maxAmplitude, amplitude);
            sumAmplitude += amplitude;
            sumSquares += channelData[i] * channelData[i];
        }

        if (chunk % 5 == 0) {
            double progress = 85.0 + (static_cast<double>(chunk) / totalChunks) * 5.0;
            updateProgress(progress, "Calculating audio statistics... (" + std::to_string(chunk) + "/" + std::to_string(totalChunks) + ")");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    double averageAmplitude = sumAmplitude / channelData.size();
    double rmsAmplitude = std::sqrt(sumSquares / channelData.size());
    double silenceThreshold = averageAmplitude * 0.1;

    // Detect loud sections
    std::vector<LoudSection> loudSections;
    double sectionDuration = 0.5; // 500ms sections
    int samplesPerSection = static_cast<int>(sampleRate * sectionDuration);
    double energyThreshold = rmsAmplitude * 2.0;

    for (size_t i = 0; i < channelData.size(); i += samplesPerSection) {
        double sectionEnergy = 0.0;
        size_t sectionEnd = std::min(i + samplesPerSection, channelData.size());

        for (size_t j = i; j < sectionEnd; j++) {
            sectionEnergy += channelData[j] * channelData[j];
        }

        double averageEnergy = std::sqrt(sectionEnergy / (sectionEnd - i));

        if (averageEnergy > energyThreshold) {
            loudSections.push_back({
                static_cast<double>(i) / sampleRate,
                static_cast<double>(sectionEnd) / sampleRate,
                averageEnergy
            });
        }
    }

    return {
        maxAmplitude,
        averageAmplitude,
        maxAmplitude - silenceThreshold,
        silenceThreshold,
        loudSections
    };
}

AudioWaveform AudioAnalyzer::analyzeAudio(const std::string& filename) {
    try {
        updateProgress(0, "Checking file size...");

        // Check file size
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        size_t fileSize = file.tellg();
        file.close();

        if (fileSize > maxFileSize) {
            throw std::runtime_error("File too large (" + std::to_string(fileSize / (1024 * 1024)) +
                                   "MB). Maximum allowed: " + std::to_string(maxFileSize / (1024 * 1024)) + "MB");
        }

        updateProgress(10, "Loading audio file...");

        double sampleRate, duration;
        std::vector<double> channelData = loadAudioFile(filename, sampleRate, duration);
        int totalSamples = static_cast<int>(channelData.size());

        updateProgress(35, "Generating waveform levels...");

        // Create multiple resolution levels
        std::vector<std::pair<std::string, int>> resolutions = {
            {"overview", std::max(1, totalSamples / 1000)},
            {"low", std::max(1, totalSamples / 5000)},
            {"medium", std::max(1, totalSamples / 20000)},
            {"high", std::max(1, totalSamples / 100000)}
        };

        std::map<std::string, WaveformLevel> waveformLevels;

        for (size_t i = 0; i < resolutions.size(); i++) {
            double progress = 35.0 + (static_cast<double>(i) / resolutions.size()) * 20.0;
            updateProgress(progress, "Generating " + resolutions[i].first + " waveform level...");

            waveformLevels[resolutions[i].first] = generateWaveformLevel(channelData, resolutions[i].second);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        updateProgress(55, "Analyzing frequency content...");
        std::vector<double> frequencyData = analyzeFrequencyContent(channelData, sampleRate);

        updateProgress(70, "Detecting beats and rhythm...");
        BeatFeatures beatFeatures = analyzeBeatFeatures(channelData, sampleRate);

        updateProgress(85, "Calculating audio statistics...");
        AudioStats audioStats = calculateAudioStats(channelData, sampleRate);

        updateProgress(95, "Finalizing analysis...");

        AudioWaveform waveformData = {
            waveformLevels["medium"].peaks, // Default to medium resolution peaks
            waveformLevels,
            frequencyData,
            beatFeatures,
            audioStats,
            sampleRate,
            duration,
            totalSamples,
            sampleRate
        };

        updateProgress(100, "Analysis complete!");
        return waveformData;

    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to analyze audio file: " + std::string(e.what()));
    }
}
