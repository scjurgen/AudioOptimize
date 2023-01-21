
#include "AudioFile.h"

#include <array>
#include <chrono>

int main(int ac, char* av[])
{
    if (ac > 3)
    {
        AudioFile<float> audioFile;

        /* in ports: 1 */
        if (audioFile.load(av[1]))
        {
            LV2_URID_Map map{};
            LV2_Log_Logger logger{};
            map.map = bogusMap;
            LV2_Feature supportedFeatures[3] = {
                {LV2_LOG__log, (void*) &logger}, {LV2_URID__map, (void*) &map}, {nullptr, nullptr}};
            ;

            const LV2_Descriptor* d{nullptr};
            std::array<LV2_Feature*, 3> feature{&supportedFeatures[0], &supportedFeatures[1], &supportedFeatures[2]};

            LV2_Handle instance = instantiate(d, 48000.0, "/", feature.data());
            connectPorts(instance);

            if (ac == 4)
            {
                modifyParameters(av[3]);
            }

            constexpr size_t seconds = 30;
            // settle parameters for some seconds
            constexpr size_t numSamples = 48000 * seconds;
            constexpr size_t numBlocks = numSamples / blockSize;
            auto start = std::chrono::steady_clock::now();
            for (size_t j = 0; j < numBlocks; ++j)
            {
                run(instance, blockSize);
            }
            auto stop = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

            std::cout << "Time taken by bounce: " << duration.count() / 1000.0 << " ms -> ";
            float secondsNeeded = duration.count() / 1'000'000.f;
            std::cout << "load of " << secondsNeeded * 100.f / seconds << " % per thread" << std::endl;
            std::array<std::vector<float>, 2> result; // max stereo?
            for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i += blockSize)
            {
                for (size_t j = 0; j < blockSize; ++j)
                {
                    /* Assign Audio In */
                    audioIn[j] = audioFile.samples[0][i + j];
                }
                run(instance, blockSize);
                for (size_t j = 0; j < blockSize; ++j)
                {
                    /* Assign Audio Out */
                    result[0].push_back(audioLeft[j]);
                    result[1].push_back(audioRight[j]);
                }
            }

            AudioFile<float>::AudioBuffer audioBufferOut;
            for (auto& b : result)
            {
                audioBufferOut.push_back(b);
            }
            AudioFile<float> audioFileSave;

            audioFileSave.setAudioBuffer(audioBufferOut);
            audioFileSave.setSampleRate(48000);
            if (!audioFileSave.save(av[2]))
            {
                std::cerr << "error saving:" << av[2] << std::endl;
            }
        }
    }
    else
    {
        printf("Usage: %s <inWaveFile> <outWaveFile> <parameterfile>\n", av[0]);
    }
}
