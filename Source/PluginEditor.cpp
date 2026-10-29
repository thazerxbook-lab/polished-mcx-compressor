#include "PluginEditor.h"

namespace
{
const juce::Colour kPink { 0xffff6ec7 };
const juce::Colour kCyan { 0xff3ee0ff };

struct KnobDef { const char* id; const char* caption; juce::uint32 colour; int x, y; };
const KnobDef kKnobs[9] =
{
    { "thresh",  "THRESHOLD", 0xffb6ff3b, 368, 168 },
    { "ratio",   "RATIO",     0xffffe14a, 454, 168 },
    { "attack",  "ATTACK",    0xffa77bff, 540, 168 },
    { "release", "RELEASE",   0xff3ee0ff, 626, 168 },
    { "knee",    "KNEE",      0xffff6ec7, 712, 168 },
    { "makeup",  "GAIN",      0xffffa63d, 408, 290 },
    { "mix",     "MIX",       0xffb6ff3b, 494, 290 },
    { "schpf",   "SC HPF",    0xffffe14a, 580, 290 },
    { "link",    "LINK",      0xff3ee0ff, 666, 290 }
};
} // namespace

//==============================================================================
void CurveView::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    drawPanelBox (g, b.reduced (2.f), 14.f);
    drawOutlinedText (g, "TRANSFER CURVE", { 14.f, 6.f, 220.f, 20.f }, juce::Font (13.f, juce::Font::bold), kLime, 4.f);

    const auto plot = b.withTrimmedLeft (44.f).withTrimmedRight (20.f).withTrimmedTop (34.f).withTrimmedBottom (32.f);
    auto X = [&] (float db) { return plot.getX() + (db + 60.f) / 60.f * plot.getWidth(); };
    auto Y = [&] (float db) { return plot.getBottom() - (db + 60.f) / 60.f * plot.getHeight(); };

    g.setFont (juce::Font (10.f, juce::Font::bold));
    for (int db = -60; db <= 0; db += 12)
    {
        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.drawVerticalLine ((int) X ((float) db), plot.getY(), plot.getBottom());
        g.drawHorizontalLine ((int) Y ((float) db), plot.getX(), plot.getRight());
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.drawText (juce::String (db), juce::Rectangle<float> (X ((float) db) - 16.f, plot.getBottom() + 4.f, 32.f, 12.f), juce::Justification::centred);
        g.drawText (juce::String (db), juce::Rectangle<float> (4.f, Y ((float) db) - 6.f, 34.f, 12.f), juce::Justification::centredRight);
    }
    g.drawText ("IN dB", juce::Rectangle<float> (plot.getRight() - 60.f, plot.getBottom() + 16.f, 70.f, 12.f), juce::Justification::centredRight);
    g.drawText ("OUT dB", juce::Rectangle<float> (4.f, 20.f, 70.f, 12.f), juce::Justification::centredLeft);

    // diagonal 1:1
    const float dashes[] = { 5.f, 5.f };
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawDashedLine (juce::Line<float> (X (-60.f), Y (-60.f), X (0.f), Y (0.f)), dashes, 2, 1.5f);

    const auto p = proc.getEngineParams();

    // umbral
    g.setColour (kLime.withAlpha (0.6f));
    g.drawDashedLine (juce::Line<float> (X (p.thresholdDb), plot.getY(), X (p.thresholdDb), plot.getBottom()), dashes, 2, 1.5f);

    // curva estatica (+ area de reduccion)
    juce::Path curve, fill;
    bool first = true;
    for (float x = plot.getX(); x <= plot.getRight() + 0.5f; x += 2.f)
    {
        const float db = -60.f + (x - plot.getX()) / plot.getWidth() * 60.f;
        const float out = db - polished::reductionDb (db, p.thresholdDb, p.ratio, p.kneeDb);
        const float y = juce::jlimit (plot.getY(), plot.getBottom(), Y (out));
        if (first) { curve.startNewSubPath (x, y); fill.startNewSubPath (x, y); first = false; }
        else       { curve.lineTo (x, y); fill.lineTo (x, y); }
    }
    fill.lineTo (X (0.f), Y (0.f));
    fill.lineTo (X (-60.f), Y (-60.f));
    fill.closeSubPath();
    g.setColour (kPink.withAlpha (0.20f));
    g.fillPath (fill);
    g.setColour (kInk);
    g.strokePath (curve, juce::PathStrokeType (5.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (kYellow);
    g.strokePath (curve, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // punto en vivo
    const float in = juce::jlimit (-60.f, 0.f, proc.liveInDb.load());
    if (in > -59.f)
    {
        const float out = in - proc.liveGrDb.load();
        const float px = X (in), py = juce::jlimit (plot.getY(), plot.getBottom(), Y (out));
        g.setColour (kInk);
        g.fillEllipse (px - 8.f, py - 8.f, 16.f, 16.f);
        g.setColour (kPink);
        g.fillEllipse (px - 6.f, py - 6.f, 12.f, 12.f);
    }
}

//==============================================================================
void HistoryView::timerCallback()
{
    gr[(size_t) head]  = proc.liveGrDb.load();
    in[(size_t) head]  = proc.liveInDb.load();
    head = (head + 1) % N;
    repaint();
}

void HistoryView::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    drawPanelBox (g, b.reduced (2.f), 14.f);
    drawOutlinedText (g, "GAIN REDUCTION", { 14.f, 6.f, 160.f, 20.f }, juce::Font (13.f, juce::Font::bold), kPink, 4.f);
    drawOutlinedText (g, "INPUT", { 150.f, 6.f, 80.f, 20.f }, juce::Font (13.f, juce::Font::bold), kCyan, 4.f);

    const auto plot = b.withTrimmedLeft (40.f).withTrimmedRight (16.f).withTrimmedTop (34.f).withTrimmedBottom (14.f);

    g.setFont (juce::Font (10.f, juce::Font::bold));
    for (int db = 0; db <= 24; db += 6)
    {
        const float y = plot.getY() + (float) db / 24.f * plot.getHeight();
        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.drawHorizontalLine ((int) y, plot.getX(), plot.getRight());
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.drawText (db == 0 ? "0" : "-" + juce::String (db), juce::Rectangle<float> (2.f, y - 6.f, 34.f, 12.f), juce::Justification::centredRight);
    }

    g.saveState();
    g.reduceClipRegion (plot.expanded (0.f, 3.f).toNearestInt());

    juce::Path inPath, grPath;
    grPath.startNewSubPath (plot.getX(), plot.getY());
    for (int i = 0; i < N; ++i)
    {
        const int idx = (head + i) % N;
        const float x = plot.getX() + (float) i / (float) (N - 1) * plot.getWidth();
        const float yIn = plot.getBottom() - juce::jlimit (0.f, 1.f, (in[(size_t) idx] + 60.f) / 60.f) * plot.getHeight();
        const float yGr = plot.getY() + juce::jlimit (0.f, 1.f, gr[(size_t) idx] / 24.f) * plot.getHeight();
        if (i == 0) inPath.startNewSubPath (x, yIn); else inPath.lineTo (x, yIn);
        grPath.lineTo (x, yGr);
    }
    grPath.lineTo (plot.getRight(), plot.getY());
    grPath.closeSubPath();

    g.setColour (kCyan.withAlpha (0.8f));
    g.strokePath (inPath, juce::PathStrokeType (1.5f));
    g.setGradientFill (juce::ColourGradient (kPink.withAlpha (0.75f), 0.f, plot.getY(), kPink.withAlpha (0.25f), 0.f, plot.getBottom(), false));
    g.fillPath (grPath);
    g.setColour (kInk);
    g.strokePath (grPath, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (kPink);
    g.strokePath (grPath, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.restoreState();
}

//==============================================================================
void MeterView::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    drawPanelBox (g, b.reduced (2.f), 14.f);
    drawOutlinedText (g, "METERS", { 12.f, 6.f, 110.f, 20.f }, juce::Font (13.f, juce::Font::bold), kYellow, 4.f);

    const float top = 38.f, h = 200.f, w = 26.f;
    const float xs[3] = { 14.f, 52.f, 90.f };
    const char* names[3] = { "IN", "GR", "OUT" };
    const float vals[3] = { proc.meterInDb.load(), proc.meterGrDb.load(), proc.meterOutDb.load() };

    for (int i = 0; i < 3; ++i)
    {
        const juce::Rectangle<float> bar (xs[i], top, w, h);
        g.setColour (juce::Colour (0xff0a1650));
        g.fillRoundedRectangle (bar, 5.f);

        juce::Rectangle<float> fillR;
        if (i == 1)   // GR cae desde arriba
        {
            const float f = juce::jlimit (0.f, 1.f, vals[i] / 24.f);
            fillR = juce::Rectangle<float> (bar.getX(), bar.getY(), w, h * f);
            g.setGradientFill (juce::ColourGradient (kYellow, 0.f, bar.getY(), kPink, 0.f, bar.getBottom(), false));
        }
        else
        {
            const float f = juce::jlimit (0.f, 1.f, (vals[i] + 60.f) / 60.f);
            fillR = juce::Rectangle<float> (bar.getX(), bar.getBottom() - h * f, w, h * f);
            juce::ColourGradient grad (kLime, 0.f, bar.getBottom(), kPink, 0.f, bar.getY(), false);
            grad.addColour (0.75, kYellow);
            g.setGradientFill (grad);
        }
        g.fillRoundedRectangle (fillR, 5.f);

        g.setColour (kInk);
        g.drawRoundedRectangle (bar, 5.f, 2.5f);

        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (12.f, juce::Font::bold));
        g.drawText (names[i], juce::Rectangle<float> (xs[i] - 6.f, top + h + 6.f, w + 12.f, 14.f), juce::Justification::centred);
        g.setFont (juce::Font (10.f, juce::Font::bold));
        const juce::String txt = (i == 1) ? juce::String (vals[i], 1) : (vals[i] < -99.f ? juce::String ("-inf") : juce::String (vals[i], 1));
        g.drawText (txt, juce::Rectangle<float> (xs[i] - 8.f, top + h + 22.f, w + 16.f, 12.f), juce::Justification::centred);
    }
}

//==============================================================================
PolishedEditor::PolishedEditor (PolishedAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p), curve (p), history (p), meters (p)
{
    setLookAndFeel (&laf);
    setSize (960, 650);
    setResizable (false, false);

    addAndMakeVisible (curve);
    addAndMakeVisible (history);
    addAndMakeVisible (meters);

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& s = knobs[i];
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 84, 16);
        s.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (kKnobs[i].colour));
        addAndMakeVisible (s);
        knobAtt[i] = std::make_unique<SliderAtt> (proc.apvts, kKnobs[i].id, s);
    }

    modeBox.addItemList (choicesOf ("mode"), 1);
    detectorBox.addItemList (choicesOf ("detector"), 1);
    addAndMakeVisible (modeBox);
    addAndMakeVisible (detectorBox);
    modeAtt     = std::make_unique<ComboAtt> (proc.apvts, "mode", modeBox);
    detectorAtt = std::make_unique<ComboAtt> (proc.apvts, "detector", detectorBox);

    for (auto* b : { &arcBtn, &autoBtn })
    {
        b->getProperties().set ("pill", true);
        addAndMakeVisible (b);
    }
    arcAtt  = std::make_unique<ButtonAtt> (proc.apvts, "arc", arcBtn);
    autoAtt = std::make_unique<ButtonAtt> (proc.apvts, "auto", autoBtn);

    resized();
}

PolishedEditor::~PolishedEditor() { setLookAndFeel (nullptr); }

juce::StringArray PolishedEditor::choicesOf (const juce::String& id)
{
    if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
        return c->choices;
    return {};
}

void PolishedEditor::resized()
{
    curve.setBounds   (20, 124, 330, 290);
    meters.setBounds  (810, 124, 130, 290);
    history.setBounds (20, 424, 500, 206);

    for (size_t i = 0; i < knobs.size(); ++i)
        knobs[i].setBounds (kKnobs[i].x, kKnobs[i].y, 84, 100);

    modeBox.setBounds     (548, 488, 180, 34);
    detectorBox.setBounds (742, 488, 180, 34);
    arcBtn.setBounds      (548, 556, 180, 38);
    autoBtn.setBounds     (742, 556, 180, 38);

    renderBackground();
}

void PolishedEditor::renderBackground()
{
    background = juce::Image (juce::Image::ARGB, getWidth(), getHeight(), true);
    juce::Graphics g (background);
    drawAlienScene (g, getWidth(), getHeight());

    drawOutlinedText (g, "POLISHED MCX", { 24.f, 14.f, 300.f, 52.f }, juce::Font (36.f, juce::Font::bold), kYellow, 8.f);
    drawOutlinedText (g, "COMPRESSOR", { 26.f, 62.f, 300.f, 18.f }, juce::Font (13.f, juce::Font::bold), kLime, 5.f);

    drawPanelBox (g, { 360.f, 124.f, 440.f, 290.f });
    drawOutlinedText (g, "DYNAMICS", { 376.f, 130.f, 200.f, 20.f }, juce::Font (13.f, juce::Font::bold), kLime, 4.f);

    drawPanelBox (g, { 530.f, 424.f, 410.f, 206.f });
    drawOutlinedText (g, "CHARACTER", { 548.f, 430.f, 200.f, 20.f }, juce::Font (13.f, juce::Font::bold), kYellow, 4.f);
}

void PolishedEditor::paint (juce::Graphics& g)
{
    g.drawImageAt (background, 0, 0);

    g.setFont (juce::Font (11.f, juce::Font::bold));
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    for (auto& k : kKnobs)
        g.drawText (k.caption, juce::Rectangle<int> (k.x, k.y - 15, 84, 14), juce::Justification::centred);

    g.drawText ("MODE",     juce::Rectangle<int> (548, 470, 180, 14), juce::Justification::centred);
    g.drawText ("DETECTOR", juce::Rectangle<int> (742, 470, 180, 14), juce::Justification::centred);
}
