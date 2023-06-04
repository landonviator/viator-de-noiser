#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "PresetBrowser.h"

class ViatordenoiserAudioProcessorEditor;
class NavBar  : public juce::Component, public juce::ChangeBroadcaster
{
public:
    NavBar(ViatordenoiserAudioProcessor&);
    ~NavBar() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    bool isSettingsActive();
    void setBGColor(juce::Colour newBGColor);
    
    void loadPreset(const juce::String& filePath);

private:
    ViatordenoiserAudioProcessor& audioProcessor;
    
    // shadow
    std::unique_ptr<juce::DropShadower> _dropShadow;
    
    // buttons
    juce::OwnedArray<viator_gui::TextButton> _buttons;
    juce::StringArray _buttonNameTexts =
    {
      "Settings", "Export", "Import", "Folder"
    };
    
    juce::StringArray _buttonTooltips =
    {
        "Settings page where you can change color themes, toggle tooltips, and more!",
        "Export current state as preset file to share or save.",
        "Import preset file and apply to parameters. (Must be an xml file with this plugin's name)",
        "Set the default location to store presets for this plugin when exported."
    };
    
    juce::String _menuTooltip = "A menu to select presets.";
    
    std::unique_ptr<juce::FileChooser> _myChooser;
    
    PresetBrowser _presetBrowser;
    
private:
    
    // buttons
    void setButtonProps();
    void initButtons();
    
    int getButtonIndex(const juce::String& name);
    
    void exportPreset();
    void importPreset();
    void createPresetFolder();
    void setPresetFolderLocation();
    
    void mouseEnter(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    
private:
    const float _shadowAlpha = 0.25f;
    juce::Colour _bgColor = juce::Colour::fromRGB(60, 60, 73).brighter(0.1);
    const int _numButtons = 4;
    juce::File _exportFile;
    juce::File _folderPath;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NavBar)
};
