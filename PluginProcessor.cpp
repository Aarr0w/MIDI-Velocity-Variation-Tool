/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
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

    addParameter(range = new juce::AudioParameterInt("range", "-range", 0,127 , 10));
    addParameter(skew = new juce::AudioParameterInt("depth", "-skew", 0,5,5));
    addParameter(baseValue = new juce::AudioParameterInt("baseValue", "-", 0,127,84));


    addParameter(base = new juce::AudioParameterChoice("base", "bBase", {"AUTO","BASE VALUE :"}, 0));
    addParameter(direction = new juce::AudioParameterChoice("direction", "-Direction", {"Up","Centred","Down"}, 0));


}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName(int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    notes.clear();                          // [1]
    time = 0;                               // [4]
    rand = 111;
    rate = static_cast<float> (sampleRate); // [5]
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

void NewProjectAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{

    // the audio buffer in a midi effect will have zero channels!
    // but we need an audio buffer to getNumSamples....so....this next line will stay commented
    //jassert(buffer.getNumChannels() == 0);                                                         // [6]

    // however we use the buffer to get timing information
    auto numSamples = buffer.getNumSamples();                                                       // [7]

    juce::MidiBuffer processedMidi;

    juce::MidiMessage m;


    for (const auto metadata : midi)                                                             
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            auto velocity = (*base)? 
                msg.getVelocity() :
                (*baseValue);

            auto a = direction->getAllValueStrings();
            auto rand = juce::Random::getSystemRandom().nextInt(*range);

            if (*skew == 5)
            {
                rand = (juce::Random::getSystemRandom().nextInt(1) == 0) ?  0 : *range;
            }
            else
            {
                for (int x = 0; x < *skew; x++)
                    rand += juce::Random::getSystemRandom().nextInt((int)*range / 5);
            }
           
            

            switch(a.indexOf(direction->getCurrentValueAsText()))
            {
                 case 0: velocity += rand;  break;

                 case 1: velocity -= (int)(*range/2); 
                         velocity += rand; break;

                 case 2: velocity -= rand; break;

                 default: velocity += 1; break;
            }

             
            processedMidi.addEvent(juce::MidiMessage::noteOn(1, msg.getNoteNumber(), velocity), msg.getTimeStamp());
        }
        else if (msg.isNoteOff())
        {
            notes.removeValue(msg.getNoteNumber());
            processedMidi.addEvent(juce::MidiMessage::noteOff(1, msg.getNoteNumber()), msg.getTimeStamp());
        }
    }


    midi.clear();                                                                                   // [10]

 
    //time = (time + numSamples) % noteDuration;                                                      // [15]

    //always use swapWith(), avoids unpredictable behavior from directly editing midi buffer
    midi.swapWith(processedMidi);
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new AarrowAudioProcessorEditor(*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream(destData, true).writeInt(*range);
    juce::MemoryOutputStream(destData, true).writeInt(*direction);
    juce::MemoryOutputStream(destData, true).writeInt(*skew);
    juce::MemoryOutputStream(destData, true).writeInt(*baseValue);

    juce::MemoryOutputStream(destData, true).writeInt(*base);


    //juce::MemoryOutputStream(destData, true).writeInt(*sync);
  

}

void NewProjectAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.


    range->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readInt());
    direction->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readInt());
    skew->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readInt());
    baseValue->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readInt());

    base->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readBool());
  

  /*  sync->setValueNotifyingHost(juce::MemoryInputStream(data, static_cast<size_t> (sizeInBytes), false).readBool());*/

  
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
