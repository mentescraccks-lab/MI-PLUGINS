#include "PluginEditor.h"
#include <cmath>

namespace {
const juce::Colour bg (0xff090a0d), panel (0xff0d0e12), border (0xff292a30),
                    gold (0xffffad00), text (0xfff4f0e7), muted (0xff858998);
}

void AndresBeatsAnalogLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                                       float position, float startAngle, float endAngle,
                                                       juce::Slider& slider)
{
    juce::ignoreUnused (slider);
    const float side = (float) juce::jmin (width, height) - 12.0f;
    const float cx = x + width * 0.5f;
    const float cy = y + height * 0.5f;
    const float radius = side * 0.5f;
    const float angle = startAngle + position * (endAngle - startAngle);

    // Soft drop shadow makes the control sit above the panel.
    g.setColour (juce::Colours::black.withAlpha (0.78f));
    g.fillEllipse (cx - radius + 1.5f, cy - radius + 4.0f, side, side);

    // Gold scale ring and fine calibration marks.
    juce::Path arc;
    arc.addCentredArc (cx, cy, radius + 2.0f, radius + 2.0f, 0.0f,
                       startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff5b421b));
    g.strokePath (arc, juce::PathStrokeType (2.0f));
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (endAngle - startAngle) * (float) i / 10.0f;
        const float inner = radius + 4.0f;
        const float outer = inner + (i % 5 == 0 ? 5.0f : 3.0f);
        juce::Line<float> tick (cx + std::cos (a) * inner, cy + std::sin (a) * inner,
                                cx + std::cos (a) * outer, cy + std::sin (a) * outer);
        g.setColour (i <= (int) std::round (position * 10.0f) ? juce::Colour (0xffffbd45) : juce::Colour (0xff55504a));
        g.drawLine (tick, i % 5 == 0 ? 1.7f : 1.0f);
    }

    // Knurled outer grip: dark nickel ring with short radial grooves.
    juce::Rectangle<float> body (cx - radius, cy - radius, side, side);
    juce::ColourGradient rim (juce::Colour (0xff666a70), cx - radius, cy - radius,
                              juce::Colour (0xff17191d), cx + radius, cy + radius, false);
    g.setGradientFill (rim);
    g.fillEllipse (body);
    g.setColour (juce::Colour (0xff85888d));
    g.drawEllipse (body, 1.0f);
    for (int i = 0; i < 32; ++i)
    {
        const float a = juce::MathConstants<float>::twoPi * (float) i / 32.0f;
        const float r1 = radius - 2.0f;
        const float r2 = radius - 5.0f;
        g.setColour (i % 2 == 0 ? juce::Colour (0xff090a0c) : juce::Colour (0xff9b9da0).withAlpha (0.7f));
        g.drawLine (cx + std::cos (a) * r1, cy + std::sin (a) * r1,
                    cx + std::cos (a) * r2, cy + std::sin (a) * r2, 1.0f);
    }

    // Satin-brushed face, with a subtle highlight and inset edge.
    const float faceRadius = radius * 0.76f;
    juce::Rectangle<float> face (cx - faceRadius, cy - faceRadius, faceRadius * 2.0f, faceRadius * 2.0f);
    juce::ColourGradient faceGradient (juce::Colour (0xff777b80), cx - faceRadius * 0.6f, cy - faceRadius,
                                      juce::Colour (0xff25282d), cx + faceRadius, cy + faceRadius, false);
    g.setGradientFill (faceGradient);
    g.fillEllipse (face);
    g.setColour (juce::Colour (0xffaeb0b2).withAlpha (0.65f));
    g.drawEllipse (face, 1.0f);
    for (int i = -5; i <= 5; ++i)
    {
        const float yy = cy + (float) i * faceRadius * 0.12f;
        const float half = std::sqrt (juce::jmax (0.0f, faceRadius * faceRadius - (yy - cy) * (yy - cy)));
        g.setColour (juce::Colours::white.withAlpha (i % 2 == 0 ? 0.035f : 0.018f));
        g.drawLine (cx - half, yy, cx + half, yy, 1.0f);
    }

    // Gold pointer and a tiny central hub.
    const float pointerLength = faceRadius * 0.76f;
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.8f, -faceRadius * 0.82f, 3.6f, pointerLength, 1.5f);
    g.setColour (juce::Colour (0xff08090b));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (cx + 1.0f, cy + 1.0f));
    g.setColour (juce::Colour (0xffffc34f));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (cx, cy));
    g.setColour (juce::Colour (0xffc18a24));
    g.fillEllipse (cx - 4.0f, cy - 4.0f, 8.0f, 8.0f);
    g.setColour (juce::Colour (0xffffd779));
    g.fillEllipse (cx - 2.0f, cy - 2.0f, 4.0f, 4.0f);
}

AndresBeatsClipAudioProcessorEditor::AndresBeatsClipAudioProcessorEditor (AndresBeatsClipAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1000, 560);
    auto label = [this](juce::Label& l, const juce::String& t, int s, juce::Colour c, bool b=false) {
        setupLabel(l,t,s,c,b); addAndMakeVisible(l);
    };
    label(title, "ANDRESBEATS CLIP", 24, gold, true);
    label(subtitle, "Ultra-Clean Peak Clipper  &  Harmonic Chemistry", 11, muted);
    label(inputTitle, "INPUT STAGE", 12, muted, true);
    label(outputTitle, "OUTPUT STAGE", 12, muted, true);
    label(coreTitle, "✦  GOLD CLIPPER CORE", 13, gold, true);
    label(footer, "MODE: MODERN    •    OVERSAMPLING: 2x    •    ANDRESBEATS MASTERING DSP", 10, muted);
    label(inputValue, "0.0 dB", 11, gold, true);
    label(ceilingValue, "0.0 dB", 11, gold, true);
    label(kneeValue, "15.0 %", 11, gold, true);
    label(mojoValue, "25.0 %", 11, gold, true);
    label(alchemyValue, "10.0 %", 11, gold, true);
    label(trimValue, "0.0 dB", 11, gold, true);
    label(mixValue, "100.0 %", 11, gold, true);
    label(airValue, "15.0 %", 11, gold, true);
    label(inputMeterLabel, "IN L                 IN R", 9, muted);
    label(outputMeterLabel, "OUT L               OUT R", 9, muted);
    label(grLabel, "CLIP ACTIVITY & GAIN REDUCTION", 10, muted, true);

    setupKnob(input, "Input Gain"); setupKnob(ceiling, "Ceiling");
    setupKnob(knee, "Knee Soft"); setupKnob(mojo, "Gold Mojo"); setupKnob(alchemy, "Alchemy");
    setupKnob(trim, "Ceil Trim"); setupKnob(mix, "Dry / Wet"); setupKnob(air, "High Air Shimmer", false);
    auto& apvts = audioProcessor.parameters;
    inputAttach = std::make_unique<Attach>(apvts,"input",input);
    ceilingAttach = std::make_unique<Attach>(apvts,"ceiling",ceiling);
    kneeAttach = std::make_unique<Attach>(apvts,"knee",knee);
    mojoAttach = std::make_unique<Attach>(apvts,"mojo",mojo);
    alchemyAttach = std::make_unique<Attach>(apvts,"alchemy",alchemy);
    trimAttach = std::make_unique<Attach>(apvts,"trim",trim);
    mixAttach = std::make_unique<Attach>(apvts,"mix",mix);
    airAttach = std::make_unique<Attach>(apvts,"air",air);

    mode.addItem("Modern",1); mode.addItem("Classic",2); mode.addItem("Hard",3);
    mode.setSelectedId(1); mode.setColour(juce::ComboBox::backgroundColourId,panel);
    mode.setColour(juce::ComboBox::textColourId,text); mode.setColour(juce::ComboBox::outlineColourId,border);
    mode.onChange = [this] { audioProcessor.parameters.getParameter("mode")->setValueNotifyingHost((float)(mode.getSelectedId()-1)/2.0f); };
    addAndMakeVisible(mode);
    oversampling.addItem("1x",1); oversampling.addItem("2x",2); oversampling.addItem("4x",3);
    oversampling.setSelectedId(2); oversampling.setColour(juce::ComboBox::backgroundColourId,panel);
    oversampling.setColour(juce::ComboBox::textColourId,text); oversampling.setColour(juce::ComboBox::outlineColourId,border);
    oversampling.onChange = [this] { audioProcessor.parameters.getParameter("oversampling")->setValueNotifyingHost((float)(oversampling.getSelectedId()-1)/2.0f); };
    addAndMakeVisible(oversampling);
    active.setButtonText("ACTIVE"); active.setToggleState(true,juce::dontSendNotification);
    active.setColour(juce::ToggleButton::textColourId,text); active.setColour(juce::ToggleButton::tickColourId,gold);
    active.onClick = [this] { audioProcessor.parameters.getParameter("active")->setValueNotifyingHost(active.getToggleState()?1.0f:0.0f); };
    addAndMakeVisible(active);
    startTimerHz(30);
}

void AndresBeatsClipAudioProcessorEditor::setupLabel(juce::Label& l, const juce::String& t, int size, juce::Colour c, bool bold)
{
    l.setText(t,juce::dontSendNotification);
    l.setFont(juce::Font((float)size, bold ? juce::Font::bold : juce::Font::plain));
    l.setColour(juce::Label::textColourId,c);
}
void AndresBeatsClipAudioProcessorEditor::setupKnob(Slider& s,const juce::String& name,bool rotary)
{
    s.setName(name);
    if (rotary) s.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    else s.setSliderStyle(Slider::LinearHorizontal);
    s.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    s.setColour(Slider::rotarySliderFillColourId,gold);
    s.setColour(Slider::rotarySliderOutlineColourId,border);
    s.setColour(Slider::thumbColourId,gold);
    s.setColour(Slider::trackColourId,border);
    if (rotary)
        s.setLookAndFeel (&analogLookAndFeel);
    addAndMakeVisible(s);
}

AndresBeatsClipAudioProcessorEditor::~AndresBeatsClipAudioProcessorEditor()
{
    for (auto* slider : { &input, &ceiling, &knee, &mojo, &alchemy, &trim, &mix, &air })
        slider->setLookAndFeel (nullptr);
}

void AndresBeatsClipAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(gold.withAlpha(0.85f)); g.fillRoundedRectangle(24.0f,25.0f,6.0f,42.0f,3.0f);
    g.setColour(border); g.drawLine(24.0f,92.0f,976.0f,92.0f,1.0f);
    auto panelBox=[&g](juce::Rectangle<float> r, bool core=false) {
        g.setColour(core ? juce::Colour(0xff15120e) : panel); g.fillRoundedRectangle(r,14.0f);
        g.setColour(core ? juce::Colour(0xff684000) : border); g.drawRoundedRectangle(r,14.0f,1.0f);
    };
    panelBox({24,170,222,338}); panelBox({266,170,468,338},true); panelBox({754,170,222,338});
    // Subtle brushed faceplate grain and small panel fasteners for a rack-unit feel.
    g.saveState();
    g.reduceClipRegion (juce::Rectangle<int> (25, 171, 950, 336));
    for (int yy = 176; yy < 506; yy += 5)
    {
        g.setColour (juce::Colours::white.withAlpha (yy % 10 == 1 ? 0.018f : 0.007f));
        g.drawLine (28.0f, (float) yy, 972.0f, (float) yy, 1.0f);
    }
    g.restoreState();
    auto screw = [&g](float sx, float sy) {
        g.setColour (juce::Colour (0xff050607)); g.fillEllipse (sx, sy, 8.0f, 8.0f);
        g.setColour (juce::Colour (0xff6c6f73)); g.drawEllipse (sx, sy, 8.0f, 8.0f, 1.0f);
        g.drawLine (sx + 2.0f, sy + 6.0f, sx + 6.0f, sy + 2.0f, 1.0f);
    };
    screw (31, 178); screw (231, 178); screw (273, 178); screw (721, 178); screw (761, 178); screw (961, 178);
    g.setColour(border); g.drawLine(42,220,228,220,1); g.drawLine(782,220,958,220,1);
    g.setColour(juce::Colour(0xff1a1713)); g.fillRoundedRectangle(286,234,428,64,9);
    g.setColour(border); g.drawRoundedRectangle(286,234,428,64,9,1);
    g.setColour(gold); g.setFont(juce::Font(11.0f,juce::Font::bold));
    g.drawText(juce::String(juce::Decibels::gainToDecibels(juce::jmax(0.0001f,audioProcessor.gainReduction.load())),1)+" dB",620,245,75,16,juce::Justification::centredRight);
    drawMeter(g,{298,276,400,7},juce::jlimit(0.0f,1.0f,grDb/12.0f),"");
    g.setColour(border); g.drawLine(24,530,976,530,1);
    g.setColour(gold); g.setFont(juce::Font(10.0f,juce::Font::bold)); g.drawText("ANDRESBEATS MASTERING DSP",740,538,236,14,juce::Justification::centredRight);
    g.setColour(muted); g.setFont(juce::Font(10.0f)); g.drawText("INPUT GAIN",68,350,130,18,juce::Justification::centred);
    g.drawText("CEILING",286,390,78,16,juce::Justification::centred);
    g.drawText("KNEE SOFT",376,390,78,16,juce::Justification::centred);
    g.drawText("GOLD MOJO",466,390,78,16,juce::Justification::centred);
    g.drawText("ALCHEMY",556,390,78,16,juce::Justification::centred);
    g.drawText("CEIL TRIM",774,350,78,16,juce::Justification::centred);
    g.drawText("DRY / WET",864,350,78,16,juce::Justification::centred);
    g.drawText("HIGH AIR SHIMMER",294,466,180,16,juce::Justification::centredLeft);
    drawMeter(g,{44,470,182,12},juce::jlimit(0.0f,1.0f,(inDb+60.0f)/60.0f),"");
    drawMeter(g,{774,470,182,12},juce::jlimit(0.0f,1.0f,(outDb+60.0f)/60.0f),"");
}
void AndresBeatsClipAudioProcessorEditor::drawMeter(juce::Graphics& g,juce::Rectangle<float> r,float amount,const juce::String&)
{
    g.setColour(juce::Colour(0xff24252a)); g.fillRoundedRectangle(r,4.0f);
    auto fill=r.withWidth(r.getWidth()*amount);
    g.setColour(gold); g.fillRoundedRectangle(fill,4.0f);
}
void AndresBeatsClipAudioProcessorEditor::resized()
{
    title.setBounds(40,20,300,32); subtitle.setBounds(40,53,360,20);
    mode.setBounds(640,38,220,34); oversampling.setBounds(500,38,104,34); active.setBounds(866,38,102,34);
    inputTitle.setBounds(42,188,160,22); outputTitle.setBounds(782,188,160,22);
    coreTitle.setBounds(288,188,240,24); footer.setBounds(24,538,690,16);
    input.setBounds(74,230,122,112); inputValue.setBounds(78,344,114,25);
    ceiling.setBounds(292,316,68,68); knee.setBounds(382,316,68,68); mojo.setBounds(472,316,68,68); alchemy.setBounds(562,316,68,68);
    ceilingValue.setBounds(286,392,80,24); kneeValue.setBounds(376,392,80,24); mojoValue.setBounds(466,392,80,24); alchemyValue.setBounds(556,392,80,24);
    trim.setBounds(780,232,68,68); mix.setBounds(870,232,68,68);
    trimValue.setBounds(774,302,80,24); mixValue.setBounds(864,302,80,24);
    air.setBounds(500,462,150,24); airValue.setBounds(648,462,60,24);
    inputMeterLabel.setBounds(44,442,180,18); outputMeterLabel.setBounds(774,442,180,18);
    grLabel.setBounds(298,242,270,18);
}
void AndresBeatsClipAudioProcessorEditor::timerCallback()
{
    inDb=juce::Decibels::gainToDecibels(juce::jmax(0.00001f,audioProcessor.inputPeak.load()));
    outDb=juce::Decibels::gainToDecibels(juce::jmax(0.00001f,audioProcessor.outputPeak.load()));
    grDb=juce::Decibels::gainToDecibels(juce::jmax(0.00001f,audioProcessor.gainReduction.load()));
    inputValue.setText(juce::String(input.getValue(),1)+" dB",juce::dontSendNotification);
    ceilingValue.setText(juce::String(ceiling.getValue(),1)+" dB",juce::dontSendNotification);
    kneeValue.setText(juce::String(knee.getValue(),1)+" %",juce::dontSendNotification);
    mojoValue.setText(juce::String(mojo.getValue(),1)+" %",juce::dontSendNotification);
    alchemyValue.setText(juce::String(alchemy.getValue(),1)+" %",juce::dontSendNotification);
    trimValue.setText(juce::String(trim.getValue(),1)+" dB",juce::dontSendNotification);
    mixValue.setText(juce::String(mix.getValue(),1)+" %",juce::dontSendNotification);
    airValue.setText(juce::String(air.getValue(),1)+" %",juce::dontSendNotification);
    repaint();
}
