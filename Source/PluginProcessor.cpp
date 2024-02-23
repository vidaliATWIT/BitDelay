/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BitDelayAudioProcessor::BitDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    time = new Echo_Parameter();
    time->defaultValue = 1.0f;
    time->currentValue = 1.7f;
    time->name = "Time";
    addParameter(time);

    volume = new Echo_Parameter();
    volume->defaultValue = 0.1f;
    volume->currentValue = 0.7f;
    volume->name = "Echo Volume";
    addParameter(volume);

    regen = new Echo_Parameter();
    regen->defaultValue = 0.1f;
    regen->currentValue = 0.7f;;
    regen->name = "Regen";
    addParameter(regen);

    dry = new Echo_Parameter();
    dry->defaultValue = 0.0f;
    dry->currentValue = 0.7f;;
    dry->name = "Dry Volume";
    addParameter(dry);

    wet = new Echo_Parameter();
    wet->defaultValue = 0.0f;
    wet->currentValue = 0.7f;;
    wet->name = "Wet Volume";
    addParameter(wet);

}

BitDelayAudioProcessor::~BitDelayAudioProcessor()
{
}

//==============================================================================
const juce::String BitDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BitDelayAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool BitDelayAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool BitDelayAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double BitDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BitDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BitDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BitDelayAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String BitDelayAudioProcessor::getProgramName(int index)
{
    return {};
}

void BitDelayAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void BitDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    auto delayBufferSize = 2.0f * sampleRate; //Our buffer is the size of 2 seconds worth of audio, for a 2 second delay
    mDelayBuffer.setSize(getTotalNumInputChannels(), (int)delayBufferSize);
    mDelayBuffer.clear();
}

void BitDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BitDelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void BitDelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int bitDepth = 8; //8 Bit delay
    float rateDivide = derivateSampleRate(getSampleRate());

    juce::AudioBuffer<float> wetBuffer;
    wetBuffer.makeCopyOf(buffer);
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);



    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    const int bufferLength = buffer.getNumSamples();
    const int delayBufferLength = mDelayBuffer.getNumSamples();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* originalBufferData = buffer.getWritePointer(channel);
        auto* bufferData = wetBuffer.getWritePointer(channel);
        auto* dryBufferData = dryBuffer.getWritePointer(channel);

        //wetBuffer.applyGainRamp(channel, 0, bufferLength, lastInputGain, volume->getValue());
        //lastInputGain = volume->getValue();

        fillBuffer(channel, bufferLength, delayBufferLength, bufferData);
        for (int i = 0; i < bufferLength; i++)
        {
            decimate(bufferData, bitDepth, rateDivide, i);
            decimate(originalBufferData, bitDepth, rateDivide, i);
        }
        readFromBuffer(channel, bufferLength, delayBufferLength, wetBuffer);

        fillBuffer(channel, bufferLength, delayBufferLength, bufferData);
        juce::FloatVectorOperations::subtract(bufferData, originalBufferData, bufferLength);

        //Add dry
        buffer.copyFromWithRamp(channel, 0, dryBufferData, bufferLength, lastDryGain, dry->getValue());
        lastDryGain = dry->getValue();
        //Add wet
        buffer.addFromWithRamp(channel, 0, bufferData, bufferLength, lastWetGain, wet->getValue());
        lastWetGain = wet->getValue();
    }

    mWritePosition += bufferLength;
    mWritePosition %= delayBufferLength;
}


//Helper methods to get correct Sample Rate
float BitDelayAudioProcessor::derivateSampleRate(double masterSampleRate)
{
    float givenSampleRate = 22050.0f - (22050.0f*time->getValue()*0.5f);
    float rateDivide = masterSampleRate / givenSampleRate;
    return rateDivide;
}

void BitDelayAudioProcessor::decimate(float* channelData, int bitDepth, int rateDivide, int i)
{
    float totalQLevels = powf(2, bitDepth);
    float val = channelData[i];
    float remainder = fmodf(val, 1 / totalQLevels);

    channelData[i] = val - remainder;

    if (rateDivide > 1)
        if (i % rateDivide != 0)
            channelData[i] = channelData[i - i % rateDivide];
}

void BitDelayAudioProcessor::fillBufferWithRamp(int channel, int bufferLength, int delayBufferLength, float* bufferData)
{
    //copy the data from main buffer to delay buffer
    if (delayBufferLength > bufferLength + mWritePosition)
    {
        mDelayBuffer.copyFromWithRamp(channel, mWritePosition, bufferData, bufferLength, lastInputGain, volume->getValue());
    }
    else
    {
        auto numSamplesToEnd = delayBufferLength - mWritePosition;
        //copy just alittle bit of whats left to fillout the end
        mDelayBuffer.copyFromWithRamp(channel, mWritePosition, bufferData, numSamplesToEnd, lastInputGain, volume->getValue());

        auto numSamplesAtStart = bufferLength - numSamplesToEnd;
        //circle back and add whats left to the beginning again
        mDelayBuffer.copyFromWithRamp(channel, 0, bufferData + numSamplesToEnd, numSamplesAtStart, lastInputGain, volume->getValue());
    }
    lastInputGain = volume->getValue();
}

void BitDelayAudioProcessor::fillBuffer(int channel, int bufferLength, int delayBufferLength, float* bufferData)
{
    //copy the data from main buffer to delay buffer
    if (delayBufferLength > bufferLength + mWritePosition)
    {
        mDelayBuffer.copyFrom(channel, mWritePosition, bufferData, bufferLength);
    }
    else
    {
        auto numSamplesToEnd = delayBufferLength - mWritePosition;
        //copy just alittle bit of whats left to fillout the end
        mDelayBuffer.copyFrom(channel, mWritePosition, bufferData, numSamplesToEnd);

        auto numSamplesAtStart = bufferLength - numSamplesToEnd;
        //circle back and add whats left to the beginning again
        mDelayBuffer.copyFrom(channel, 0, bufferData + numSamplesToEnd, numSamplesAtStart);
    }
}

//Add audio back into main buffer
void BitDelayAudioProcessor::readFromBuffer(int channel, int bufferLength, int delayBufferLength, juce::AudioBuffer<float>& buffer)
{
    //original auto readPosition = mWritePosition - getSampleRate();
    auto readPosition = mWritePosition - (getSampleRate() * time->getValue());
    if (readPosition < 0)
        readPosition += delayBufferLength;

    auto g = lastFeedbackGain;
    lastFeedbackGain = regen->getValue();
    //
    if (readPosition + bufferLength < delayBufferLength)
    {
        buffer.addFromWithRamp(channel, 0, mDelayBuffer.getReadPointer(channel, readPosition), bufferLength, g, g);
    }
    else
    {
        auto numSamplesToEnd = delayBufferLength - readPosition;
        buffer.addFromWithRamp(channel, 0, mDelayBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, g, g);

        auto numSamplesAtStart = bufferLength - numSamplesToEnd;
        buffer.addFromWithRamp(channel, numSamplesToEnd, mDelayBuffer.getReadPointer(channel, 0), numSamplesAtStart, g, g);
    }
}

//==============================================================================
bool BitDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BitDelayAudioProcessor::createEditor()
{
    return new BitDelayAudioProcessorEditor(*this);
}

//==============================================================================
void BitDelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BitDelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitDelayAudioProcessor();
}
