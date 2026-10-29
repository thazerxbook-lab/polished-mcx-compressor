#pragma once
#include "PluginProcessor.h"
#include "AlienUI.h"
#include <array>

//==============================================================================
// Curva de transferencia (entrada vs salida) con punto en vivo
class CurveView : public juce::Component, private juce::Timer
{
public:
    explicit CurveView (PolishedAudioProcessor& p) : proc (p) { startTimerHz (30); }
    ~CurveView() override { stopTimer(); }
    void paint (juce::Graphics&) override;
private:
    void timerCallback() override { repaint(); }
    PolishedAudioProcessor& proc;
};

//==============================================================================
// Historial de reduccion de ganancia (scroll) + nivel de entrada
class HistoryView : public juce::Component, private juce::Timer
{
public:
    explicit HistoryView (PolishedAudioProcessor& p) : proc (p) { gr.fill (0.f); in.fill (-120.f); startTimerHz (30); }
    ~HistoryView() override { stopTimer(); }
    void paint (juce::Graphics&) override;
private:
    static constexpr int N = 480;
    void timerCallback() override;
    PolishedAudioProcessor& proc;
    std::array<float, N> gr {}, in {};
    int head = 0;
};

//==============================================================================
// Medidores verticales: IN / GR / OUT
class MeterView : public juce::Component, private juce::Timer
{
public:
    explicit MeterView (PolishedAudioProcessor& p) : proc (p) { startTimerHz (30); }
    ~MeterView() override { stopTimer(); }
    void paint (juce::Graphics&) override;
private:
    void timerCallback() override { repaint(); }
    PolishedAudioProcessor& proc;
};

//==============================================================================
class PolishedEditor : public juce::AudioProcessorEditor
{
public:
    explicit PolishedEditor (PolishedAudioProcessor&);
    ~PolishedEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void renderBackground();
    juce::StringArray choicesOf (const juce::String& id);

    PolishedAudioProcessor& proc;
    AlienLookAndFeel laf;
    CurveView curve;
    HistoryView history;
    MeterView meters;
    juce::Image background;

    std::array<juce::Slider, 9> knobs;
    std::array<std::unique_ptr<SliderAtt>, 9> knobAtt;
    juce::ComboBox modeBox, detectorBox;
    juce::ToggleButton arcBtn { "ARC  AUTO RELEASE" }, autoBtn { "AUTO GAIN" };
    std::unique_ptr<ComboAtt> modeAtt, detectorAtt;
    std::unique_ptr<ButtonAtt> arcAtt, autoAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolishedEditor)
};
