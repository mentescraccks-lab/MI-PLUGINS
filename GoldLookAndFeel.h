#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace Palette
{
    inline const juce::Colour bg         { 0xff0b0c10 };
    inline const juce::Colour panel      { 0xff101116 };
    inline const juce::Colour panelEdge  { 0xff22242c };
    inline const juce::Colour coreEdge   { 0xff5b3b0e };
    inline const juce::Colour orange     { 0xfff59e0b };
    inline const juce::Colour gold       { 0xfffbbf24 };
    inline const juce::Colour text       { 0xffa7adba };
    inline const juce::Colour dim        { 0xff5d6578 };
    inline const juce::Colour field      { 0xff15161c };
    inline const juce::Colour fieldEdge  { 0xff262831 };
    inline const juce::Colour track      { 0xff2a2c34 };
    inline const juce::Colour knobTop    { 0xff3a3c45 };
    inline const juce::Colour knobBottom { 0xff121318 };
    inline const juce::Colour dark       { 0xff1a1206 };

    // Requiere JUCE 8 (FontOptions)
    inline juce::Font ui (float size, bool bold = false)
    {
        return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
    }

    inline juce::Font mono (float size, bool bold = false)
    {
        return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), size,
                                              bold ? juce::Font::bold : juce::Font::plain));
    }
}

class GoldLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GoldLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, Palette::bg);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float pos,
                           float startAngle, float endAngle, juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (2.0f);
        const float r = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto c = bounds.getCentre();
        const float angle = startAngle + pos * (endAngle - startAngle);

        const float trackR = r - 2.0f;
        const float stroke = r > 30.0f ? 4.0f : 3.2f;

        juce::Path track;
        track.addCentredArc (c.x, c.y, trackR, trackR, 0.0f, startAngle, endAngle, true);
        g.setColour (juce::Colour (0xff22140a));
        g.strokePath (track, juce::PathStrokeType (stroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path value;
        value.addCentredArc (c.x, c.y, trackR, trackR, 0.0f, startAngle, angle, true);
        g.setColour (Palette::orange);
        g.strokePath (value, juce::PathStrokeType (stroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float bodyR = r - (r > 30.0f ? 9.0f : 7.0f);
        g.setGradientFill (juce::ColourGradient (Palette::knobTop, c.x - bodyR * 0.5f, c.y - bodyR * 0.8f,
                                                 Palette::knobBottom, c.x + bodyR * 0.5f, c.y + bodyR, false));
        g.fillEllipse (c.x - bodyR, c.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        g.setColour (juce::Colour (0xff0a0b0e));
        g.drawEllipse (c.x - bodyR, c.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.2f);

        juce::Path pointer;
        pointer.addRoundedRectangle (-1.3f, -bodyR + 3.0f, 2.6f, bodyR * 0.42f, 1.2f);
        g.setColour (Palette::gold);
        g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (c.x, c.y));
    }

    int getSliderThumbRadius (juce::Slider&) override { return 8; }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float, float, juce::Slider::SliderStyle, juce::Slider&) override
    {
        const float cy = (float) y + (float) height * 0.5f;
        juce::Rectangle<float> trk ((float) x + 2.0f, cy - 2.0f, (float) width - 4.0f, 4.0f);

        g.setColour (Palette::track);
        g.fillRoundedRectangle (trk, 2.0f);

        g.setColour (Palette::orange.withAlpha (0.45f));
        g.fillRoundedRectangle (trk.withRight (sliderPos), 2.0f);

        g.setColour (Palette::gold.withAlpha (0.18f));
        g.fillEllipse (sliderPos - 11.0f, cy - 11.0f, 22.0f, 22.0f);
        g.setColour (Palette::gold);
        g.fillEllipse (sliderPos - 7.0f, cy - 7.0f, 14.0f, 14.0f);
    }
};
