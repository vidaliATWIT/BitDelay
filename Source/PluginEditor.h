/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {

    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // fill
        //g.setColour(juce::Colours::orange);
        //g.fillEllipse(rx, ry, rw, rw);

        // outline
        g.setColour(juce::Colours::darkred);
        g.drawEllipse(rx, ry, rw, rw, 2.0f);

        juce::Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        // pointer
        g.setColour(juce::Colours::darkred);
        g.fillPath(p);

    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackWidth = juce::jmin(6.0f, slider.isHorizontal() ? (float)height * 0.25f : (float)width * 0.25f);
        
        juce::Point<float> startPoint(slider.isHorizontal() ? (float)x : (float)x + (float)width * 0.5f,
            slider.isHorizontal() ? (float)y + (float)height * 0.5f : (float)(height + y));

        juce::Point<float> endPoint(slider.isHorizontal() ? (float)(width + x) : startPoint.x,
            slider.isHorizontal() ? startPoint.y : (float)y);

        juce::Path backgroundTrack;
        backgroundTrack.startNewSubPath(startPoint);
        backgroundTrack.lineTo(endPoint);
        g.setColour(juce::Colours::black);
        g.strokePath(backgroundTrack, { trackWidth, juce::PathStrokeType::mitered, juce::PathStrokeType::square });

        juce::Path valueTrack;
        juce::Point<float> minPoint, maxPoint, thumbPoint;

        auto kx = slider.isHorizontal() ? sliderPos : ((float)x + (float)width * 0.5f);
        auto ky = slider.isHorizontal() ? ((float)y + (float)height * 0.5f) : sliderPos;

        minPoint = startPoint;
        maxPoint = { kx, ky };

        auto thumbWidth = getSliderThumbRadius(slider);

        valueTrack.startNewSubPath(minPoint);
        valueTrack.lineTo(maxPoint);
        g.setColour(juce::Colours::darkred);
        g.strokePath(valueTrack, { trackWidth, juce::PathStrokeType::mitered, juce::PathStrokeType::square });
        
        g.setColour(juce::Colours::white);
        g.fillEllipse(juce::Rectangle<float>(static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre(maxPoint));
        
        //if (slider.isBar())
            //drawLinearSliderOutline(g, x, y, width, height, style, slider);
    }


};

//==============================================================================
/**
*/
class BitDelayAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    BitDelayAudioProcessorEditor(BitDelayAudioProcessor&);
    ~BitDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;
    void BitDelayAudioProcessorEditor::retrieveParameterValues();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BitDelayAudioProcessor& audioProcessor;
    juce::Label timeLabel;
    juce::Slider timeSlider;
    juce::Label echoVolLabel;
    juce::Slider echoVolSlider;
    juce::Label regenLabel;
    juce::Slider regenSlider;

    juce::Label dryLabel;
    juce::Slider drySlider;

    juce::Label wetLabel;
    juce::Slider wetSlider;

    CustomLookAndFeel newLookAndFeel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BitDelayAudioProcessorEditor)
};
