#pragma once

#include "KindOfADelay.h"

#include <juce_audio_processors/juce_audio_processors.h>
namespace ID
{
#define PARAMETER_ID(str) constexpr const char* str{#str};
PARAMETER_ID(percentageCPU)
PARAMETER_ID(bpm)
PARAMETER_ID(feedback)
PARAMETER_ID(crossfeedback)
PARAMETER_ID(beatIndexLeft)
PARAMETER_ID(beatIndexRight)
PARAMETER_ID(cutoff)
PARAMETER_ID(diffuse)
PARAMETER_ID(mix)
PARAMETER_ID(modulationDepth)
PARAMETER_ID(modulationSpeed)

#undef PARAMETER_ID
}

class CpuTime
{
  public:
    void setSampleRate(const float sampleRate)
    {
        m_sampleRate = sampleRate;
    }
    void start()
    {
        m_beginTime = std::chrono::high_resolution_clock::now();
    }
    typedef std::function<void(int)> MarkingFunction;

    void laps(size_t numSamples, MarkingFunction markingCallback)
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - m_beginTime);
        elapsedTotalNanoSeconds += static_cast<size_t>(elapsed.count());
        samplesProcessed += static_cast<size_t>(numSamples);
        const float secondsPoll = 0.5f; // every x seconds show cpu load
        if (static_cast<float>(samplesProcessed) > m_sampleRate * secondsPoll)
        {
            auto pRate = static_cast<float>(100.0 * static_cast<double>(elapsedTotalNanoSeconds) /
                                            (secondsPoll * 1'000'000'000.0));
            avgCpu += static_cast<size_t>(pRate * 100.f);
            avgCpu -= m_avgCpu[head];
            m_avgCpu[head++] = static_cast<size_t>(pRate * 100.f);
            head = head % m_avgCpu.size();
            markingCallback(static_cast<int>(avgCpu / m_avgCpu.size() / 10));
            elapsedTotalNanoSeconds = 0;
            samplesProcessed = 0;
        }
    }

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_beginTime;
    std::array<size_t, 8> m_avgCpu{300, 300, 300, 300, 300, 300, 300, 300};
    size_t head = 0;
    size_t avgCpu = 300 * 8;

    size_t elapsedTotalNanoSeconds{0};
    size_t samplesProcessed = 0;
    float m_sampleRate{48000.f};
};

class AudioPluginAudioProcessor : public juce::AudioProcessor
{
  public:
    juce::StringArray beatStrings = juce::StringArray{"1/64 triplet",
                                                      "1/64",
                                                      "1/32 triplet",
                                                      "1/32",
                                                      "1/16 triplet",
                                                      "1/32 dot",
                                                      "1/16 quintuplet",
                                                      "1/16",
                                                      "5/16",
                                                      "1/8 triplet",
                                                      "1/16 dot",
                                                      "1/8 quintuplet",
                                                      "1/16 doubledot",
                                                      "1/8",
                                                      "9/16",
                                                      "5/8",
                                                      "1/4 triplet",
                                                      "11/16",
                                                      "1/8 dot",
                                                      "1/4 quintuplet",
                                                      "13/16",
                                                      "1/8 doubledot",
                                                      "15/16",
                                                      "1/4",
                                                      "17/16",
                                                      "9/8",
                                                      "19/16",
                                                      "5/4",
                                                      "21/16",
                                                      "1/2 triplet",
                                                      "11/8",
                                                      "23/16",
                                                      "1/4 dot",
                                                      "25/16",
                                                      "1/2 quintuplet",
                                                      "13/8",
                                                      "27/16",
                                                      "1/4 doubledot",
                                                      "29/16",
                                                      "15/8",
                                                      "31/16",
                                                      "1/2",
                                                      "1 triplet",
                                                      "1/2 dot",
                                                      "1 quintuplet",
                                                      "1/2 doubledot",
                                                      "1",
                                                      "2 triplet",
                                                      "1 dot",
                                                      "2 quintuplet",
                                                      "1 doubledot",
                                                      "2",
                                                      "4 triplet",
                                                      "2 dot",
                                                      "4 quintuplet",
                                                      "2 doubledot",
                                                      "4",
                                                      "4 dot",
                                                      "4 doubledot"};
    AudioPluginAudioProcessor()
        : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                             .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                             .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
          )
    {
        addParameter(mix = new juce::AudioParameterInt(juce::ParameterID(ID::mix, 1), "Mix", 0, 100, 20,
                                                       juce::AudioParameterIntAttributes().withLabel("").withCategory(
                                                           juce::AudioProcessorParameter::Category::genericParameter)));
        addParameter(bpm = new juce::AudioParameterFloat(
                         juce::ParameterID(ID::bpm, 1), "BPM", juce::NormalisableRange<float>(40.0f, 250.f), 120.f,
                         juce::AudioParameterFloatAttributes().withLabel("BPM").withCategory(
                             juce::AudioProcessorParameter::Category::genericParameter)));


        addParameter(beatIndexLeft = new juce::AudioParameterChoice(juce::ParameterID(ID::beatIndexLeft, 1), "Topos",
                                                                    beatStrings, 23));

        addParameter(beatIndexRight = new juce::AudioParameterChoice(juce::ParameterID(ID::beatIndexRight, 1), "Topos",
                                                                     beatStrings, 23));
        addParameter(feedback =
                         new juce::AudioParameterInt(juce::ParameterID(ID::feedback, 1), "Feedback", 0, 100, 30,
                                                     juce::AudioParameterIntAttributes().withLabel("%").withCategory(
                                                         juce::AudioProcessorParameter::Category::genericParameter)));
        addParameter(crossFeedback = new juce::AudioParameterInt(
                         juce::ParameterID(ID::crossfeedback, 1), "Cross Feedback", 0, 100, 10,
                         juce::AudioParameterIntAttributes().withLabel("%").withCategory(
                             juce::AudioProcessorParameter::Category::genericParameter)));
        addParameter(cutoff =
                         new juce::AudioParameterInt(juce::ParameterID(ID::cutoff, 1), "Cutoff", 100, 20000, 8000,
                                                     juce::AudioParameterIntAttributes().withLabel("hz").withCategory(
                                                         juce::AudioProcessorParameter::Category::genericParameter)));
        addParameter(diffuse =
                         new juce::AudioParameterInt(juce::ParameterID(ID::feedback, 1), "Diffuse", 0, 100, 10,
                                                     juce::AudioParameterIntAttributes().withLabel("").withCategory(
                                                         juce::AudioProcessorParameter::Category::genericParameter)));
        addParameter(modulationDepth = new juce::AudioParameterInt(
                         juce::ParameterID(ID::modulationDepth, 1), "Modulation Depth", 0, 100, 20,
                         juce::AudioParameterIntAttributes().withLabel("").withCategory(
                             juce::AudioProcessorParameter::Category::genericParameter)));
        addParameter(modulationSpeed = new juce::AudioParameterFloat(
                         juce::ParameterID(ID::modulationSpeed, 1), "Modulation Speed",
                         juce::NormalisableRange<float>(0.01f, 20.f), 0.3f,
                         juce::AudioParameterFloatAttributes().withLabel("Hz").withCategory(
                             juce::AudioProcessorParameter::Category::genericParameter)));
        addParameter(percentageCPU = new juce::AudioParameterInt(
                         juce::ParameterID(ID::percentageCPU, 1), "Thread Usage", 0, 1000, 0,
                         juce::AudioParameterIntAttributes()
                             .withLabel("per 1000")
                             .withCategory(juce::AudioProcessorParameter::Category::inputMeter)));
    }
    ~AudioPluginAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        m_sampleRate = static_cast<size_t>(sampleRate);
        m_cpuTime.setSampleRate(m_sampleRate);
        juce::ignoreUnused(sampleRate, samplesPerBlock);
        pluginRunner = std::make_unique<KindOfADelay<10000>>(sampleRate);
    }

    void releaseResources() override
    {
        auto ptr = pluginRunner.release();
        delete ptr;
    }

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
#if JucePlugin_IsMidiEffect
        juce::ignoreUnused(layouts);
        return true;
#else
        // This is the place where you check if the layout is supported.
        // In this template code we only support mono or stereo.
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
            layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

            // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;
#endif

        return true;
#endif
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        juce::ignoreUnused(midiMessages);
        juce::ScopedNoDenormals noDenormals;
        m_cpuTime.start();
        if ((getTotalNumInputChannels() == 2) && (getTotalNumOutputChannels() == 2))
        {
            if (*mix != prev_mix)
            {
                prev_mix = *mix;
                pluginRunner->setMix(static_cast<float>(*mix) / 100.f);
            }
            if (*bpm != prev_bpm)
            {
                prev_bpm = *bpm;
                pluginRunner->setBpm(static_cast<float>(*bpm));
            }
            if (*beatIndexLeft != prev_beatIndexLeft)
            {
                prev_beatIndexLeft = *beatIndexLeft;
                pluginRunner->setRhythmLeft(static_cast<size_t>(*beatIndexLeft));
            }
            if (*beatIndexRight != prev_beatIndexRight)
            {
                prev_beatIndexRight = *beatIndexRight;
                pluginRunner->setRhythmRight(static_cast<size_t>(*beatIndexRight));
            }
            if (*feedback != prev_feedback)
            {
                prev_feedback = *feedback;
                pluginRunner->setFeedback(static_cast<float>(*feedback) / 100.f);
            }
            if (*crossFeedback != prev_crossFeedback)
            {
                prev_crossFeedback = *crossFeedback;
                pluginRunner->setCrossFeedback(static_cast<float>(*crossFeedback) / 100.f);
            }
            if (*cutoff != prev_cutoff)
            {
                prev_cutoff = *cutoff;
                pluginRunner->setFilterCutoff(static_cast<float>(*cutoff));
            }
            if (*diffuse != prev_diffuse)
            {
                prev_diffuse = *diffuse;
                pluginRunner->setDiffuse(static_cast<float>(*diffuse) / 100.f);
            }
            if (*modulationDepth != prev_modulationDepth)
            {
                prev_modulationDepth = *modulationDepth;
                pluginRunner->setModulationDepth(static_cast<float>(*modulationDepth));
            }
            if (*modulationSpeed != prev_modulationSpeed)
            {
                prev_modulationSpeed = *modulationSpeed;
                pluginRunner->setModulationSpeed(static_cast<float>(*modulationSpeed));
            }
            pluginRunner->processBlock(buffer.getReadPointer(0), buffer.getReadPointer(1), buffer.getWritePointer(0),
                                       buffer.getWritePointer(1), static_cast<size_t>(buffer.getNumSamples()));
        }
        m_cpuTime.laps(static_cast<size_t>(buffer.getNumSamples()), [this](int value) { *percentageCPU = value; });
    }


    size_t m_sampleRate{48000};

    juce::AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override

    {
        return false; // (change this to false if you choose to not supply an editor)
    }

    const juce::String getName() const override
    {
        return JucePlugin_Name;
    }

    bool acceptsMidi() const override
    {
#if JucePlugin_WantsMidiInput
        return true;
#else
        return false;
#endif
    }

    bool producesMidi() const override
    {
#if JucePlugin_ProducesMidiOutput
        return true;
#else
        return false;
#endif
    }

    bool isMidiEffect() const override
    {
#if JucePlugin_IsMidiEffect
        return true;
#else
        return false;
#endif
    }

    double getTailLengthSeconds() const override
    {
        return 2.0;
    }

    void setCurrentProgram(int /*index*/) override {}

    int getCurrentProgram() override
    {
        return 0;
    }

    int getNumPrograms() override
    {
        return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
                  // so this should be at least 1, even if you're not really implementing programs.
    }

    const juce::String getProgramName(int index) override
    {
        switch (index)
        {
            case 0:
                return {"Program 0"};
            default:
                return {"Program unknown"};
        }
    }

    void changeProgramName(int index, const juce::String& newName) override
    {
        juce::ignoreUnused(index, newName);
    }

    void getStateInformation(juce::MemoryBlock& destData) override
    {
        // You should use this method to store your parameters in the memory block.
        // You could do that either as raw data, or use the XML or ValueTree classes
        // as intermediaries to make it easy to save and load complex data.
        juce::ignoreUnused(destData);
    }
    void setStateInformation(const void* data, int sizeInBytes) override
    {
        // You should use this method to restore your parameters from this memory block,
        // whose contents will have been created by the getStateInformation() call.
        juce::ignoreUnused(data, sizeInBytes);
    }


  private:
    juce::AudioParameterInt* mix;
    int prev_mix;
    juce::AudioParameterInt* feedback;
    int prev_feedback;
    juce::AudioParameterInt* crossFeedback;
    int prev_crossFeedback;
    juce::AudioParameterChoice* beatIndexLeft;
    int prev_beatIndexLeft;
    juce::AudioParameterChoice* beatIndexRight;
    int prev_beatIndexRight;
    juce::AudioParameterInt* cutoff;
    int prev_cutoff;
    juce::AudioParameterInt* diffuse;
    int prev_diffuse;
    juce::AudioParameterInt* modulationDepth;
    int prev_modulationDepth;
    juce::AudioParameterFloat* bpm;
    float prev_bpm;
    juce::AudioParameterFloat* modulationSpeed;
    float prev_modulationSpeed;
    juce::AudioParameterInt* percentageCPU;
    std::unique_ptr<KindOfADelay<10000>> pluginRunner;
    CpuTime m_cpuTime{};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
