#include <JuceHeader.h>
#include "Header.h"

Header::Header(ViatordenoiserAudioProcessor& p) : audioProcessor(p),
_navBar(audioProcessor)
{
    addAndMakeVisible(_navBar);
    _navBar.addChangeListener(this);
}

Header::~Header()
{
}

void Header::paint (juce::Graphics& g)
{
    auto rect = getLocalBounds();
    
    // force image to color correctly
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    
    // add logo image
    auto headerLogo = juce::ImageCache::getFromMemory(BinaryData::landon_png, BinaryData::landon_pngSize);
    g.drawImageWithin(headerLogo,
                      getWidth() * 0.02,
                      0,
                      getWidth() * 0.17,
                      getHeight(),
                      juce::RectanglePlacement::centred);
    
    g.setColour(juce::Colour::fromRGB(60, 60, 73).brighter(0.1).withAlpha(0.25f));
    g.drawLine(rect.getX(), rect.getHeight(), rect.getWidth(), rect.getHeight(), 2.0f);
}

void Header::resized()
{
    //_navBarArea  = getLocalBounds().removeFromRight(getHeight() * 9.6);
    //_navBar.setBounds(_navBarArea.withSizeKeepingCentre(_navBarArea.getWidth() * 0.75, getHeight() * 0.5));
}

void Header::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    getParentComponent()->resized();
}

bool Header::isSettingsActive()
{
    return _navBar.isSettingsActive();
}

void Header::setBGColor(juce::Colour newBGColor)
{
    _navBar.setBGColor(newBGColor);
}
