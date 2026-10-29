#pragma once
// Estilo alien compartido (OVNI, rayo tractor, knobs y menus de caricatura)
#include <juce_audio_processors/juce_audio_processors.h>

class AlienLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AlienLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (12.f, juce::Font::bold); }
    juce::Font getPopupMenuFont() override { return juce::Font (13.f, juce::Font::bold); }
    juce::Font getLabelFont (juce::Label&) override { return juce::Font (11.f, juce::Font::bold); }
};

//==============================================================================
inline const juce::Colour kInk      { 0xff000000 };

inline const juce::Colour kPurple   { 0xff6d3fe0 };
inline const juce::Colour kNavy     { 0xff071545 };
inline const juce::Colour kYellow   { 0xffffe14a };
inline const juce::Colour kLime     { 0xffb6ff3b };

inline void drawOutlinedText (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> r,
                       juce::Font f, juce::Colour fill, float outline,
                       juce::Justification j = juce::Justification::centredLeft)
{
    juce::GlyphArrangement ga;
    ga.addFittedText (f, text, r.getX(), r.getY(), r.getWidth(), r.getHeight(), j, 1);
    juce::Path p;
    ga.createPath (p);
    g.setColour (kInk);
    g.strokePath (p, juce::PathStrokeType (outline, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (fill);
    g.fillPath (p);
}

inline void drawStar (juce::Graphics& g, float cx, float cy, float r)
{
    juce::Path p;
    const float inner = r * 0.28f;
    for (int i = 0; i < 8; ++i)
    {
        const float a = juce::MathConstants<float>::pi * 0.25f * (float) i - juce::MathConstants<float>::halfPi;
        const float rad = (i % 2 == 0) ? r : inner;
        const float x = cx + std::cos (a) * rad, y = cy + std::sin (a) * rad;
        if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
    }
    p.closeSubPath();
    g.setColour (kInk);
    g.strokePath (p, juce::PathStrokeType (3.f, juce::PathStrokeType::mitered));
    g.setColour (kYellow);
    g.fillPath (p);
}

inline void drawCloud (juce::Graphics& g, float x, float y, float w)
{
    const float h = w * 0.4f;
    juce::Path p;
    p.addEllipse (x, y + 0.4f * h, 0.5f * w, 0.6f * h);
    p.addEllipse (x + 0.22f * w, y, 0.5f * w, 0.95f * h);
    p.addEllipse (x + 0.5f * w, y + 0.3f * h, 0.5f * w, 0.7f * h);
    g.setColour (kInk);
    g.strokePath (p, juce::PathStrokeType (7.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (juce::Colour (0xff9b7bff));
    g.fillPath (p);
}

inline void drawEllipseOutlined (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour fill, float stroke)
{
    g.setColour (fill);
    g.fillEllipse (r);
    g.setColour (kInk);
    g.drawEllipse (r, stroke);
}

inline void drawUfo (juce::Graphics& g, float cx)
{
    using R = juce::Rectangle<float>;
    // domo
    drawEllipseOutlined (g, R (cx - 65.f, 8.f, 130.f, 90.f), juce::Colour (0xff3b46e8), 3.5f);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.fillEllipse (cx - 50.f, 20.f, 22.f, 34.f);
    // alien
    drawEllipseOutlined (g, R (cx - 27.f, 24.f, 54.f, 52.f), juce::Colour (0xff8be85a), 3.f);
    drawEllipseOutlined (g, R (cx - 21.f, 40.f, 17.f, 22.f), kInk, 1.f);
    drawEllipseOutlined (g, R (cx + 4.f, 40.f, 17.f, 22.f), kInk, 1.f);
    g.setColour (juce::Colours::white);
    g.fillEllipse (cx - 15.f, 44.f, 5.f, 5.f);
    g.fillEllipse (cx + 10.f, 44.f, 5.f, 5.f);
    juce::Path smile;
    smile.startNewSubPath (cx - 8.f, 64.f);
    smile.quadraticTo (cx, 71.f, cx + 8.f, 64.f);
    g.setColour (kInk);
    g.strokePath (smile, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    // plato
    drawEllipseOutlined (g, R (cx - 150.f, 56.f, 300.f, 60.f), kPurple, 3.5f);
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.fillEllipse (cx - 120.f, 62.f, 120.f, 14.f);
    // manitos del alien
    drawEllipseOutlined (g, R (cx - 36.f, 56.f, 18.f, 11.f), juce::Colour (0xff8be85a), 2.5f);
    drawEllipseOutlined (g, R (cx + 18.f, 56.f, 18.f, 11.f), juce::Colour (0xff8be85a), 2.5f);
    // luces
    const float lx[] = { -105.f, -52.f, 52.f, 105.f };
    const float ly[] = { 84.f, 91.f, 91.f, 84.f };
    for (int i = 0; i < 4; ++i)
        drawEllipseOutlined (g, R (cx + lx[i] - 17.f, ly[i], 34.f, 13.f), kYellow, 2.5f);
    // luz del rayo
    drawEllipseOutlined (g, R (cx - 45.f, 103.f, 90.f, 18.f), juce::Colour (0xffd8ff3a), 3.f);
}

inline AlienLookAndFeel::AlienLookAndFeel()
{
    setColour (juce::ComboBox::textColourId, juce::Colours::white);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff0b1f6b));
    setColour (juce::PopupMenu::textColourId, juce::Colours::white);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, kPurple);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId, kPurple);
    setColour (juce::TextEditor::backgroundColourId, kNavy);
    setColour (juce::TextEditor::textColourId, juce::Colours::white);
    setColour (juce::TextEditor::outlineColourId, kPurple);
}

inline void AlienLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                         float startAngle, float endAngle, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.f);
    const float r = std::min (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const float ang = startAngle + pos * (endAngle - startAngle);
    const auto col = s.findColour (juce::Slider::rotarySliderFillColourId);
    const float tr = r - 3.f;

    juce::Path track;
    track.addCentredArc (c.x, c.y, tr, tr, 0.f, startAngle, endAngle, true);
    g.setColour (kInk);
    g.strokePath (track, juce::PathStrokeType (8.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (juce::Colour (0xff0a1650));
    g.strokePath (track, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path val;
    val.addCentredArc (c.x, c.y, tr, tr, 0.f, startAngle, ang, true);
    g.setColour (col);
    g.strokePath (val, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float kr = r - 9.f;
    g.setColour (kInk);
    g.fillEllipse (c.x - kr - 2.f, c.y - kr - 2.f, 2.f * kr + 4.f, 2.f * kr + 4.f);
    g.setColour (kPurple);
    g.fillEllipse (c.x - kr, c.y - kr, 2.f * kr, 2.f * kr);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.fillEllipse (c.x - kr * 0.6f, c.y - kr * 0.85f, kr * 0.8f, kr * 0.4f);

    const float dx = std::sin (ang), dy = -std::cos (ang);
    juce::Path ptr;
    ptr.startNewSubPath (c.x + dx * kr * 0.25f, c.y + dy * kr * 0.25f);
    ptr.lineTo (c.x + dx * kr * 0.92f, c.y + dy * kr * 0.92f);
    g.setColour (kInk);
    g.strokePath (ptr, juce::PathStrokeType (5.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (col);
    g.strokePath (ptr, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

inline void AlienLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (1.5f);
    g.setColour (box.isEnabled() ? kPurple : juce::Colours::grey);
    g.fillRoundedRectangle (r, 7.f);
    g.setColour (kInk);
    g.drawRoundedRectangle (r, 7.f, 2.5f);
    juce::Path arrow;
    const float cx = (float) w - 12.f, cy = (float) h * 0.5f;
    arrow.addTriangle (cx - 4.f, cy - 2.f, cx + 4.f, cy - 2.f, cx, cy + 3.f);
    g.setColour (kYellow);
    g.fillPath (arrow);
}

inline void AlienLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (7, 1, box.getWidth() - 24, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

inline void AlienLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    if (b.getProperties().contains ("pill"))
    {
        auto r = b.getLocalBounds().toFloat().reduced (1.5f);
        g.setColour (b.getToggleState() ? kLime : juce::Colour (0xff2a2a8c));
        g.fillRoundedRectangle (r, 9.f);
        g.setColour (kInk);
        g.drawRoundedRectangle (r, 9.f, 2.5f);
        g.setColour (b.getToggleState() ? kInk : juce::Colours::white);
        g.setFont (juce::Font (11.f, juce::Font::bold));
        g.drawText (b.getButtonText(), r, juce::Justification::centred);
        return;
    }
    auto r = b.getLocalBounds().toFloat().reduced (2.f);
    const auto col = b.findColour (juce::ToggleButton::tickColourId);
    g.setColour (b.getToggleState() ? col : juce::Colour (0xff1a2a78));
    g.fillEllipse (r);
    if (b.getToggleState())
    {
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.fillEllipse (r.getX() + 4.f, r.getY() + 3.f, r.getWidth() * 0.35f, r.getHeight() * 0.25f);
    }
    g.setColour (highlighted ? juce::Colours::white : kInk);
    g.drawEllipse (r, 2.5f);
}

//

//==============================================================================
// Panel estilo caricatura (relleno azul noche + contorno negro grueso)
inline void drawPanelBox (juce::Graphics& g, juce::Rectangle<float> r, float corner = 12.f)
{
    g.setColour (juce::Colour (0xe0071545));
    g.fillRoundedRectangle (r, corner);
    g.setColour (kInk);
    g.drawRoundedRectangle (r, corner, 3.f);
}

// Escena completa: cielo, estrellas, nubes, luna, rayo tractor y OVNI con alien
inline void drawAlienScene (juce::Graphics& g, int W, int H)
{
    const float cx = 480.f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0d3fb0), 0.f, 0.f, juce::Colour (0xff08266e), 0.f, (float) H, false));
    g.fillAll();
    g.setColour (kInk);
    g.drawRect (0, 0, W, H, 4);

    const float stars[][3] = { { 350, 34, 9 }, { 600, 30, 10 }, { 660, 98, 6 }, { 790, 26, 8 }, { 925, 40, 8 }, { 300, 100, 6 }, { 22, 40, 7 }, { 560, 100, 5 } };
    for (auto& st : stars) drawStar (g, st[0], st[1], st[2]);
    drawCloud (g, -20.f, 84.f, 120.f);
    drawCloud (g, 690.f, 88.f, 90.f);

    auto crescent = [&] (float R, float Rc, juce::Colour c)
    {
        g.saveState();
        juce::Path cl;
        cl.setUsingNonZeroWinding (false);
        cl.addRectangle (640.f, 0.f, 160.f, 120.f);
        cl.addEllipse (732.f - Rc, 32.f - Rc, 2.f * Rc, 2.f * Rc);
        g.reduceClipRegion (cl);
        g.setColour (c);
        g.fillEllipse (716.f - R, 44.f - R, 2.f * R, 2.f * R);
        g.restoreState();
    };
    crescent (32.f, 21.f, kInk);
    crescent (28.f, 24.f, kYellow);

    juce::Path beam;
    beam.startNewSubPath (cx - 45.f, 112.f);
    beam.lineTo (cx + 45.f, 112.f);
    beam.lineTo (cx + 560.f, (float) H);
    beam.lineTo (cx - 560.f, (float) H);
    beam.closeSubPath();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff0ff4a), 0.f, 112.f, juce::Colour (0xff6fe026), 0.f, (float) H, false));
    g.fillPath (beam);
    for (int k = -2; k <= 2; ++k)
    {
        juce::Path ray;
        ray.startNewSubPath (cx + k * 14.f, 112.f);
        ray.lineTo (cx + k * 14.f + 10.f, 112.f);
        ray.lineTo (cx + k * 200.f + 50.f, (float) H);
        ray.lineTo (cx + k * 200.f - 50.f, (float) H);
        ray.closeSubPath();
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.fillPath (ray);
    }
    drawUfo (g, cx);
}
