#include "AudioAnalazyer.hpp"
#include <limits>

AudioAnalyzer::AudioAnalyzer(size_t maxFileSize) : maxFileSize(maxFileSize) {}

void AudioAnalyzer::setProgressCallback(std::function<void(const AnalysisProgress&)> callback) {
    progressCallback = callback;
}

void AudioAnalyzer::updateProgress(double progress, const std::string& stage) {
    if (progressCallback) {
        try {
            progressCallback({progress, stage});
        } catch (const std::exception& e) {
            std::cerr << "Progress callback error: " << e.what() << std::endl;
        }
    }
}

std::vector<double> AudioAnalyzer::loadAudioFile(const std::string& filename, double& sampleRate, double& duration) {
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(sfInfo));

    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfInfo);
    if (!file) {
        throw std::runtime_error("Failed to open audio file: " + std::string(sf_strerror(nullptr)));
    }

    if (sfInfo.frames <= 0 || sfInfo.samplerate <= 0 || sfInfo.channels <= 0) {
        sf_close(file);
        throw std::runtime_error("Invalid audio file parameters");
    }

    size_t expectedSize = static_cast<size_t>(sfInfo.frames) * sfInfo.channels;
    size_t memoryRequired = expectedSize * sizeof(double);
    if (memoryRequired > maxFileSize) {
        sf_close(file);
        throw std::runtime_error("Audio file too large for analysis (requires " +
                                std::to_string(memoryRequired / (1024 * 1024)) +
                                "MB, max allowed: " + std::to_string(maxFileSize / (1024 * 1024)) + "MB)");
    }

    sampleRate = sfInfo.samplerate;
    duration = static_cast<double>(sfInfo.frames) / sampleRate;

    std::vector<double> audioData;
    try {
        audioData.resize(sfInfo.frames * sfInfo.channels);
    } catch (const std::bad_alloc& e) {
        sf_close(file);
        throw std::runtime_error("Failed to allocate memory for audio data: " + std::string(e.what()));
    }

    const sf_count_t chunkSize = 1000000;
    sf_count_t totalSamplesRead = 0;

    for (sf_count_t offset = 0; offset < sfInfo.frames; offset += chunkSize) {
        sf_count_t samplesToRead = std::min(chunkSize, sfInfo.frames - offset);
        sf_count_t samplesRead = sf_readf_double(file,
            audioData.data() + (offset * sfInfo.channels),
            samplesToRead);

        if (samplesRead != samplesToRead) {
            sf_close(file);
            throw std::runtime_error("Failed to read complete audio file");
        }

        totalSamplesRead += samplesRead;
    }

    if (totalSamplesRead != sfInfo.frames) {
        sf_close(file);
        throw std::runtime_error("Failed to read complete audio file");
    }

    sf_close(file);

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

    double globalMax = 0.0;
    double globalSum = 0.0;
    double globalBassSum = 0.0;
    double globalRhythmSum = 0.0;

    for (size_t i = 0; i < channelData.size(); i++) {
        double sample = channelData[i];
        double absSample = std::abs(sample);
        globalMax = std::max(globalMax, absSample);
        globalSum += absSample;

        if (i > 0) {
            double bassComponent = sample * 0.8 + channelData[i-1] * 0.2;
            globalBassSum += bassComponent * bassComponent;

            double onset = std::abs(sample - channelData[i-1]);
            globalRhythmSum += onset;
        }
    }

    double globalAverage = globalSum / channelData.size();
    double globalBassAverage = std::sqrt(globalBassSum / (channelData.size() - 1));
    double globalRhythmAverage = globalRhythmSum / (channelData.size() - 1);

    double silenceThreshold = globalAverage * 0.1;
    double bassThreshold = globalBassAverage * 0.3;
    double rhythmThreshold = globalRhythmAverage * 0.5;

    for (int i = 0; i < segmentCount; i++) {
        int startSample = i * samplesPerPixel;
        int endSample = std::min(startSample + samplesPerPixel, static_cast<int>(channelData.size()));

        if (startSample >= static_cast<int>(channelData.size()) || startSample >= endSample) {
            peaks.push_back(0.0);
            rms.push_back(0.0);
            continue;
        }

        double maxPeak = -std::numeric_limits<double>::infinity();
        double minPeak = std::numeric_limits<double>::infinity();
        double sumSquares = 0.0;
        double bassEnergy = 0.0;
        double rhythmEnergy = 0.0;
        double onsetEnergy = 0.0;
        int sampleCount = 0;
        int bassCount = 0;
        int rhythmCount = 0;

        for (int j = startSample; j < endSample && j < static_cast<int>(channelData.size()); j++) {
            double sample = channelData[j];
            maxPeak = std::max(maxPeak, sample);
            minPeak = std::min(minPeak, sample);
            sumSquares += sample * sample;
            sampleCount++;

            if (j > startSample) {
                double highFreq = sample - channelData[j - 1];
                double midFreq = sample - highFreq * 0.5;
                double bassComponent = midFreq - highFreq * 0.3;

                bassEnergy += bassComponent * bassComponent * 2.0;
                bassCount++;

                double onset = std::abs(sample - channelData[j - 1]);
                onsetEnergy += onset * onset;

                bool prevPositive = channelData[j - 1] >= 0;
                bool currPositive = sample >= 0;
                if (prevPositive != currPositive) {
                    rhythmEnergy += onset * 1.5;
                }
                rhythmCount++;
            }
        }

        double rmsValue = std::sqrt(sumSquares / sampleCount);
        double avgBassEnergy = bassCount > 0 ? std::sqrt(bassEnergy / bassCount) : 0.0;
        double avgRhythmEnergy = rhythmCount > 0 ? std::sqrt(rhythmEnergy / rhythmCount) : 0.0;
        double avgOnsetEnergy = rhythmCount > 0 ? std::sqrt(onsetEnergy / rhythmCount) : 0.0;

        double peakAmplitude = std::max(std::abs(maxPeak), std::abs(minPeak));

        double volumeComponent = peakAmplitude * 0.3;
        double bassComponent = avgBassEnergy * 2.5;
        double rhythmComponent = avgRhythmEnergy * 2.0;
        double onsetComponent = avgOnsetEnergy * 1.8;

        double combinedAmplitude = volumeComponent + bassComponent + rhythmComponent + onsetComponent;

        double processedAmplitude;
        if (combinedAmplitude < silenceThreshold) {
            processedAmplitude = combinedAmplitude * 0.2;
        } else if (bassComponent > bassThreshold * 2.0) {
            processedAmplitude = combinedAmplitude * 1.8;
        } else if (rhythmComponent > rhythmThreshold * 1.5) {
            processedAmplitude = combinedAmplitude * 1.5;
        } else if (combinedAmplitude > globalAverage * 3) {
            processedAmplitude = combinedAmplitude * 1.2;
        } else {
            double normalized = (combinedAmplitude - silenceThreshold) / (globalAverage * 3 - silenceThreshold);
            processedAmplitude = combinedAmplitude * (0.6 + normalized * 0.8);
        }

        double compressionFactor = 0.6;
        double finalAmplitude = std::pow(processedAmplitude, compressionFactor);

        peaks.push_back(finalAmplitude);
        rms.push_back(rmsValue);
    }

    if (!peaks.empty()) {
        double maxPeak = *std::max_element(peaks.begin(), peaks.end());
        double minPeak = *std::min_element(peaks.begin(), peaks.end());
        double range = maxPeak - minPeak;

        if (range > 0.0) {
            for (double& peak : peaks) {
                peak = (peak - minPeak) / range;
            }
        } else {
            for (double& peak : peaks) {
                peak = 0.5;
            }
        }
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

        if (startIndex + windowSize > static_cast<int>(channelData.size())) {
            break;
        }

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
            updateProgress(progress, "Calculating audio statistics... (" + std::to_string(chunk) + "/" + std::to_string(totalChunks) + " chunks)");
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

        updateProgress(5, "File size OK (" + std::to_string(fileSize / (1024 * 1024)) + "MB)");
        updateProgress(10, "Loading audio file...");

        double sampleRate, duration;
        std::vector<double> channelData = loadAudioFile(filename, sampleRate, duration);
        int totalSamples = static_cast<int>(channelData.size());

        updateProgress(30, "Audio loaded (" + std::to_string(totalSamples / 1000) + "k samples, " +
                      std::to_string(static_cast<int>(duration)) + "s duration)");
        updateProgress(35, "Generating waveform levels...");

    int maxSamplesPerPixel = std::min(100000, totalSamples / 1000);
    std::vector<std::pair<std::string, int>> resolutions = {
        {"overview", std::max(1, totalSamples / 1000)},
        {"low", std::max(1, std::min(totalSamples / 5000, maxSamplesPerPixel / 5))},
        {"medium", std::max(1, std::min(totalSamples / 20000, maxSamplesPerPixel / 20))},
        {"high", std::max(1, std::min(totalSamples / 100000, maxSamplesPerPixel / 100))}
    };

        std::map<std::string, WaveformLevel> waveformLevels;

        for (size_t i = 0; i < resolutions.size(); i++) {
            double progress = 35.0 + (static_cast<double>(i) / resolutions.size()) * 20.0;
            updateProgress(progress, "Generating " + resolutions[i].first + " waveform level (" + std::to_string(i + 1) + "/" + std::to_string(resolutions.size()) + ")...");

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

        updateProgress(100, "Analysis complete! (" + std::to_string(waveformData.data.size()) + " waveform points)");
        return waveformData;

    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to analyze audio file: " + std::string(e.what()));
    }
}

void AudioAnalyzer::cacheAudioForSpectrum(const std::string& filename) {
    if (cachedAudioFile == filename && !cachedAudioData.empty()) {
        return;
    }

    try {
        cachedAudioData = loadAudioFile(filename, cachedSampleRate, cachedDuration);
        cachedAudioFile = filename;
    } catch (const std::exception& e) {
        std::cerr << "Error caching audio for spectrum: " << e.what() << std::endl;
        clearAudioCache();
    }
}

void AudioAnalyzer::clearAudioCache() {
    cachedAudioFile.clear();
    cachedAudioData.clear();
    cachedSampleRate = 0.0;
    cachedDuration = 0.0;
}

std::vector<float> AudioAnalyzer::getSpectrumAtTime(const std::string& filename, double time, int spectrumSize) {
    std::vector<float> spectrum(spectrumSize, 0.0f);

    try {
        if (cachedAudioFile != filename || cachedAudioData.empty()) {
            cacheAudioForSpectrum(filename);
        }

        if (cachedAudioData.empty()) {
            return spectrum;
        }

        size_t sampleIndex = static_cast<size_t>(time * cachedSampleRate);
        if (sampleIndex >= cachedAudioData.size()) {
            return spectrum;
        }

        int windowSize = 1024;
        size_t startIndex = sampleIndex;
        size_t endIndex = std::min(startIndex + windowSize, cachedAudioData.size());

        std::vector<double> window;
        for (size_t i = startIndex; i < endIndex; i++) {
            window.push_back(cachedAudioData[i]);
        }

        while (window.size() < static_cast<size_t>(windowSize)) {
            window.push_back(0.0);
        }

        for (int i = 0; i < windowSize; i++) {
            double windowValue = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (windowSize - 1)));
            window[i] *= windowValue;
        }

        std::vector<float> magnitudes(spectrumSize);

        for (int bin = 0; bin < spectrumSize; bin++) {
            double real = 0.0, imag = 0.0;

            for (int n = 0; n < windowSize; n++) {
                double angle = -2.0 * M_PI * bin * n / windowSize;
                real += window[n] * std::cos(angle);
                imag += window[n] * std::sin(angle);
            }

            double magnitude = std::sqrt(real * real + imag * imag);
            magnitudes[bin] = static_cast<float>(magnitude);
        }

        float maxMagnitude = 0.0f;
        for (float mag : magnitudes) {
            maxMagnitude = std::max(maxMagnitude, mag);
        }

        if (maxMagnitude > 0.0f) {
            for (int i = 0; i < spectrumSize; i++) {
                float frequencyWeight = 1.0f;
                float normalizedFreq = static_cast<float>(i) / spectrumSize;

                if (normalizedFreq < 0.1f) {
                    frequencyWeight = 2.0f;
                } else if (normalizedFreq < 0.3f) {
                    frequencyWeight = 1.5f;
                } else if (normalizedFreq > 0.7f) {
                    frequencyWeight = 0.5f;
                }

                spectrum[i] = (magnitudes[i] / maxMagnitude) * frequencyWeight;
                spectrum[i] = std::min(1.0f, spectrum[i]);
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error getting spectrum: " << e.what() << std::endl;
    }

    return spectrum;
}
