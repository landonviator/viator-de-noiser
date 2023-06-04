#pragma once
#include <JuceHeader.h>
#include "NavBar.h"
#include "../PluginProcessor.h"

class Header  : public juce::Component, public juce::ChangeListener
{
public:
    Header(ViatordenoiserAudioProcessor&);
    ~Header() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    bool isSettingsActive();
    void setBGColor(juce::Colour newBGColor);

private:
    ViatordenoiserAudioProcessor& audioProcessor;
    
    NavBar _navBar;
    
private:
    // change listener
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    
private:
    juce::Rectangle<int> _navBarArea;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Header)
};
