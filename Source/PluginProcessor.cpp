#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout PolishedAudioProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto pid = [] (const char* id) { return ParameterID { id, 1 }; };

    // Threshold
    layout.add (std::make_unique<AudioParameterFloat> (pid ("thresh"), "Threshold",
        NormalisableRange<float> (-60.f, 0.f, 0.1f), -18.f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " dB"; })
                                       .withValueFromStringFunction ([] (const String& t) { return t.getFloatValue(); })));

    // Ratio (20 = infinito / limitador)
    NormalisableRange<float> rr (1.f, 20.f, 0.1f);
    rr.setSkewForCentre (4.f);
    layout.add (std::make_unique<AudioParameterFloat> (pid ("ratio"), "Ratio", rr, 4.f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return v >= 19.9f ? String ("inf:1") : String (v, 1) + ":1"; })
                                       .withValueFromStringFunction ([] (const String& t) { return t.contains ("inf") ? 20.f : t.getFloatValue(); })));

    // Attack
    NormalisableRange<float> ar (0.1f, 100.f, 0.f);
    ar.setSkewForCentre (10.f);
    layout.add (std::make_unique<AudioParameterFloat> (pid ("attack"), "Attack", ar, 10.f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (v, v < 10.f ? 2 : 1) + " ms"; })
                                       .withValueFromStringFunction ([] (const String& t) { return t.getFloatValue(); })));

    // Release
    NormalisableRange<float> rl (5.f, 2000.f, 0.f);
    rl.setSkewForCentre (150.f);
    layout.add (std::make_unique<AudioParameterFloat> (pid ("release"), "Release", rl, 150.f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return v >= 1000.f ? String (v / 1000.f, 2) + " s" : String ((int) std::round (v)) + " ms"; })
                                       .withValueFromStringFunction ([] (const String& t) { auto s = t.trim().toLowerCase(); float v = s.getFloatValue();
                                                                                           return (s.endsWith ("s") && ! s.endsWith ("ms")) ? v * 1000.f : v; })));

    auto pctText = [] (float v, int) { return String ((int) std::round (v)) + " %"; };
    auto pctFrom = [] (const String& t) { return t.getFloatValue(); };

    layout.add (std::make_unique<AudioParameterFloat> (pid ("knee"), "Knee", NormalisableRange<float> (0.f, 100.f, 1.f), 30.f,
        AudioParameterFloatAttributes().withStringFromValueFunction (pctText).withValueFromStringFunction (pctFrom)));

    layout.add (std::make_unique<AudioParameterFloat> (pid ("makeup"), "Gain", NormalisableRange<float> (-12.f, 24.f, 0.1f), 0.f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (v > 0.05f ? "+" : "") + String (v, 1) + " dB"; })
                                       .withValueFromStringFunction ([] (const String& t) { return t.getFloatValue(); })));

    layout.add (std::make_unique<AudioParameterFloat> (pid ("mix"), "Mix", NormalisableRange<float> (0.f, 100.f, 1.f), 100.f,
        AudioParameterFloatAttributes().withStringFromValueFunction (pctText).withValueFromStringFunction (pctFrom)));

    NormalisableRange<float> hr (20.f, 500.f, 0.f);
    hr.setSkewForCentre (100.f);
    layout.add (std::make_unique<AudioParameterFloat> (pid ("schpf"), "Sidechain HPF", hr, 20.f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return v <= 21.f ? String ("OFF") : String ((int) std::round (v)) + " Hz"; })
                                       .withValueFromStringFunction ([] (const String& t) { return t.toLowerCase().contains ("off") ? 20.f : t.getFloatValue(); })));

    layout.add (std::make_unique<AudioParameterFloat> (pid ("link"), "Stereo Link", NormalisableRange<float> (0.f, 100.f, 1.f), 100.f,
        AudioParameterFloatAttributes().withStringFromValueFunction (pctText).withValueFromStringFunction (pctFrom)));

    layout.add (std::make_unique<AudioParameterChoice> (pid ("mode"), "Mode", StringArray { "Electro", "Opto" }, 0));
    layout.add (std::make_unique<AudioParameterChoice> (pid ("detector"), "Detector", StringArray { "Peak", "RMS" }, 0));
    layout.add (std::make_unique<AudioParameterBool> (pid ("arc"), "ARC", true));
    layout.add (std::make_unique<AudioParameterBool> (pid ("auto"), "Auto Gain", false));

    return layout;
}

//==============================================================================
PolishedAudioProcessor::PolishedAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    pThresh   = apvts.getRawParameterValue ("thresh");
    pRatio    = apvts.getRawParameterValue ("ratio");
    pAttack   = apvts.getRawParameterValue ("attack");
    pRelease  = apvts.getRawParameterValue ("release");
    pKnee     = apvts.getRawParameterValue ("knee");
    pMakeup   = apvts.getRawParameterValue ("makeup");
    pMix      = apvts.getRawParameterValue ("mix");
    pScHpf    = apvts.getRawParameterValue ("schpf");
    pLink     = apvts.getRawParameterValue ("link");
    pMode     = apvts.getRawParameterValue ("mode");
    pArc      = apvts.getRawParameterValue ("arc");
    pDetector = apvts.getRawParameterValue ("detector");
    pAuto     = apvts.getRawParameterValue ("auto");
}

polished::Params PolishedAudioProcessor::getEngineParams() const
{
    polished::Params p;
    p.thresholdDb = pThresh->load();
    p.ratio       = pRatio->load();
    p.attackMs    = pAttack->load();
    p.releaseMs   = pRelease->load();
    p.kneeDb      = pKnee->load() * 0.24f;      // 0..100 % -> 0..24 dB
    p.makeupDb    = pMakeup->load();
    p.mix         = pMix->load() * 0.01f;
    p.scHz        = pScHpf->load();
    p.link        = pLink->load() * 0.01f;
    p.opto        = pMode->load() > 0.5f;
    p.arc         = pArc->load() > 0.5f;
    p.rms         = pDetector->load() > 0.5f;
    p.autoGain    = pAuto->load() > 0.5f;
    return p;
}

void PolishedAudioProcessor::prepareToPlay (double sr, int)
{
    sampleRateHz = sr;
    comp.prepare (sr);
    comp.setParams (getEngineParams());
}

bool PolishedAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo();
}

void PolishedAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, n);

    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    if (numCh == 0) return;

    float* L = buffer.getWritePointer (0);
    float* R = numCh > 1 ? buffer.getWritePointer (1) : nullptr;

    comp.setParams (getEngineParams());

    float inPk = 0.f, outPk = 0.f, grMax = 0.f;
    for (int i = 0; i < n; ++i)
    {
        float l = L[i], r = R ? R[i] : l;
        inPk = juce::jmax (inPk, std::abs (l), std::abs (r));
        comp.process (l, r);
        outPk = juce::jmax (outPk, std::abs (l), std::abs (r));
        grMax = juce::jmax (grMax, comp.grDb());
        L[i] = l;
        if (R) R[i] = r;
    }

    const float inDb = polished::gainToDb (inPk), outDb = polished::gainToDb (outPk);
    const float decay = 24.f * (float) n / (float) sampleRateHz;      // caida de 24 dB/s
    liveInDb.store (inDb);
    liveGrDb.store (grMax);
    meterInDb.store  (juce::jmax (inDb,  meterInDb.load()  - decay));
    meterOutDb.store (juce::jmax (outDb, meterOutDb.load() - decay));
    meterGrDb.store  (juce::jmax (grMax, meterGrDb.load()  - decay));
}

juce::AudioProcessorEditor* PolishedAudioProcessor::createEditor() { return new PolishedEditor (*this); }

void PolishedAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PolishedAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PolishedAudioProcessor(); }
