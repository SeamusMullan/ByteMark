#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

PluginProcessor::~PluginProcessor()
{
    // destroy anything created ig
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumInputChannels();
    spec.maximumBlockSize = samplesPerBlock;

    preLP.prepare(spec);
    preHP.prepare(spec);
    haasDelay.prepare(spec);
    chorus.prepare(spec);
    convolution.prepare(spec);
    compressor.prepare(spec);

    // Reset DSP modules
    preLP.reset();
    preHP.reset();
    haasDelay.reset();
    chorus.reset();
    convolution.reset();
    compressor.reset();

    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void PluginProcessor::FifoQueue::push(const juce::AudioBuffer<float>& buffer)
{
    int start1, size1, start2, size2;
    fifo.prepareToWrite(buffer.getNumSamples(), start1, size1, start2, size2);

    if (size1 > 0)
        circularBuffer.copyFrom(0, start1, buffer, 0, 0, size1);
    if (size2 > 0)
        circularBuffer.copyFrom(0, start2, buffer, 0, size1, size2);

    if (size1 > 0)
        circularBuffer.copyFrom(1, start1, buffer, 1, 0, size1);
    if (size2 > 0)
        circularBuffer.copyFrom(1, start2, buffer, 1, size1, size2);

    fifo.finishedWrite(size1 + size2);
}

bool PluginProcessor::FifoQueue::pull(juce::AudioBuffer<float>& buffer)
{
    int start1, size1, start2, size2;
    fifo.prepareToRead(buffer.getNumSamples(), start1, size1, start2, size2);

    if (size1 + size2 < buffer.getNumSamples())
        return false;

    if (size1 > 0)
        buffer.copyFrom(0, 0, circularBuffer, 0, start1, size1);
    if (size2 > 0)
        buffer.copyFrom(0, size1, circularBuffer, 0, start2, size2);

    if (size1 > 0)
        buffer.copyFrom(1, 0, circularBuffer, 1, start1, size1);
    if (size2 > 0)
        buffer.copyFrom(1, size1, circularBuffer, 1, start2, size2);

    fifo.finishedRead(size1 + size2);
    return true;
}



void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any extra output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Apply your audio processing here
    // For example:
    // applyGain(buffer);
    // applyEffects(buffer);

    // Now push the processed buffer into the FIFO
    fifoQueue.push(buffer);
}


//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
