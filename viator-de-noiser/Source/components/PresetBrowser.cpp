#include <JuceHeader.h>
#include "PresetBrowser.h"
#include "NavBar.h"

PresetBrowser::PresetBrowser(ViatordenoiserAudioProcessor& p) : audioProcessor(p)
{
    setComponentID("Preset");
    
    // buttons
    initButtons();
    
    // menu
    initMenu();
}

PresetBrowser::~PresetBrowser()
{
    _presetMenu.removeListener(this);
}

void PresetBrowser::paint (juce::Graphics& g)
{
}

void PresetBrowser::resized()
{
    const auto buttonSize = getHeight();
    const auto menuHeight = getHeight();
    const auto menuWidth = getWidth() - buttonSize * 2.0;
    const auto topMargin = 0;
    auto compX = 0;
    
    _buttons[getButtonIndex("<")]->setBounds(compX, topMargin, buttonSize, buttonSize);
    compX += buttonSize;
    
    _presetMenu.setBounds(compX, topMargin, menuWidth, menuHeight);
    compX += menuWidth;

    _buttons[getButtonIndex(">")]->setBounds(compX, topMargin, buttonSize, buttonSize);
}

#pragma mark Buttons
void PresetBrowser::initButtons()
{
    for (int i = 0; i < _numButtons; i++)
    {
        _buttons.add(std::make_unique<viator_gui::TextButton>(_buttonNameTexts[i]));
        _buttons[i]->setComponentID("Preset");
        addAndMakeVisible(_buttons[i]);
    }
    
    _buttons[getButtonIndex("<")]->setClickingTogglesState(false);
    _buttons[getButtonIndex("<")]->setButtonStyle(viator_gui::TextButton::ButtonStyle::kNormal);
    _buttons[getButtonIndex(">")]->setClickingTogglesState(false);
    _buttons[getButtonIndex(">")]->setButtonStyle(viator_gui::TextButton::ButtonStyle::kNormal);
    
    _buttons[getButtonIndex("<")]->onClick = [this]()
    {
        navigateMenuOnButtonClick(Direction::kLeft);
    };
    
    _buttons[getButtonIndex(">")]->onClick = [this]()
    {
        navigateMenuOnButtonClick(Direction::kRight);
    };
}

int PresetBrowser::getButtonIndex(const juce::String &name)
{
    int index = -1;
    
    for (int i = 0; i < _numButtons; i++)
    {
        if (_buttons[i]->getName() == name)
        {
            index = i;
        }
    }
    
    jassert(index > -1);
    return index;
}

void PresetBrowser::navigateMenuOnButtonClick(Direction newDirection)
{
    auto currentIndex = _presetMenu.getSelectedItemIndex();
    auto num = _presetMenu.getNumItems();
    
    if (newDirection == Direction::kLeft)
    {
        _presetMenu.setSelectedId(currentIndex > 0 ? currentIndex : num);
    }
    
    if (newDirection == Direction::kRight)
    {
        int lastIndex = num - 1;
        int nextIndex = currentIndex + 2;
        
        _presetMenu.setSelectedId(currentIndex < lastIndex ? nextIndex : 1);
    }
}

#pragma mark Menu
void PresetBrowser::initMenu()
{
    addAndMakeVisible(_presetMenu);
    _presetMenu.setComponentID("Preset");
    _presetMenu.setTextWhenNothingSelected("Presets");
    _presetMenu.addListener(this);
}

void PresetBrowser::comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged)
{
    // Check if the change is initiated by the user
    if (!isSettingSelectionProgrammatically)
    {
        auto* parent = dynamic_cast<NavBar*>(getParentComponent());

        // get file path from _xmlFilePaths by index
        parent->loadPreset(_xmlFilePaths[_presetMenu.getSelectedItemIndex()]);

        // save current selection
        auto currentIndex = _presetMenu.getSelectedItemIndex();
        audioProcessor.variableTree.setProperty("presetMenu", currentIndex, nullptr);
    }
    
    else
    {
        // Reset the flag for subsequent user-initiated changes
        isSettingSelectionProgrammatically = false;
    }
}

void PresetBrowser::attachMenu()
{
    isSettingSelectionProgrammatically = true;
    
    int savedSelection = audioProcessor.variableTree.getProperty("presetMenu");
    savedSelection += 1;
    _presetMenu.setSelectedId(savedSelection, juce::dontSendNotification);
}

void PresetBrowser::updateMenuWithPresets()
{
    // clean slate
    _xmlFiles.clear();
    _presetMenu.clear();
    
    // get preset folder
    auto folderPath = audioProcessor.variableTree.getProperty("presetFolder").toString();
    auto presetFolder = juce::File(folderPath);
    
    // make list of files
    presetFolder.findChildFiles(_xmlFiles, juce::File::TypesOfFileToFind::findFiles, true, "*.xml");
    _xmlFiles.sort();

    // populate menu with file names
    for (int i = 0; i < _xmlFiles.size(); ++i)
    {
        // add fill path to _xmlFilePaths
        _xmlFilePaths.insert(i, _xmlFiles[i].getFullPathName());
        juce::String fileName = _xmlFiles[i].getFileNameWithoutExtension();
        _presetMenu.addItem(fileName, i + 1);
    }
    
    // restore menu selection
//    if (_presetMenu.getText() == "")
//    {
        attachMenu();
//    }
}
