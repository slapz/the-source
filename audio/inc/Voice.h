#pragma once

#include "JuceHeader.h"
#include "SynthParams.h"

class Sound : public SynthesiserSound {
public:
    bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel(int /*midiChannel*/) override { return true; }
};

struct Waveforms {
    static float sinus(float phs, float trngAmount, float width)  { 
        ignoreUnused(trngAmount, width);
        return std::sin(phs); 
    }
    static float square(float phs, float trngAmount, float width) {
        ignoreUnused(trngAmount, width);
        //square wave with duty cycle
        if (phs < 2.f * float_Pi * width) {
            return 1.f;
        } else {
            return -1.f;
        }
        //return std::copysign(1.f, float_Pi - phs);
    }
    static float saw(float phs, float trngAmount, float width) {
        ignoreUnused(width);
        //return (1 - trngAmount) * phs / (float_Pi*2.f) - .5f + trngAmount * (-abs(float_Pi - phs))*(1 / float_Pi) + .5f;
        if (phs < trngAmount*float_Pi) { return (.5f - 1.f / (trngAmount*float_Pi) * phs); }
        else { return (-.5f + 1.f / (2.f*float_Pi - trngAmount*float_Pi) * (phs-trngAmount*float_Pi)); }
    }
};


template<float(*_waveform)(float, float, float)>
struct Oscillator {
    float phase;
    float phaseDelta;
    float trngAmount;
    float width;

    Oscillator() : phase(0.f), phaseDelta(0.f) {}

    void reset() {
        phase = 0.f;
        phaseDelta = 0.f;
    }

    bool isActive() const {
        return phaseDelta > 0.f;
    }

    float next() {
        const float result = _waveform(phase, trngAmount, width);
        phase = std::fmod(phase + phaseDelta, float_Pi * 2.0f);
        return result;
    }

    float next(float pitchMod) {
        const float result = _waveform(phase, trngAmount, width);
        phase = std::fmod(phase + phaseDelta*pitchMod, float_Pi * 2.0f);
        return result;
    }
};


class Voice : public SynthesiserVoice {
public:
    Voice(SynthParams &p, int blockSize) 
    :    lastSample(0.f)
    , inputDelay1(0.f)
    , inputDelay2(0.f)
    , outputDelay1(0.f)
    , outputDelay2(0.f)
    , params(p)
    , level (0.f)
    , tailOff (0.f)
    , pitchModBuffer(1,blockSize)

	{}
    


    bool canPlaySound (SynthesiserSound* sound) override
    {
        ignoreUnused(sound);
        return true;
    }

    void startNote (int midiNoteNumber, float velocity,
                    SynthesiserSound*, int currentPitchWheelPosition) override
    {
        lastSample = 0.f;
        inputDelay1 = 0.f;
        inputDelay2 = 0.f;
        outputDelay1 = 0.f;
        outputDelay2 = 0.f;
        
        level = velocity * 0.15f;
        tailOff = 0.f;

        currentPitchValue = currentPitchWheelPosition;

        const float sRate = static_cast<float>(getSampleRate());
        float freqHz = static_cast<float>(MidiMessage::getMidiNoteInHertz (midiNoteNumber, params.freq.get()));

        // change the phases of both lfo waveforms, in case the user switches them during a note
        lfo1sine.phase = 0.f;
        lfo1sine.phaseDelta = params.lfo1freq.get() / sRate * 2.f * float_Pi;
        lfo1square.phase = 0.f;
        lfo1square.phaseDelta = params.lfo1freq.get() / sRate * 2.f * float_Pi;


        osc1.phase = 0.f;
        osc1.phaseDelta = freqHz * (Param::fromCent(params.osc1fine.get()) * Param::fromSemi(params.osc1coarse.get())) / sRate * 2.f * float_Pi;
        osc1.trngAmount = params.osc1trngAmount.get();
        osc1.width = params.osc1pulsewidth.get();
        lfo1square.width = params.osc1pulsewidth.get();
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
                                // stopNote method could be called more than once.
                tailOff = 1.0;
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..
            clearCurrentNote();
            lfo1sine.reset();
            lfo1square.reset();
            osc1.reset();
        }
    }

    void pitchWheelMoved (int newValue) override
    {
        currentPitchValue = newValue;
    }

    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override
    {
        // not interested in controllers in this case.
    }

    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        renderModulation(numSamples);
        const float *pitchMod = pitchModBuffer.getReadPointer(0);

        const float currentAmp = params.vol.get();
        const float currentPan = params.panDir.get();

        // Pan Influence
        const float currentAmpRight = currentAmp + (currentAmp / 100.f * currentPan);
        const float currentAmpLeft = currentAmp - (currentAmp / 100.f * currentPan);

        if (lfo1square.isActive() || lfo1sine.isActive())
        {
            if (tailOff > 0.f)
            {
                for (int s = 0; s < numSamples; ++s)
                {
                    float currentSample;

                    if (params.passtype.get() == 0) currentSample = biquadLowpass(osc1.next(pitchMod[s])) * level * tailOff;
                    else currentSample = biquadHighpass(osc1.next(pitchMod[s])) * level * tailOff;
                    // TODO bandpass!
       
                    

                    //check if the output is a stereo output
                    if (outputBuffer.getNumChannels() == 2) {
                        outputBuffer.addSample(0, startSample + s, currentSample*currentAmpLeft);
                        outputBuffer.addSample(1, startSample + s, currentSample*currentAmpRight);
                    }
                    else {
                        for (int c = 0; c < outputBuffer.getNumChannels(); ++c)
                            outputBuffer.addSample(c, startSample + s, currentSample * currentAmp);
                    }

                    tailOff *= 0.99999f;
                    if (tailOff <= 0.005f)
                    {
                        clearCurrentNote();
                        lfo1sine.reset();
                        lfo1square.reset();
                        break;
                    }
                }
            }
            else
            {
                for (int s = 0; s < numSamples; ++s)
                {
                    //const float currentSample = (osc1.next(pitchMod[s])) * level * currentAmp;
                    float currentSample;
                    if (params.passtype.get() == 0) currentSample = biquadLowpass(osc1.next(pitchMod[s])) * level;
                    else currentSample = biquadHighpass(osc1.next(pitchMod[s])) * level;
                    // TODO Bandpass

                    //check if the output is a stereo output
                    if (outputBuffer.getNumChannels() == 2) {
                        outputBuffer.addSample(0, startSample + s, currentSample*currentAmpLeft);
                        outputBuffer.addSample(1, startSample + s, currentSample*currentAmpRight);
                    }
                    else {
                        for (int c = 0; c < outputBuffer.getNumChannels(); ++c)
                            outputBuffer.addSample(c, startSample + s, currentSample * currentAmp);
                    }
                }
            }
        }
    }

protected:
    void renderModulation(int numSamples) {

        // add pitch wheel values
        float currentPitchInCents = (params.osc1PitchRange.get() * 100) * ((currentPitchValue - 8192.0f) / 8192.0f);

        const float modAmount = params.osc1lfo1depth.get();
        if (params.lfo1wave.get() == 0) // if lfo1wave is 0, lfo is set to sine wave
        {
            for (int s = 0; s < numSamples;++s)
            {
                pitchModBuffer.setSample(0, s, Param::fromSemi(lfo1sine.next()*modAmount) * Param::fromCent(currentPitchInCents));
            }
        }
        else // if lfo1wave is 1, lfo is set to square wave
        {
            for (int s = 0; s < numSamples;++s)
            {
                pitchModBuffer.setSample(0, s, Param::fromSemi(lfo1square.next()*modAmount) * Param::fromCent(currentPitchInCents));
            }
        }
    }

    float biquadHighpass(float inputSignal) {
        const float sRate = static_cast<float>(getSampleRate());

        //New Filter Design: Biquad (2 delays) Source: http://www.musicdsp.org/showArchiveComment.php?ArchiveID=259
        float k, coeff1, coeff2, coeff3, b0, b1, b2, a1, a2;

        const float currentHighcutFreq = params.hpCutoff.get() / sRate;
        const float currentResonance = pow(10.f, -params.hpResonance.get() / 20.f);

        // coefficients for highpass, depending on resonance and highcut frequency
        k = 0.5f * currentResonance * sin(float_Pi * currentHighcutFreq);
        coeff1 = 0.5f * (1.f - k) / (1.f + k);
        coeff2 = (0.5f + coeff1) * cos(float_Pi * currentHighcutFreq);
        coeff3 = (0.5f + coeff1 + coeff2) * 0.25f;

        b0 = 2.f * coeff3;
        b1 = -4.f * coeff3;
        b2 = 2.f * coeff3;
        a1 = -2.f * coeff2;
        a2 = 2.f * coeff1;

        lastSample = inputSignal;

        inputSignal = b0*inputSignal + b1*inputDelay1 + b2*inputDelay2 - a1*outputDelay1 - a2*outputDelay2;

        //delaying samples
        inputDelay2 = inputDelay1;
        inputDelay1 = lastSample;
        outputDelay2 = outputDelay1;
        outputDelay1 = inputSignal;

        return inputSignal;
    }

    float biquadLowpass(float inputSignal) {
        const float sRate = static_cast<float>(getSampleRate());

        //New Filter Design: Biquad (2 delays) Source: http://www.musicdsp.org/showArchiveComment.php?ArchiveID=259
        float k, coeff1, coeff2, coeff3, b0, b1, b2, a1, a2;

        const float currentLowcutFreq = params.lpCutoff.get() / sRate;
        const float currentResonance = pow(10.f, -params.lpResonance.get() / 20.f);

        // coefficients for lowpass, depending on resonance and lowcut frequency
        k = 0.5f * currentResonance * sin(2.f * float_Pi * currentLowcutFreq);
        coeff1 = 0.5f * (1.f - k) / (1.f + k);
        coeff2 = (0.5f + coeff1) * cos(2.f * float_Pi * currentLowcutFreq);
        coeff3 = (0.5f + coeff1 - coeff2) * 0.25f;

        b0 = 2.f * coeff3;
        b1 = 2.f * 2.f * coeff3;
        b2 = 2.f * coeff3;
        a1 = 2.f * -coeff2;
        a2 = 2.f * coeff1;

        lastSample = inputSignal;
        
        inputSignal = b0*inputSignal + b1*inputDelay1 + b2*inputDelay2 - a1*outputDelay1 - a2*outputDelay2;
        
        //delaying samples
        inputDelay2 = inputDelay1;
        inputDelay1 = lastSample;
        outputDelay2 = outputDelay1;
        outputDelay1 = inputSignal;
        
        return inputSignal;
    }


private:
    
    //New Filter Design
    float lastSample, inputDelay1, inputDelay2, outputDelay1, outputDelay2;
    
    SynthParams &params;

    Oscillator<&Waveforms::square> osc1;

    Oscillator<&Waveforms::sinus> lfo1sine;
    Oscillator<&Waveforms::square> lfo1square;

    float level, tailOff;

    int currentPitchValue;

    AudioSampleBuffer pitchModBuffer;

};


