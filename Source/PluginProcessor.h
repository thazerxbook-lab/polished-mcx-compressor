#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "CompressorEngine.h"

class PolishedAudioProcessor : public juce::AudioProcessor
{
public:
    PolishedAudioProcessor();
    ~PolishedAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "POLISHED MCX"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    polished::Params getEngineParams() const;

    juce::AudioProcessorValueTreeState apvts;

    // medidores (los lee la interfaz)
    std::atomic<float> liveInDb { -120.f }, liveGrDb { 0.f };         // instantaneos por bloque
    std::atomic<float> meterInDb { -120.f }, meterOutDb { -120.f }, meterGrDb { 0.f };  // con caida

private:
    std::atomic<float> *pThresh {}, *pRatio {}, *pAttack {}, *pRelease {}, *pKnee {}, *pMakeup {},
                       *pMix {}, *pScHpf {}, *pLink {}, *pMode {}, *pArc {}, *pDetector {}, *pAuto {};

    polished::Compressor comp;
    double sampleRateHz = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolishedAudioProcessor)
};
