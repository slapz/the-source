/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "Voice.h"
#include "HostParam.h"
#include "StepSequencer.h"

// UI header, should be hidden behind a factory
#include <PluginEditor.h>

//==============================================================================
PluginAudioProcessor::PluginAudioProcessor()
{
    addParameter(new HostParam<Param>(osc1fine));
    addParameter(new HostParam<Param>(osc1coarse));

    addParameter(new HostParam<Param>(lfo1freq));
    addParameter(new HostParam<Param>(osc1lfo1depth));

    addParameter(new HostParam<Param>(osc1trngAmount));
    addParameter(new HostParam<Param>(osc1pulsewidth));

    addParameter(new HostParam<Param>(lpCutoff));
    addParameter(new HostParam<Param>(lpResonance));

    addParameter(new HostParam<Param>(envAttack));
    addParameter(new HostParam<Param>(envDecay));
    addParameter(new HostParam<Param>(envSustain));
    addParameter(new HostParam<Param>(envRelease));

    addParameter(new HostParam<Param>(panDir));
}

PluginAudioProcessor::~PluginAudioProcessor()
{
}

//==============================================================================
const String PluginAudioProcessor::getName() const
{
#ifdef JucePlugin_Name
    return JucePlugin_Name;
#else
    // standalone
    return "plugin";
#endif
}

const String PluginAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String PluginAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool PluginAudioProcessor::isInputChannelStereoPair (int index) const
{
    ignoreUnused(index);
    return true;
}

bool PluginAudioProcessor::isOutputChannelStereoPair (int index) const
{
    ignoreUnused(index);
    return true;
}

bool PluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double PluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginAudioProcessor::setCurrentProgram (int index)
{
    ignoreUnused(index);
}

const String PluginAudioProcessor::getProgramName (int index)
{
    ignoreUnused(index);
    return String();
}

void PluginAudioProcessor::changeProgramName (int index, const String& newName)
{
    ignoreUnused(index,newName);
}

//==============================================================================
void PluginAudioProcessor::prepareToPlay (double sRate, int samplesPerBlock)
{
    ignoreUnused(samplesPerBlock);
    synth.setCurrentPlaybackSampleRate(sRate);
    synth.clearVoices();
    for (int i = 8; --i >= 0;)
    {
        synth.addVoice(new Voice(*this, samplesPerBlock));
    }
    synth.clearSounds();
    synth.addSound(new Sound());
}

void PluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void PluginAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    updateHostInfo();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // I've added this to avoid people getting screaming feedback
    // when they first compile the plugin, but obviously you don't need to
    // this code if your algorithm already fills all the output channels.
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    runSeq(midiMessages);

    // pass these messages to the keyboard state so that it can update the component
    // to show on-screen which keys are being pressed on the physical midi keyboard.
    // This call will also add midi messages to the buffer which were generated by
    // the mouse-clicking on the on-screen keyboard.
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    // and now get the synth to process the midi events and generate its output.
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

void PluginAudioProcessor::updateHostInfo()
{
    // currentPositionInfo used for getting the bpm.
    if (AudioPlayHead* playHead = getPlayHead())
    {
        if (playHead->getCurrentPosition (positionInfo[getAudioIndex()])) {
            positionIndex.exchange(getGUIIndex());
            return;
        }
    }
    positionInfo[getAudioIndex()].resetToDefault();
}

/*
* I actually wanted to create a extra class or object of sequencer but did not succeed in doing so.
* That's why the sequencer call is here for now.
*/
void PluginAudioProcessor::runSeq(MidiBuffer & midiMessages)
{
    // get GUI params
    // TODO: if midi step note changed while holdNote, then needs noteoff message
    midiSeq[0] = static_cast<int>(SynthParams::seqStep1.get());
    midiSeq[1] = static_cast<int>(SynthParams::seqStep2.get());
    midiSeq[2] = static_cast<int>(SynthParams::seqStep3.get());
    midiSeq[3] = static_cast<int>(SynthParams::seqStep4.get());
    midiSeq[4] = static_cast<int>(SynthParams::seqStep5.get());
    midiSeq[5] = static_cast<int>(SynthParams::seqStep6.get());
    midiSeq[6] = static_cast<int>(SynthParams::seqStep7.get());
    midiSeq[7] = static_cast<int>(SynthParams::seqStep8.get());
    seqMode = static_cast<int>(SynthParams::seqMode.get());
    seqNumSteps = static_cast<int>(SynthParams::seqNumSteps.get());
    seqStepSpeed = SynthParams::seqStepSpeed.get(); // in quarterNotes
    seqNoteLength = jmin(SynthParams::seqStepLength.get(), seqStepSpeed);

    AudioPlayHead::CurrentPositionInfo hostPlayHead = positionInfo[getAudioIndex()];
    double currPos = hostPlayHead.ppqPosition;

    if (hostPlayHead.isPlaying && (seqMode == 2)) // TODO: play and stop mode button
    {    
        // create noteOn event
        if (currPos >= seqNextStep)
        {
            // stop note if could not stop before playing next note (important for seqNoteLength == seqStepSpeed)
            if (seqIsPlaying && (midiSeq[seqNote] != -1) && (seqNote != -1))
            {
                MidiMessage m = MidiMessage::noteOff(1, midiSeq[seqNote]);
                midiMessages.addEvent(m, 0);
                seqIsPlaying = false;
            }

            // calculate the right note to play
            seqNote = static_cast<int>(currPos / seqStepSpeed) % seqNumSteps; // TODO: start bug if bpm high and stepSpeed 'high'

            if(!seqIsPlaying && (midiSeq[seqNote] != -1) && (seqNote != -1))
            {
                // pulse on step 1
                float seqVelocity = 0.45f;
                if (seqNote == 0)
                {
                    seqVelocity = 0.85f;
                }

                // send midimessage into midibuffer
                MidiMessage m = MidiMessage::noteOn(1, midiSeq[seqNote], seqVelocity);
                midiMessages.addEvent(m, 0);
                stopNoteTime = currPos + seqNoteLength;
                seqIsPlaying = true;
            }
        }

        // create corresponding noteOff event to end each step
        if (currPos >= stopNoteTime)
        {
            if (seqIsPlaying && (midiSeq[seqNote] != -1))
            {
                // send midimessage into midibuffer
                MidiMessage m = MidiMessage::noteOff(1, midiSeq[seqNote]);
                midiMessages.addEvent(m, 0);
                seqIsPlaying = false;
            }
        }
    }
    else
    {
        // stop sequencer only if not already stopped
        if (seqNote != -1)
        {
            seqNextStep = 0.0;
            stopNoteTime = 0.0;
            seqIsPlaying = false;
            seqNote = -1;

            // stop all midimessages coming from sequencer
            MidiMessage m = MidiMessage::allNotesOff(1);
            midiMessages.addEvent(m, 0);
        }
    }

    // recalculate next step position in host sync mode
    if (seqMode == 2)
    {
        seqNextStep = currPos + abs(remainder(currPos, seqStepSpeed));
    }
}
//==============================================================================
bool PluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* PluginAudioProcessor::createEditor()
{
    return new PluginAudioProcessorEditor (*this);
}

//==============================================================================
void PluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    ignoreUnused(destData);
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    ignoreUnused(data,sizeInBytes);
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginAudioProcessor();
}
