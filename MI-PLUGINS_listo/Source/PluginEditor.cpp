#include "PluginEditor.h"

LoudnessAnalyzerAudioProcessorEditor::LoudnessAnalyzerAudioProcessorEditor(
    LoudnessAnalyzerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(520, 300);
    titleLabel.setText("LOUDNESS ANALYZER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(juce::FontOptions(25.0f, juce::Font::bold)));
    addAndMakeVisible(titleLabel);

    momentaryLabel.setJustificationType(juce::Justification::centred);
    momentaryLabel.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    addAndMakeVisible(momentaryLabel);

    shortTermLabel.setJustificationType(juce::Justification::centred);
    shortTermLabel.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    addAndMakeVisible(shortTermLabel);

    statusLabel.setText("Analyzer only • prototype DSP • no audio processing", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setFont(juce::Font(juce::FontOptions(12.0f)));
    addAndMakeVisible(statusLabel);

    resetButton.onClick = [this] { audioProcessor.resetMeters(); };
    addAndMakeVisible(resetButton);
    startTimerHz(15);
}

void LoudnessAnalyzerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff101318));
    g.setColour(juce::Colour(0xff242a33));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(12.0f), 14.0f);
    g.setColour(juce::Colour(0xff75e0b0));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(12.0f), 14.0f, 1.5f);
    g.setColour(juce::Colour(0xff75e0b0));
    g.drawLine(28.0f, 67.0f, 492.0f, 67.0f, 1.0f);
}

void LoudnessAnalyzerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(28);
    titleLabel.setBounds(area.removeFromTop(42));
    area.removeFromTop(18);
    auto row = area.removeFromTop(105);
    momentaryLabel.setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(4));
    shortTermLabel.setBounds(row.reduced(4));
    resetButton.setBounds(area.removeFromTop(38).removeFromLeft(110));
    statusLabel.setBounds(area.removeFromTop(28));
}

void LoudnessAnalyzerAudioProcessorEditor::timerCallback()
{
    momentaryLabel.setText("M  " + juce::String(audioProcessor.getMomentaryLUFS(), 1) + " LUFS",
                           juce::dontSendNotification);
    shortTermLabel.setText("S  " + juce::String(audioProcessor.getShortTermLUFS(), 1) + " LUFS",
                           juce::dontSendNotification);
}

