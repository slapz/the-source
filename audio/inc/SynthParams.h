
#pragma once

#include "JuceHeader.h"
#include "Param.h"

class SynthParams {
public:
    SynthParams();

    Param freq;  //!< master tune in Hz

    Param lfo1freq; //!< lfo frequency in Hz
    Param lfo1wave; //!< lfo wave switch 0 = sine wave or 1 = square wave

    Param osc1fine;      //!< fine tune in [-100..100] ct
    Param osc1lfo1depth; //!< modulation depth in [-12..12] st

    ParamDb vol; //!< volume in [0..1]

    MidiKeyboardState keyboardState;

    void writeXMLPatch(); // writes the XML string to a file

    void readXMLPatch(); // fill the synth params with data from a file


protected:
};
