/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
BitDelayAudioProcessorEditor::BitDelayAudioProcessorEditor(BitDelayAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    timeSlider.setRange(0.0f, 1.7f);
    
    timeLabel.setText("Echo Time", juce::dontSendNotification);
    
    timeSlider.setSliderStyle(juce::Slider::Rotary);
    timeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false,0,0);
    //timeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    //timeSlider.setValue(1.0f);

    //Add listener to timeSlider
    timeSlider.addListener(this);

    echoVolSlider.setSliderStyle(juce::Slider::Rotary);
    echoVolSlider.setRange(0.0f, 0.7f);
    echoVolSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    echoVolLabel.setText("Input Gain", juce::dontSendNotification);
    //echoVolSlider.setValue(0.7f);

    echoVolSlider.addListener(this);

    regenSlider.setSliderStyle(juce::Slider::Rotary);
    regenSlider.setRange(0.0f, .7f);
    regenSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    regenLabel.setText("Echo Regen", juce::dontSendNotification);
    //regenSlider.setValue(0.1f);

    regenSlider.addListener(this);

    dryLabel.setText("Dry Volume", juce::dontSendNotification);
    drySlider.setRange(0.0f, 0.7f);
    drySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    
    wetLabel.setText("Wet Volume", juce::dontSendNotification);
    wetSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    wetSlider.setRange(0.0f, 0.7f);

    addAndMakeVisible(timeSlider);
    addAndMakeVisible(timeLabel);
    addAndMakeVisible(echoVolSlider);
    addAndMakeVisible(echoVolLabel);
    addAndMakeVisible(regenSlider);
    addAndMakeVisible(regenLabel);
    addAndMakeVisible(drySlider);
    addAndMakeVisible(wetSlider);
    addAndMakeVisible(dryLabel);
    addAndMakeVisible(wetLabel);

    setLookAndFeel(&newLookAndFeel);

    setSize(400, 300);
}

BitDelayAudioProcessorEditor::~BitDelayAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void BitDelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::darkgrey);//getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

}

void BitDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    timeLabel.setBounds(20, 60, 120, 20);
    timeLabel.setJustificationType(juce::Justification::centred);
    timeSlider.setBounds(20, 100, 120, 120);
    echoVolLabel.setBounds(140, 60, 120, 20);
    echoVolLabel.setJustificationType(juce::Justification::centred);
    //echoVolSlider.setBounds(100, 40, getWidth() - 110, 20);
    echoVolSlider.setBounds(140, 100, 120, 120);
    regenLabel.setBounds(260, 60, 120, 20);
    regenLabel.setJustificationType(juce::Justification::centred);
    //regenSlider.setBounds(100, 70, getWidth() - 110, 20);
    regenSlider.setBounds(260, 100, 120, 120);
    
    dryLabel.setBounds(40, 245, 80, 20);
    wetLabel.setBounds(40, 265, 80, 20);
    drySlider.setBounds(120, 245, 250, 20);
    wetSlider.setBounds(120, 265, 250, 20);
}

void BitDelayAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &timeSlider)
        processor.getParameters()[0]->setValue(slider->getValue());
    else if (slider == &echoVolSlider)
        processor.getParameters()[1]->setValue(slider->getValue());
    else if (slider == &regenSlider)
        processor.getParameters()[2]->setValue(slider->getValue());
}
