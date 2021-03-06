/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.2.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_B8FE7FA7A346D8E2__
#define __JUCE_HEADER_B8FE7FA7A346D8E2__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "PanelBase.h"
#include "EnvelopeCurve.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
An auto-generated component, created by the Introjucer.

Describe your class and how it works here!
                                                                    //[/Comments]
*/
class EnvPanel  : public PanelBase,
                  public SliderListener,
                  public ComboBoxListener
{
public:
    //==============================================================================
    EnvPanel (SynthParams &p);
    ~EnvPanel();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void updateCurve();
    void updateModAmountKnobs();
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    SynthParams::EnvVol &envVol;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<MouseOverKnob> attackTime;
    ScopedPointer<MouseOverKnob> decayTime;
    ScopedPointer<MouseOverKnob> sustainLevel;
    ScopedPointer<MouseOverKnob> releaseTime;
    ScopedPointer<MouseOverKnob> attackShape;
    ScopedPointer<MouseOverKnob> decayShape;
    ScopedPointer<MouseOverKnob> releaseShape;
    ScopedPointer<MouseOverKnob> speedMod1;
    ScopedPointer<EnvelopeCurve> envelopeCurve;
    ScopedPointer<Label> shapeLabel1;
    ScopedPointer<ModSourceBox> envSpeedModSrc2;
    ScopedPointer<ModSourceBox> envSpeedModSrc1;
    ScopedPointer<MouseOverKnob> speedMod2;
    ScopedPointer<Label> speedModLabel;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvPanel)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_B8FE7FA7A346D8E2__
