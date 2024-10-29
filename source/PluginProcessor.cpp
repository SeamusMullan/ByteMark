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
                       ),
    apvts(*this, nullptr, "Parameters", createParameterLayout()),
    paramManager(apvts)
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

    lowMidCrossover.prepare(spec);
    midHighCrossover.prepare(spec);
    bassMonoFilter.prepare (spec);
    haasDelay.prepare(spec);
    haasDelay.setMaximumDelayInSamples (sampleRate);
    chorus.prepare(spec);
    convolution.prepare(spec);
    compressor.prepare(spec);
    lowGain.prepare(spec);
    midGain.prepare(spec);
    highGain.prepare(spec);

    // Reset DSP modules
    lowMidCrossover.reset();
    midHighCrossover.reset();
    bassMonoFilter.reset();
    haasDelay.reset();
    chorus.reset();
    convolution.reset();
    compressor.reset();
    lowGain.reset();
    midGain.reset();
    highGain.reset();

    lowMidCrossover.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    midHighCrossover.setType (juce::dsp::LinkwitzRileyFilterType::highpass);

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

    // Update parameters
    paramManager.updateParameters();
    paramManager.updateEffectParameters (compressor, lowMidCrossover, midHighCrossover,
                                         lowGain, midGain, highGain, haasDelay, chorus, convolution);

    // Check for bypass
    if (paramManager.isBypassed())
    {
        // Bypass processing
        return;
    }

    // Apply input gain
    buffer.applyGain (juce::Decibels::decibelsToGain (paramManager.getInGain()));

    // Prepare the main audio block
    juce::dsp::AudioBlock<float> mainBlock (buffer);

    // Create separate buffers for low, mid, and high bands
    juce::AudioBuffer<float> lowBuffer, midBuffer, highBuffer;
    lowBuffer.setSize (buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);
    midBuffer.setSize (buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);
    highBuffer.setSize (buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);

    // Copy the input buffer to the band buffers
    lowBuffer.makeCopyOf (buffer);
    midBuffer.makeCopyOf (buffer);
    highBuffer.makeCopyOf (buffer);

    // Create audio blocks for each band
    juce::dsp::AudioBlock<float> lowBlock (lowBuffer);
    juce::dsp::AudioBlock<float> midBlock (midBuffer);
    juce::dsp::AudioBlock<float> highBlock (highBuffer);

    juce::dsp::ProcessContextReplacing<float> lowContext (lowBlock);
    juce::dsp::ProcessContextReplacing<float> midContext (midBlock);
    juce::dsp::ProcessContextReplacing<float> highContext (highBlock);

    // Set crossover frequencies from parameters
    float lowMidFreq = apvts.getRawParameterValue ("LOW_MID_FREQ")->load();
    float midHighFreq = apvts.getRawParameterValue ("MID_HIGH_FREQ")->load();

    lowMidCrossover.setCutoffFrequency (lowMidFreq);  // Low-pass filter for low band
    midHighCrossover.setCutoffFrequency (midHighFreq); // High-pass filter for high band

    // Process each band separately
    lowMidCrossover.process (lowContext); // Low-pass filter for low band
    midHighCrossover.process (highContext); // High-pass filter for high band

    // Process mid band (band-pass by subtracting low and high from the original)
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* originalData = buffer.getReadPointer (channel);
        auto* lowData = lowBuffer.getReadPointer (channel);
        auto* highData = highBuffer.getReadPointer (channel);
        auto* midData = midBuffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            midData[sample] = originalData[sample] - lowData[sample] - highData[sample];
        }
    }

    // Apply gains to each band
    lowGain.setGainDecibels (apvts.getRawParameterValue ("LOW_GAIN")->load());
    midGain.setGainDecibels (apvts.getRawParameterValue ("MID_GAIN")->load());
    highGain.setGainDecibels (apvts.getRawParameterValue ("HIGH_GAIN")->load());

    lowGain.process (lowContext);
    midGain.process (midContext);
    highGain.process (highContext);

    // Apply solo/mute logic
    bool lowSolo = apvts.getRawParameterValue ("LOW_SOLO")->load();
    bool midSolo = apvts.getRawParameterValue ("MID_SOLO")->load();
    bool highSolo = apvts.getRawParameterValue ("HIGH_SOLO")->load();

    float low = (lowSolo || (!lowSolo && !midSolo && !highSolo)) ? 1.0f : 0.0f;
    float mid = (midSolo || (!lowSolo && !midSolo && !highSolo)) ? 1.0f : 0.0f;
    float high = (highSolo || (!lowSolo && !midSolo && !highSolo)) ? 1.0f : 0.0f;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* lowData = lowBuffer.getWritePointer (channel);
        auto* midData = midBuffer.getWritePointer (channel);
        auto* highData = highBuffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            lowData[sample] *= low;
            midData[sample] *= mid;
            highData[sample] *= high;
        }
    }

    // Recombine the bands into the main buffer
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* mainData = buffer.getWritePointer (channel);
        auto* lowData = lowBuffer.getReadPointer (channel);
        auto* midData = midBuffer.getReadPointer (channel);
        auto* highData = highBuffer.getReadPointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            mainData[sample] = lowData[sample] + midData[sample] + highData[sample];
        }
    }



    // Apply Haas Delay
    // Retrieve the Haas delay time in samples
    float haasDelayTimeMs = apvts.getRawParameterValue ("HAAS_TIME")->load();
    float haasDelaySamples = haasDelayTimeMs * static_cast<float>( (getSampleRate() / 1000.0));
    haasDelay.setDelay (haasDelaySamples);

    // Get the mix parameter (0.0 to 1.0)
    float haasMix = apvts.getRawParameterValue ("HAAS_MIX")->load() / 100.0f;

    // Apply delay to the right channel
    if (buffer.getNumChannels() > 1)
    {
        auto* rightChannelData = buffer.getWritePointer (1);

        // Create an array of pointers for the AudioBlock constructor
        float* channelDataArray[] = { rightChannelData };

        juce::dsp::AudioBlock<float> rightBlock (channelDataArray, 1, static_cast<size_t> (buffer.getNumSamples()));
        juce::dsp::ProcessContextReplacing<float> rightContext (rightBlock);

        // Process the delay
        //haasDelay.process (rightContext);

        // Mix dry and wet signals
        auto* originalRightChannelData = buffer.getReadPointer (1);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            rightChannelData[sample] = (1.0f - haasMix) * originalRightChannelData[sample] + haasMix * rightBlock.getSample (0, sample);
        }
    }

    // Apply Chorus
    // chorus.process (juce::dsp::ProcessContextReplacing<float> (mainBlock));

    // Apply Convolution
    // convolution.process (juce::dsp::ProcessContextReplacing<float> (mainBlock));

    // Apply Compressor
    compressor.process (juce::dsp::ProcessContextReplacing<float> (mainBlock));

    // Apply output gain
    buffer.applyGain (juce::Decibels::decibelsToGain (paramManager.getOutGain()));

    // Now push the processed buffer into the FIFO
    fifoQueue.push (buffer);
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
