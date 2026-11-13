#include "PluginEditor.h"
using namespace juce;

PA3609Editor::PA3609Editor (PA3609Processor& p) : AudioProcessorEditor (&p), proc (p)
{
    for (auto* par : p.getParameters())
    {
        auto* pid = dynamic_cast<AudioProcessorParameterWithID*> (par);
        if (pid == nullptr) continue;
        auto* lab = labels.add (new Label ({}, pid->getName (24)));
        lab->setJustificationType (Justification::centred);
        lab->setFont (FontOptions (10.5f, Font::bold));
        addAndMakeVisible (lab);

        if (dynamic_cast<AudioParameterBool*> (par))
        {
            auto* b = new ToggleButton();
            comps.add (b);
            bAtt.add (new AudioProcessorValueTreeState::ButtonAttachment (p.apvts, pid->paramID, *b));
        }
        else if (auto* ch = dynamic_cast<AudioParameterChoice*> (par))
        {
            auto* c = new ComboBox();
            c->addItemList (ch->choices, 1);
            comps.add (c);
            cAtt.add (new AudioProcessorValueTreeState::ComboBoxAttachment (p.apvts, pid->paramID, *c));
        }
        else
        {
            auto* s = new Slider (Slider::RotaryVerticalDrag, Slider::TextBoxBelow);
            s->setTextBoxStyle (Slider::TextBoxBelow, false, 70, 14);
            s->setColour (Slider::rotarySliderFillColourId, Colour (0xff1b95cb));
            s->setColour (Slider::thumbColourId, Colours::white);
            comps.add (s);
            sAtt.add (new AudioProcessorValueTreeState::SliderAttachment (p.apvts, pid->paramID, *s));
        }
        addAndMakeVisible (comps.getLast());
    }
    setSize (1060, 620);
    startTimerHz (30);
}

void PA3609Editor::paint (Graphics& g)
{
    g.setGradientFill (ColourGradient (Colour (0xff4a5b6b), 0, 0, Colour (0xff2f3b47), 0, (float) getHeight(), false));
    g.fillAll();
    g.setColour (Colour (0xffe8a24a));
    g.setFont (FontOptions (22.f, Font::bold));
    g.drawText ("PARAGON AUDIO   PA-3609", 20, 8, 500, 30, Justification::left);

    auto bar = [&] (float x, float y, float w, float db, const String& t)
    {
        g.setColour (Colour (0xff0d1012)); g.fillRect (x, y, w, 10.f);
        const float f = jlimit (0.f, 1.f, (db + 60.f) / 60.f) * w;
        g.setColour (db > -3 ? Colour (0xffe0473c) : db > -12 ? Colour (0xffe8c24a) : Colour (0xff3fbf6f));
        g.fillRect (x, y, f, 10.f);
        g.setColour (Colours::white); g.setFont (11.f); g.drawText (t, (int) x - 34, (int) y - 2, 32, 14, Justification::right);
    };
    auto fall = [] (float& d, float v) { d = jmax (20.f * std::log10 (v + 1e-9f), d - 1.f); return d; };
    bar (700, 18, 140, fall (dispIn,  proc.inPk.load()),  "in");
    bar (900, 18, 140, fall (dispOut, proc.outPk.load()), "out");
    for (int i = 0; i < 2; ++i)
    {
        dispGr[i] += (proc.grMeter[i].load() - dispGr[i]) * .25f;
        g.setColour (Colour (0xff0d1012)); g.fillRect (20.f, 50.f + i * 12, 300.f, 8.f);
        g.setColour (Colour (0xffe8a24a)); g.fillRect (20.f, 50.f + i * 12, jlimit (0.f, 1.f, dispGr[i] / 20.f) * 300.f, 8.f);
        g.setColour (Colours::white); g.setFont (10.f); g.drawText ("GR" + String (i + 1), 326, 47 + i * 12, 40, 12, Justification::left);
    }
}

void PA3609Editor::resized()
{
    const int cols = 10, cw = 104, ch = 118;
    for (int i = 0; i < comps.size(); ++i)
    {
        const int x = 10 + (i % cols) * cw, y = 85 + (i / cols) * ch;
        labels[i]->setBounds (x, y, cw, 14);
        if (dynamic_cast<ComboBox*> (comps[i]))        comps[i]->setBounds (x + 14, y + 34, cw - 28, 24);
        else if (dynamic_cast<ToggleButton*> (comps[i])) comps[i]->setBounds (x + cw / 2 - 11, y + 32, 24, 24);
        else                                             comps[i]->setBounds (x + 8, y + 16, cw - 16, 94);
    }
}
