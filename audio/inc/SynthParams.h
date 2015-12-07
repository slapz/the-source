#pragma once

#include "JuceHeader.h"
#include "Param.h"

class SynthParams {
public:
    SynthParams();
    ~SynthParams();

    Param freq;  //!< master tune in Hz

    Param lfo1freq; //!< lfo frequency in Hz
    Param lfo1wave; //!< lfo wave switch 0 = sine wave or 1 = square wave

    Param osc1fine;      //!< fine tune in [-100..100] ct
    Param osc1coarse;    //!< coarse tune in [-11..11] st
    Param osc1lfo1depth; //!< modulation depth in [-12..12] st

    Param passtype; //!< passtype that decides whether lowpass, highpass or bandpass filter is used
    
    Param lpCutoff; //!< filter cutoff frequency in Hz
    Param lpResonance; //! filter resonance in dB

    Param hpCutoff; //!< filter cutoff frequency in Hz
    Param hpResonance; //! filter resonance in dB

    Param osc1trngAmount; //Triangle Amount [0 ... 1]
    Param osc1PitchRange; //!< range in [0..12] st
    Param osc1pulsewidth;//!< pulse width in [0,01..0,99]

    Param panDir; //!< pan R/L [-100..100]

    ParamDb vol; //!< volume in [0..1]

    MidiKeyboardState keyboardState;

protected:
};
