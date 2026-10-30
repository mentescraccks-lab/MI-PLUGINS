#include "PluginEditor.h"

using namespace Palette;

namespace
{
    constexpr int kW = 800;
    constexpr int kH = 600;

    void drawLabel (juce::Graphics& g, const juce::String& t, float cx, float cy)
    {
        g.setColour (text);
        g.setFont (ui (11.5f, true).withExtraKerningFactor (0.05f));
        g.drawText (t, juce::Rectangle<float> (cx - 55.0f, cy - 8.0f, 110.0f, 16.0f), juce::Justification::centred, false);
    }

    void drawPulse (juce::Graphics& g, float x, float y)
    {
        juce::Path p;
        p.startNewSubPath (x, y + 7.0f);
        p.lineTo (x + 4.0f, y + 7.0f);
        p.lineTo (x + 6.5f, y + 1.0f);
        p.lineTo (x + 10.0f, y + 13.0f);
        p.lineTo (x + 12.5f, y + 4.0f);
        p.lineTo (x + 14.0f, y + 7.0f);
        p.lineTo (x + 17.0f, y + 7.0f);
        g.setColour (dim);
        g.strokePath (p, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    void drawPanel (juce::Graphics& g, juce::Rectangle<float> r, bool core)
    {
        if (core)
        {
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1c140a), r.getX(), r.getY(),
                                                     juce::Colour (0xff0f1014), r.getX(), r.getY() + 120.0f, false));
        }
        else
        {
            g.setColour (panel);
        }
        g.fillRoundedRectangle (r, 14.0f);

        g.setColour (core ? coreEdge : panelEdge);
        g.drawRoundedRectangle (r.reduced (0.5f), 14.0f, 1.2f);
    }
}

AndresBeatsClipAudioProcessorEditor::AndresBeatsClipAudioProcessorEditor (AndresBeatsClipAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      inputGainAtt (p.apvts, IDs::inputGain, inputGain),
      ceilingAtt   (p.apvts, IDs::ceiling,   ceiling),
      kneeAtt      (p.apvts, IDs::knee,      knee),
      mojoAtt      (p.apvts, IDs::mojo,      mojo),
      alchemyAtt   (p.apvts, IDs::alchemy,   alchemy),
      ceilTrimAtt  (p.apvts, IDs::ceilTrim,  ceilTrim),
      dryWetAtt    (p.apvts, IDs::dryWet,    dryWet),
      airAtt       (p.apvts, IDs::air,       air),
      inputGainBox (inputGain, true, gold, true, {}, juce::Justification::centred, 11.5f),
      ceilingBox   (ceiling,   true, gold, true, {}, juce::Justification::centred, 11.0f),
      kneeBox      (knee,      true, gold, true, {}, juce::Justification::centred, 11.0f),
      mojoBox      (mojo,      true, gold, true, {}, juce::Justification::centred, 11.0f),
      alchemyBox   (alchemy,   true, gold, true, {}, juce::Justification::centred, 11.0f),
      ceilTrimBox  (ceilTrim,  true, gold, true, {}, juce::Justification::centred, 11.0f),
      dryWetBox    (dryWet,    true, gold, true, {}, juce::Justification::centred, 11.0f),
      kneeHeader   (knee, false, text, false, "Knee: ", juce::Justification::centredRight, 10.5f),
      airValue     (air,  false, gold, true, {}, juce::Justification::centredRight, 11.0f),
      osSelector   (*p.apvts.getParameter (IDs::os),   juce::StringArray { "1x", "2x", "4x" }),
      modeSelector (*p.apvts.getParameter (IDs::mode), juce::StringArray { "Modern", "Classic", "Hard" }),
      activeAtt    (p.apvts, IDs::active, activeButton),
      inMeter  ("IN L",  "IN R"),
      outMeter ("OUT L", "OUT R")
{
    setLookAndFeel (&laf);
    setSize (kW, kH);
    setResizable (false, false);

    for (auto* s : { &inputGain, &ceiling, &knee, &mojo, &alchemy, &ceilTrim, &dryWet })
        setupKnob (*s);

    air.setSliderStyle (juce::Slider::LinearHorizontal);
    air.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (air);

    for (auto* c : std::initializer_list<juce::Component*> {
             &inputGainBox, &ceilingBox, &kneeBox, &mojoBox, &alchemyBox, &ceilTrimBox, &dryWetBox,
             &kneeHeader, &airValue, &osSelector, &modeSelector, &activeButton, &grMeter, &inMeter, &outMeter })
        addAndMakeVisible (*c);

    auto repaintFooter = [this] { repaint (20, 556, 760, 40); };
    osSelector.onChange   = repaintFooter;
    modeSelector.onChange = repaintFooter;

    startTimerHz (30);
}

AndresBeatsClipAudioProcessorEditor::~AndresBeatsClipAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void AndresBeatsClipAudioProcessorEditor::setupKnob (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    s.setMouseDragSensitivity (180);
    addAndMakeVisible (s);
}

void AndresBeatsClipAudioProcessorEditor::resized()
{
    inputGain.setBounds (69, 245, 80, 80);
    ceiling.setBounds   (242, 298, 56, 56);
    knee.setBounds      (328, 298, 56, 56);
    mojo.setBounds      (414, 298, 56, 56);
    alchemy.setBounds   (500, 298, 56, 56);
    ceilTrim.setBounds  (624, 257, 56, 56);
    dryWet.setBounds    (698, 257, 56, 56);
    air.setBounds       (389, 408, 118, 24);

    inputGainBox.setBounds (69, 348, 80, 20);
    ceilingBox.setBounds   (230, 377, 80, 21);
    kneeBox.setBounds      (316, 377, 80, 21);
    mojoBox.setBounds      (402, 377, 80, 21);
    alchemyBox.setBounds   (488, 377, 80, 21);
    ceilTrimBox.setBounds  (612, 337, 75, 20);
    dryWetBox.setBounds    (687, 337, 79, 20);
    kneeHeader.setBounds   (417, 197, 150, 18);
    airValue.setBounds     (511, 412, 44, 18);

    osSelector.setBounds   (393, 29, 98, 29);
    modeSelector.setBounds (504, 29, 175, 29);
    activeButton.setBounds (691, 30, 87, 27);

    grMeter.setBounds  (231, 233, 336, 52);
    inMeter.setBounds  (35, 392, 148, 45);
    outMeter.setBounds (616, 392, 146, 45);
}

void AndresBeatsClipAudioProcessorEditor::timerCallback()
{
    auto follow = [] (float& shown, float fresh) { shown = juce::jmax (fresh, shown * 0.80f); };

    follow (dispInL,  proc.inPeakL.exchange (0.0f));
    follow (dispInR,  proc.inPeakR.exchange (0.0f));
    follow (dispOutL, proc.outPeakL.exchange (0.0f));
    follow (dispOutR, proc.outPeakR.exchange (0.0f));

    dispGr = juce::jmin (proc.grDb.exchange (0.0f), dispGr * 0.88f);

    inMeter.setLevels (dispInL, dispInR);
    outMeter.setLevels (dispOutL, dispOutR);
    grMeter.setGainReduction (dispGr);
}

void AndresBeatsClipAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);

    g.setGradientFill (juce::ColourGradient (orange.withAlpha (0.05f), 400.0f, 0.0f,
                                             juce::Colours::transparentBlack, 400.0f, 260.0f, false));
    g.fillRect (0, 0, kW, 260);

    // ---- Cabecera ----
    juce::Rectangle<float> logo (20.0f, 26.0f, 36.0f, 36.0f);
    g.setGradientFill (juce::ColourGradient (gold, logo.getX(), logo.getY(),
                                             juce::Colour (0xffea580c), logo.getRight(), logo.getBottom(), false));
    g.fillRoundedRectangle (logo, 9.0f);

    juce::Path bolt;
    bolt.startNewSubPath (0.60f, 0.10f);
    bolt.lineTo (0.27f, 0.55f);
    bolt.lineTo (0.47f, 0.55f);
    bolt.lineTo (0.40f, 0.90f);
    bolt.lineTo (0.73f, 0.43f);
    bolt.lineTo (0.53f, 0.43f);
    bolt.closeSubPath();
    g.setColour (dark);
    g.fillPath (bolt, juce::AffineTransform::scale (logo.getWidth(), logo.getHeight()).translated (logo.getX(), logo.getY()));

    g.setColour (gold);
    g.setFont (ui (22.0f, true).withExtraKerningFactor (0.07f));
    g.drawText ("ANDRESBEATS CLIP", 68, 24, 215, 26, juce::Justification::centredLeft, false);

    juce::Rectangle<float> badge (281.0f, 26.0f, 63.0f, 19.0f);
    g.setColour (orange.withAlpha (0.12f));
    g.fillRoundedRectangle (badge, 5.0f);
    g.setColour (orange.withAlpha (0.9f));
    g.drawRoundedRectangle (badge, 5.0f, 1.0f);
    g.setFont (ui (10.0f, true));
    g.drawText ("GOLD PRO", badge, juce::Justification::centred, false);

    g.setColour (juce::Colour (0xff5f6b86));
    g.setFont (ui (10.5f));
    g.drawText ("Ultra-Clean Peak Clipper & Harmonic Chemistry", 68, 51, 320, 14, juce::Justification::centredLeft, false);

    g.setColour (juce::Colour (0xff1e2027));
    g.drawHorizontalLine (77, 20.0f, 778.0f);

    // ---- Paneles ----
    drawPanel (g, { 20.0f, 182.0f, 177.0f, 269.0f }, false);
    drawPanel (g, { 214.0f, 182.0f, 370.0f, 269.0f }, true);
    drawPanel (g, { 601.0f, 182.0f, 177.0f, 269.0f }, false);

    // INPUT STAGE
    g.setColour (text);
    g.setFont (ui (12.0f, true).withExtraKerningFactor (0.05f));
    g.drawText ("INPUT STAGE", 35, 196, 130, 18, juce::Justification::centredLeft, false);
    drawPulse (g, 167.0f, 198.0f);
    g.setColour (juce::Colour (0xff1e2027));
    g.drawHorizontalLine (222, 35.0f, 183.0f);
    drawLabel (g, "INPUT GAIN", 109.0f, 336.0f);

    // OUTPUT STAGE
    g.setColour (text);
    g.setFont (ui (12.0f, true).withExtraKerningFactor (0.05f));
    g.drawText ("OUTPUT STAGE", 616, 196, 130, 18, juce::Justification::centredLeft, false);
    drawPulse (g, 745.0f, 198.0f);
    g.setColour (juce::Colour (0xff1e2027));
    g.drawHorizontalLine (222, 616.0f, 762.0f);
    drawLabel (g, "CEIL TRIM", 652.0f, 324.0f);
    drawLabel (g, "DRY / WET", 726.0f, 324.0f);

    // GOLD CLIPPER CORE
    g.setColour (gold);
    juce::Path star1, star2;
    star1.addStar ({ 238.0f, 208.0f }, 4, 2.0f, 7.0f, 0.0f);
    star2.addStar ({ 245.0f, 200.0f }, 4, 1.2f, 3.8f, 0.0f);
    g.fillPath (star1);
    g.fillPath (star2);

    g.setFont (ui (12.0f, true).withExtraKerningFactor (0.05f));
    g.drawText ("GOLD CLIPPER CORE", 252, 197, 170, 18, juce::Justification::centredLeft, false);

    g.setGradientFill (juce::ColourGradient (orange.withAlpha (0.55f), 231.0f, 0.0f,
                                             orange.withAlpha (0.08f), 567.0f, 0.0f, false));
    g.fillRect (231.0f, 224.5f, 336.0f, 1.0f);

    drawLabel (g, "CEILING",    270.0f, 365.0f);
    drawLabel (g, "KNEE SOFT",  356.0f, 365.0f);
    drawLabel (g, "GOLD MOJO",  442.0f, 365.0f);
    drawLabel (g, "ALCHEMY",    528.0f, 365.0f);

    // Fila HIGH AIR SHIMMER
    juce::Rectangle<float> airRow (231.0f, 406.0f, 336.0f, 28.0f);
    g.setColour (juce::Colour (0xff0e0f14));
    g.fillRoundedRectangle (airRow, 8.0f);
    g.setColour (fieldEdge);
    g.drawRoundedRectangle (airRow.reduced (0.5f), 8.0f, 1.0f);

    g.setColour (orange);
    for (int i = 0; i < 3; ++i)
    {
        const float x = 247.0f + 5.5f * (float) i;
        g.fillRect (x, 413.0f, 1.3f, 14.0f);
        g.fillRect (x - 1.8f, 416.0f + 3.0f * (float) ((i + 1) % 3), 4.9f, 2.4f);
    }

    g.setColour (juce::Colours::white);
    g.setFont (ui (11.0f, true).withExtraKerningFactor (0.04f));
    g.drawText ("HIGH AIR SHIMMER", 265, 412, 130, 16, juce::Justification::centredLeft, false);

    // ---- Pie ----
    g.setColour (juce::Colour (0xff1b1d24));
    g.drawHorizontalLine (556, 20.0f, 778.0f);

    const auto modeText = proc.apvts.getParameter (IDs::mode)->getCurrentValueAsText();
    const auto osText   = proc.apvts.getParameter (IDs::os)->getCurrentValueAsText();

    g.setColour (dim);
    g.setFont (mono (9.5f));
    g.drawText ("MODE: " + modeText + "   \xe2\x80\xa2   OVERSAMPLING: " + osText + "   \xe2\x80\xa2   HQ LINEAR PHASE ALGORITHM",
                20, 563, 520, 16, juce::Justification::centredLeft, false);

    g.setColour (juce::Colour (0xffd97706));
    g.setFont (mono (9.5f, true).withExtraKerningFactor (0.05f));
    g.drawText ("ANDRESBEATS MASTERING DSP", 478, 563, 300, 16, juce::Justification::centredRight, false);
}
