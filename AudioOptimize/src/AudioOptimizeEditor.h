#pragma once

#include "AudioOptimizeProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
  public:
    //==============================================================================
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
        : AudioProcessorEditor(&p)
        , processorRef(p)
    {
        juce::ignoreUnused(processorRef);
        // Make sure that before the constructor has finished, you've set the
        // editor's size to whatever you need it to be.
        setSize(400, 300);
    }

    ~AudioPluginAudioProcessorEditor() override = default;

    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        g.setColour(juce::Colours::white);
        g.setFont(15.0f);
        g.drawFittedText("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    }

    void resized() override
    {
        // This is generally where you'll want to lay out the positions of any
        // subcomponents in your editor..
    }


  private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
