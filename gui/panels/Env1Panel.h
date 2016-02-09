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

#ifndef __JUCE_HEADER_935F64844A5D65A4__
#define __JUCE_HEADER_935F64844A5D65A4__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "PanelBase.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class Env1Panel  : public PanelBase,
                   public SliderListener,
                   public ComboBoxListener
{
public:
    //==============================================================================
    Env1Panel (SynthParams &p);
    ~Env1Panel();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<MouseOverKnob> attackTime1;
    ScopedPointer<MouseOverKnob> decayTime1;
    ScopedPointer<MouseOverKnob> sustainLevel1;
    ScopedPointer<MouseOverKnob> releaseTime1;
    ScopedPointer<MouseOverKnob> attackShape1;
    ScopedPointer<MouseOverKnob> decayShape1;
    ScopedPointer<MouseOverKnob> releaseShape1;
    ScopedPointer<MouseOverKnob> keyVelToEnv1;
    ScopedPointer<Label> Env1Label;
    ScopedPointer<ComboBox> env2SpeedModSrc1;
    ScopedPointer<ComboBox> env2SpeedModSrc2;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Env1Panel)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_935F64844A5D65A4__