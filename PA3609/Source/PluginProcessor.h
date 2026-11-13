#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class PA3609Processor : public juce::AudioProcessor
{
public:
    PA3609Processor();
    void prepareToPlay (double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& l) const override
    { return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
          && l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Paragon PA-3609"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> grMeter[2] { 0.f, 0.f }, inPk { 0.f }, outPk { 0.f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    struct St { float l = 0, c = 0, o1 = 0, o2 = 0, lp = 0, sh = 0; } st[2];
    float satd = 1.f;
    float sat (float x, float tot) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PA3609Processor)
};
