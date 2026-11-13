#pragma once
#include "PluginProcessor.h"

class PA3609Editor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PA3609Editor (PA3609Processor&);
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { repaint(); }
    PA3609Processor& proc;
    juce::OwnedArray<juce::Component> comps;
    juce::OwnedArray<juce::Label> labels;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sAtt;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cAtt;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> bAtt;
    float dispGr[2] { 0, 0 }, dispIn = -60, dispOut = -60;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PA3609Editor)
};
