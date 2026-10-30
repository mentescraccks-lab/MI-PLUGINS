#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "GoldLookAndFeel.h"

//==============================================================================
// Texto con el valor de un slider (con o sin caja)
class ValueBox : public juce::Component, private juce::Slider::Listener
{
public:
    ValueBox (juce::Slider& s, bool boxedStyle, juce::Colour colourToUse, bool boldText,
              juce::String prefixText, juce::Justification just, float size)
        : slider (s), boxed (boxedStyle), colour (colourToUse), bold (boldText),
          prefix (std::move (prefixText)), justification (just), fontSize (size)
    {
        setInterceptsMouseClicks (false, false);
        slider.addListener (this);
    }

    ~ValueBox() override { slider.removeListener (this); }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (0.5f);
        if (boxed)
        {
            g.setColour (Palette::field);
            g.fillRoundedRectangle (r, 4.0f);
            g.setColour (Palette::fieldEdge);
            g.drawRoundedRectangle (r, 4.0f, 1.0f);
        }

        g.setColour (colour);
        g.setFont (Palette::mono (fontSize, bold));
        g.drawText (prefix + slider.getTextFromValue (slider.getValue()),
                    getLocalBounds().reduced (boxed ? 2 : 0, 0), justification, false);
    }

private:
    void sliderValueChanged (juce::Slider*) override { repaint(); }

    juce::Slider& slider;
    bool boxed;
    juce::Colour colour;
    bool bold;
    juce::String prefix;
    juce::Justification justification;
    float fontSize;
};

//==============================================================================
// Selector segmentado (1x / 2x / 4x  y  Modern / Classic / Hard)
class SegmentedSelector : public juce::Component
{
public:
    SegmentedSelector (juce::RangedAudioParameter& p, juce::StringArray labelsToUse)
        : labels (std::move (labelsToUse)),
          attachment (p, [this] (float v)
                      {
                          selected = juce::roundToInt (v);
                          repaint();
                          if (onChange != nullptr)
                              onChange();
                      })
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        attachment.sendInitialUpdate();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (Palette::field);
        g.fillRoundedRectangle (r, 7.0f);
        g.setColour (Palette::fieldEdge);
        g.drawRoundedRectangle (r, 7.0f, 1.0f);

        const float segW = (r.getWidth() - 8.0f) / (float) labels.size();
        for (int i = 0; i < labels.size(); ++i)
        {
            juce::Rectangle<float> seg (r.getX() + 4.0f + segW * (float) i, r.getY() + 4.0f, segW, r.getHeight() - 8.0f);
            const bool on = (i == selected);

            if (on)
            {
                g.setColour (Palette::orange);
                g.fillRoundedRectangle (seg.reduced (1.0f, 0.0f), 5.0f);
            }

            g.setColour (on ? Palette::dark : Palette::text);
            g.setFont (Palette::ui (11.5f, on));
            g.drawText (labels[i], seg, juce::Justification::centred, false);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const float segW = ((float) getWidth() - 9.0f) / (float) labels.size();
        const int idx = juce::jlimit (0, labels.size() - 1, (int) ((e.position.x - 4.0f) / segW));
        attachment.setValueAsCompleteGesture ((float) idx);
    }

    std::function<void()> onChange;

private:
    juce::StringArray labels;
    int selected = 0;
    juce::ParameterAttachment attachment;
};

//==============================================================================
// Botón ACTIVE con icono de escudo
class ActiveButton : public juce::ToggleButton
{
public:
    ActiveButton()
    {
        setButtonText ("ACTIVE");
        setClickingTogglesState (true);
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void paintButton (juce::Graphics& g, bool, bool) override
    {
        const bool on = getToggleState();
        auto r = getLocalBounds().toFloat().reduced (0.5f);

        g.setColour (Palette::field);
        g.fillRoundedRectangle (r, 7.0f);
        g.setColour (on ? juce::Colour (0xff3b3e4a) : Palette::fieldEdge);
        g.drawRoundedRectangle (r, 7.0f, 1.2f);

        juce::Path shield;
        shield.startNewSubPath (0.0f, -6.5f);
        shield.lineTo (5.5f, -4.2f);
        shield.lineTo (5.5f, 1.0f);
        shield.quadraticTo (5.5f, 5.0f, 0.0f, 7.0f);
        shield.quadraticTo (-5.5f, 5.0f, -5.5f, 1.0f);
        shield.lineTo (-5.5f, -4.2f);
        shield.closeSubPath();

        g.setColour (on ? juce::Colours::white : Palette::dim);
        g.strokePath (shield, juce::PathStrokeType (1.3f), juce::AffineTransform::translation (18.0f, r.getCentreY()));
        g.fillRect (juce::Rectangle<float> (17.2f, r.getCentreY() - 2.5f, 1.6f, 3.4f));
        g.fillEllipse (17.1f, r.getCentreY() + 1.8f, 1.8f, 1.8f);

        g.setFont (Palette::ui (12.0f, true));
        g.drawText ("ACTIVE", getLocalBounds().withTrimmedLeft (32), juce::Justification::centredLeft, false);
    }
};

//==============================================================================
// Medidor "CLIP ACTIVITY & GAIN REDUCTION"
class GainReductionMeter : public juce::Component
{
public:
    void setGainReduction (float db)
    {
        gr = juce::jmin (0.0f, db);
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (juce::Colour (0xff0d0e13));
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (Palette::fieldEdge);
        g.drawRoundedRectangle (r, 8.0f, 1.0f);

        g.setColour (Palette::text);
        g.setFont (Palette::ui (9.5f, true).withExtraKerningFactor (0.06f));
        g.drawText ("CLIP ACTIVITY & GAIN REDUCTION", 10, 8, 230, 13, juce::Justification::centredLeft, false);

        g.setColour (Palette::orange);
        g.setFont (Palette::mono (10.0f, true));
        g.drawText (juce::String::formatted ("-%.1f dB", std::abs (gr)), getWidth() - 110, 8, 100, 13,
                    juce::Justification::centredRight, false);

        juce::Rectangle<float> bar (10.0f, 28.0f, (float) getWidth() - 20.0f, 13.0f);
        g.setColour (juce::Colour (0xff16171d));
        g.fillRoundedRectangle (bar, 4.0f);

        const float frac = juce::jlimit (0.0f, 1.0f, -gr / 12.0f);
        if (frac > 0.0f)
        {
            g.setGradientFill (juce::ColourGradient (Palette::orange.darker (0.3f), bar.getX(), 0.0f,
                                                     Palette::gold, bar.getRight(), 0.0f, false));
            g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * frac), 4.0f);
        }

        static const char* labs[] = { "0dB", "-3dB", "-6dB", "-9dB", "-12dB" };
        g.setColour (juce::Colour (0xff6f7a94));
        g.setFont (Palette::mono (8.0f));
        for (int i = 0; i < 5; ++i)
        {
            const float cx = bar.getX() + 16.0f + (bar.getWidth() - 32.0f) * (float) i / 4.0f;
            g.drawText (labs[i], juce::Rectangle<float> (cx - 20.0f, bar.getY(), 40.0f, bar.getHeight()),
                        juce::Justification::centred, false);
        }
    }

private:
    float gr = 0.0f;
};

//==============================================================================
// Medidor estéreo (IN L / IN R  o  OUT L / OUT R)
class StereoMeter : public juce::Component
{
public:
    StereoMeter (juce::String left, juce::String right) : labelL (std::move (left)), labelR (std::move (right)) {}

    void setLevels (float l, float r)
    {
        levelL = l;
        levelR = r;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (Palette::field);
        g.fillRoundedRectangle (r, 7.0f);
        g.setColour (Palette::fieldEdge);
        g.drawRoundedRectangle (r, 7.0f, 1.0f);

        const float pad = 9.0f;
        const float barW = (r.getWidth() - pad * 2.0f - 8.0f) * 0.5f;

        g.setFont (Palette::mono (8.5f));
        g.setColour (Palette::dim);
        g.drawText (labelL, (int) pad, 6, 60, 12, juce::Justification::centredLeft, false);
        g.drawText (labelR, getWidth() - (int) pad - 60, 6, 60, 12, juce::Justification::centredRight, false);

        drawBar (g, { pad, 27.0f, barW, 9.0f }, levelL);
        drawBar (g, { pad + barW + 8.0f, 27.0f, barW, 9.0f }, levelR);
    }

private:
    static void drawBar (juce::Graphics& g, juce::Rectangle<float> bar, float level)
    {
        g.setColour (juce::Colour (0xff1e2027));
        g.fillRoundedRectangle (bar, 4.0f);

        const float db = juce::Decibels::gainToDecibels (level, -60.0f);
        const float frac = juce::jlimit (0.0f, 1.0f, juce::jmap (db, -60.0f, 0.0f, 0.0f, 1.0f));
        if (frac <= 0.0f)
            return;

        juce::ColourGradient grad (juce::Colour (0xff22c55e), bar.getX(), 0.0f,
                                   juce::Colour (0xffef4444), bar.getRight(), 0.0f, false);
        grad.addColour (0.7, juce::Colour (0xfff5b50b));
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bar.withWidth (juce::jmax (4.0f, bar.getWidth() * frac)), 4.0f);
    }

    juce::String labelL, labelR;
    float levelL = 0.0f, levelR = 0.0f;
};
