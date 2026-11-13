#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace juce;

namespace
{
    const char* LREC[] = { "50", "100", "200", "800", "a1", "a2" };
    const char* CREC[] = { "100", "400", "800", "1500", "a1", "a2" };
    const char* RAT[]  = { "1.5:1", "2:1", "3:1", "4:1", "6:1" };
    const float LRV[]  = { 50, 100, 200, 800, -1, -2 };
    const float CRV[]  = { 100, 400, 800, 1500, -1, -2 };
    const float RATV[] = { 1.5f, 2, 3, 4, 6 };
    const float OFFSET = 20.f; // 0 dBFS = +20 dBu

    float dbf (float v)  { return 20.f * std::log10 (v + 1e-9f); }
    float p10 (float v)  { return std::pow (10.f, v / 20.f); }

    float grc (float lv, float thr, float ratio, float knee)
    {
        const float o = lv - thr, k = 1.f - 1.f / ratio;
        if (2 * o < -knee) return 0;
        if (2 * std::abs (o) <= knee) { const float x = o + knee / 2; return k * x * x / (2 * knee); }
        return k * o;
    }
    float relc (float r, float tot, float fs)
    {
        if (r > 0) return r;
        const float tau = std::min (1.2f, std::max (.08f, 1.2f - tot * .07f)) * (r == -2 ? 2.f : 1.f);
        return 1.f - std::exp (-1.f / (tau * fs));
    }
    float track (float cur, float tgt, float a, float r, float tot, float fs)
    { return tgt > cur ? cur + (tgt - cur) * a : cur + (tgt - cur) * relc (r, tot, fs); }

    struct Q { bool li, ci; float lt, ct, cx, cg, la, ca, lr, cr, oa1, or1, oa2, or2; };
}

AudioProcessorValueTreeState::ParameterLayout PA3609Processor::layout()
{
    AudioProcessorValueTreeState::ParameterLayout L;
    auto F = [&] (String id, String n, float a, float b, float d)
        { L.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, n, NormalisableRange<float> (a, b), d)); };
    auto B = [&] (String id, String n, bool d)
        { L.add (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, n, d)); };
    auto C = [&] (String id, String n, StringArray s, int d)
        { L.add (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, n, s, d)); };

    for (int c = 1; c <= 2; ++c)
    {
        String s (c);
        F ("l_thr" + s, "Limit " + s + " Thresh", 4, 15, 12);
        C ("l_rec" + s, "Limit " + s + " Recovery", StringArray (LREC, 6), 1);
        B ("l_fast" + s, "Limit " + s + " Fast", true);
        B ("l_in" + s, "Limit " + s + " In", true);
        F ("c_thr" + s, "Comp " + s + " Thresh", -20, 10, -4);
        C ("c_rec" + s, "Comp " + s + " Recovery", StringArray (CREC, 6), 2);
        F ("c_gain" + s, "Comp " + s + " Gain", 0, 20, 6);
        C ("c_ratio" + s, "Comp " + s + " Ratio", StringArray (RAT, 5), 2);
        B ("c_fast" + s, "Comp " + s + " Fast", true);
        B ("c_in" + s, "Comp " + s + " In", true);
    }
    B ("bypass", "Bypass", false);   B ("stereo", "Stereo Mode", true);
    B ("ms", "Mid/Side", false);     F ("link", "Link %", 0, 100, 100);
    B ("opto", "Opto Mode", false);  B ("olim", "Opto Limit", false);
    F ("hf", "HF Filter %", 0, 100, 0);
    F ("inG", "Input dB", -12, 12, 0);   F ("sch", "SC HPF Hz", 20, 400, 20);
    F ("drive", "Drive %", 0, 100, 50);  F ("mix", "Mix %", 0, 100, 100);
    F ("outG", "Output dB", -12, 12, 0);
    return L;
}

PA3609Processor::PA3609Processor()
    : AudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PA3609", layout()) {}

void PA3609Processor::prepareToPlay (double, int) { st[0] = St(); st[1] = St(); }

float PA3609Processor::sat (float x, float tot) const
{
    const float k = (.04f + .035f * tot) * satd;
    return std::tanh (x * (1 + k)) / (1 + k) + .02f * k * x * std::abs (x);
}

void PA3609Processor::processBlock (AudioBuffer<float>& b, MidiBuffer&)
{
    ScopedNoDenormals nd;
    if (b.getNumChannels() < 2) return;
    const float fs = (float) getSampleRate();
    const int n = b.getNumSamples();
    float* L = b.getWritePointer (0);
    float* R = b.getWritePointer (1);
    auto g = [&] (const String& id) { return apvts.getRawParameterValue (id)->load(); };

    float ip = 0, op = 0;
    if (g ("bypass") > .5f)
    {
        for (int i = 0; i < n; ++i) ip = std::max ({ ip, std::abs (L[i]), std::abs (R[i]) });
        inPk = outPk = ip; grMeter[0] = grMeter[1] = 0; return;
    }

    const bool ms = g ("ms") > .5f, shared = g ("stereo") > .5f && ! ms;
    const bool opto = g ("opto") > .5f, olim = g ("olim") > .5f;
    const float lk = g ("link") / 100.f, hfA = 1 - std::exp (-6.283f * 3000.f / fs), hk = g ("hf") / 100.f * 2.5f;
    const float ig = p10 (g ("inG")), og = p10 (g ("outG")), mixw = g ("mix") / 100.f;
    const float shA = 1 - std::exp (-6.283f * g ("sch") / fs);
    satd = g ("drive") / 50.f;

    auto prep = [&] (int ch)
    {
        String s (ch + 1); Q q;
        q.li = g ("l_in" + s) > .5f;  q.ci = g ("c_in" + s) > .5f;
        q.lt = g ("l_thr" + s) - OFFSET;  q.ct = g ("c_thr" + s) - OFFSET;
        q.cx = RATV[(int) g ("c_ratio" + s)];  q.cg = g ("c_gain" + s);
        const bool lf = g ("l_fast" + s) > .5f, cf = g ("c_fast" + s) > .5f;
        q.la = 1 - std::exp (-1 / ((lf ? .0005f : .005f) * fs));
        q.ca = 1 - std::exp (-1 / ((cf ? .002f : .02f) * fs));
        const float lv = LRV[(int) g ("l_rec" + s)], cv = CRV[(int) g ("c_rec" + s)];
        q.lr = lv > 0 ? 1 - std::exp (-1 / (lv / 1000.f * fs)) : lv;
        q.cr = cv > 0 ? 1 - std::exp (-1 / (cv / 1000.f * fs)) : cv;
        q.oa1 = 1 - std::exp (-1 / ((olim ? .004f : .01f) * fs));
        q.or1 = 1 - std::exp (-1 / (.06f * fs));
        q.oa2 = 1 - std::exp (-1 / (.08f * fs));
        q.or2 = 1 - std::exp (-1 / ((olim ? 1.2f : 2.f) * fs));
        return q;
    };
    const Q Qs[2] = { prep (0), prep (shared ? 0 : 1) };

    for (int i = 0; i < n; ++i)
    {
        ip = std::max ({ ip, std::abs (L[i]), std::abs (R[i]) });
        const float dl = L[i] * ig, dr = R[i] * ig;
        float a0 = dl, b0 = dr;
        if (ms) { const float m = (a0 + b0) * .5f, s = (a0 - b0) * .5f; a0 = m; b0 = s; }
        const float xin[2] = { a0, b0 };
        float y[2], dd[2], ds[2];
        for (int c = 0; c < 2; ++c)
        {
            St& S = st[c];
            const float v = xin[c] - S.sh; S.sh += shA * v;
            const float hp = v - S.lp; S.lp += hfA * hp;
            ds[c] = v + hk * hp; dd[c] = std::abs (ds[c]);
        }
        const float mx = std::max (dd[0], dd[1]);

        for (int c = 0; c < 2; ++c)
        {
            const Q& q = Qs[c]; St& S = st[c];
            const float lvIn = (1 - lk) * dd[c] + lk * mx;
            S.l = track (S.l, q.li ? grc (dbf (lvIn), q.lt, 20, 1.5f) : 0, q.la, q.lr, S.l + S.c, fs);
            const float lvC = lvIn * p10 (-S.l);
            if (opto)
            {
                const float T = q.ci ? grc (dbf (lvC), q.ct, olim ? 10.f : 3.f, olim ? 6.f : 14.f) : 0;
                S.o1 = track (S.o1, T, q.oa1, q.or1, 0, fs);
                S.o2 = track (S.o2, T, q.oa2, q.or2, 0, fs);
                S.c = std::max (S.o1, S.o2);
            }
            else
                S.c = track (S.c, q.ci ? grc (dbf (lvC), q.ct, q.cx, 6) : 0, q.ca, q.cr, S.l + S.c, fs);

            const float tot = S.l + S.c, gn = p10 ((q.ci ? q.cg : 0) - tot);
            y[c] = sat (xin[c] * gn, tot);
            if (opto) y[c] += .015f * y[c] * std::abs (y[c]);
        }
        float wl = y[0], wr = y[1];
        if (ms) { wl = y[0] + y[1]; wr = y[0] - y[1]; }
        L[i] = (wl * mixw + dl * (1 - mixw)) * og;
        R[i] = (wr * mixw + dr * (1 - mixw)) * og;
        op = std::max ({ op, std::abs (L[i]), std::abs (R[i]) });
    }
    grMeter[0] = st[0].l + st[0].c; grMeter[1] = st[1].l + st[1].c;
    inPk = ip; outPk = op;
}

AudioProcessorEditor* PA3609Processor::createEditor() { return new PA3609Editor (*this); }

void PA3609Processor::getStateInformation (MemoryBlock& d)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, d);
}
void PA3609Processor::setStateInformation (const void* d, int s)
{
    if (auto xml = getXmlFromBinary (d, s)) apvts.replaceState (ValueTree::fromXml (*xml));
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PA3609Processor(); }
