#include <JuceHeader.h>
#include "NavBar.h"
#include "../PluginEditor.h"

NavBar::NavBar(ViatordenoiserAudioProcessor& p) : audioProcessor(p), _presetBrowser(audioProcessor)
{
    // shadow
    _dropShadow = std::make_unique<juce::DropShadower>(juce::DropShadow(juce::Colours::black.withAlpha(_shadowAlpha), 10, {}));
        _dropShadow->setOwner(this);
    
    // buttons
    initButtons();
    setButtonProps();
    
    _buttons[0]->onClick = [this]()
    {
        sendChangeMessage();
    };
    
    _buttons[getButtonIndex("Export")]->onClick = [this]()
    {
        // create default preset folder if needed
        createPresetFolder();
        
        exportPreset();
    };
    
    _buttons[getButtonIndex("Import")]->onClick = [this]()
    {
        importPreset();
    };
    
    _buttons[getButtonIndex("Folder")]->onClick = [this]()
    {
        setPresetFolderLocation();
    };
    
    auto folderPath = audioProcessor.variableTree.getProperty("presetFolder");
    if (folderPath != "")
    {
        _folderPath = folderPath;
        _presetBrowser.updateMenuWithPresets();
    }
    
    // preset browser
    addAndMakeVisible(_presetBrowser);
    _presetBrowser.addMouseListener(this, true);
    createPresetFolder();
}

NavBar::~NavBar()
{
}

void NavBar::paint (juce::Graphics& g)
{
    juce::Rectangle<int> rect = getLocalBounds();
    viator_utils::Gradient::addVerticalGradient(g, _bgColor, rect, viator_utils::Gradient::RectShape::kRounded);
}

void NavBar::resized()
{
    const auto buttonHeight = getHeight();
    const auto buttonWidth = buttonHeight;
    float buttonX = getWidth() - buttonWidth;
    float buttonY = 0;
    
    _buttons[0]->setBounds(buttonX, buttonY, buttonWidth, buttonHeight);
    buttonX = 0;
    _buttons[getButtonIndex("Folder")]->setBounds(buttonX, buttonY, buttonWidth * 2.0, buttonHeight);
    buttonX += buttonWidth * 2.0;
    _buttons[getButtonIndex("Import")]->setBounds(buttonX, buttonY, buttonWidth * 2.0, buttonHeight);
    buttonX += buttonWidth * 2.0;
    _buttons[getButtonIndex("Export")]->setBounds(buttonX, buttonY, buttonWidth * 2.0, buttonHeight);
    
    _presetBrowser.setBounds(_buttons[getButtonIndex("Export")]->getRight(),
                             buttonY,
                             _buttons[0]->getX() - _buttons[getButtonIndex("Export")]->getRight(),
                             buttonHeight);
}

#pragma mark Buttons
void NavBar::setButtonProps()
{
    _buttons[getButtonIndex("Settings")]->setButtonStyle(viator_gui::TextButton::ButtonStyle::kSettings);
    _buttons[getButtonIndex("Export")]->setButtonStyle(viator_gui::TextButton::ButtonStyle::kNormal);
    _buttons[getButtonIndex("Import")]->setButtonStyle(viator_gui::TextButton::ButtonStyle::kNormal);
    _buttons[getButtonIndex("Folder")]->setButtonStyle(viator_gui::TextButton::ButtonStyle::kNormal);
    _buttons[getButtonIndex("Settings")]->setClickingTogglesState(true);
    _buttons[getButtonIndex("Export")]->setClickingTogglesState(false);
    _buttons[getButtonIndex("Import")]->setClickingTogglesState(false);
    _buttons[getButtonIndex("Folder")]->setClickingTogglesState(false);
}

void NavBar::initButtons()
{
    for (int i = 0; i < _numButtons; i++)
    {
        _buttons.add(std::make_unique<viator_gui::TextButton>(_buttonNameTexts[i]));
        _buttons[i]->addMouseListener(this, false);
        addAndMakeVisible(_buttons[i]);
    }
}

int NavBar::getButtonIndex(const juce::String &name)
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

void NavBar::exportPreset()
{
    // Show a file save dialog to choose the export location
    _myChooser = std::make_unique<juce::FileChooser> ("Please select the location to save your preset...", _folderPath, "*.xml");

    auto folderChooserFlags = juce::FileBrowserComponent::saveMode;

    _myChooser->launchAsync (folderChooserFlags, [this] (const juce::FileChooser& chooser)
    {
        juce::File file (chooser.getResult());
        
        auto name = file.getFullPathName();
        
        if (name != "")
        {
            // name xml after project
            juce::String appName = ProjectInfo::projectName;
            juce::XmlElement xml(appName);
            
            // get ref to params
            auto params = audioProcessor._parameterMap.getSliderParams();
            
            // Add parameters to the XML document
            for (auto& param : params)
            {
                auto identifier = param.paramID;
                auto value = audioProcessor._treeState.getRawParameterValue(identifier)->load();
                juce::XmlElement* paramElement = xml.createNewChildElement("Parameter");
                paramElement->setAttribute(identifier,
                                           static_cast<double>(value));
            }
            
            // Write the XML document to the file
            xml.writeTo(file);
            
            // Show a message to indicate successful export
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                                   "Export Successful",
                                                   "Preset exported successfully.");
            
            _presetBrowser.updateMenuWithPresets();
        }
    });
}

void NavBar::importPreset()
{
    // open the file chooser
    _myChooser = std::make_unique<juce::FileChooser> ("Please select the location to save your preset...", _exportFile, "*.xml");

    // set the flags
    auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    // launch the file chooser
    _myChooser->launchAsync (folderChooserFlags, [this] (const juce::FileChooser& chooser)
    {
        // get the xml file
        juce::File file (chooser.getResult());
        
        // load the preset
        loadPreset(file.getFullPathName());
    });
}

void NavBar::setPresetFolderLocation()
{
    // Show a file save dialog to choose the export location
    _myChooser = std::make_unique<juce::FileChooser> ("Please select the location to save your preset...");

    auto folderChooserFlags = juce::FileBrowserComponent::canSelectDirectories;

    _myChooser->launchAsync (folderChooserFlags, [this] (const juce::FileChooser& chooser)
    {
        juce::File file (chooser.getResult());
        
        if (file.isDirectory())
        {
            _folderPath = file.getFullPathName();
            audioProcessor.variableTree.setProperty("presetFolder", file.getFullPathName(), nullptr);
        }
    });
}

void NavBar::createPresetFolder()
{
    if (!_folderPath.isDirectory())
    {
        _folderPath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory);
        auto path = _folderPath.getFullPathName();
        auto pluginName = ProjectInfo::projectName;
        path.append("/", juce::CharPointer_UTF8::getBytesRequiredFor(path.getCharPointer()));
        path.append(pluginName, juce::CharPointer_UTF8::getBytesRequiredFor(path.getCharPointer()));
        juce::File _folder(path);
        if (!_folder.exists())
        {
            _folder.createDirectory();
        }
        
        _folderPath = _folder.getFullPathName();
        audioProcessor.variableTree.setProperty("presetFolder", _folder.getFullPathName(), nullptr);
        _presetBrowser.updateMenuWithPresets();
    }
}

#pragma mark Public
void NavBar::loadPreset(const juce::String& filePath)
{
    // get the xml file
    juce::File file (filePath);
    
    // get the xml data
    juce::String xmlData;
    
    // create an input stream from the chosen file
    std::unique_ptr<juce::FileInputStream> inputStream(file.createInputStream());
    if (inputStream != nullptr)
    {
        xmlData = inputStream->readEntireStreamAsString();
    }
    
    // make xml doc from data
    juce::XmlDocument xmlDoc(xmlData);
    
    // parse the xml data
    if (xmlDoc.parse(xmlData))
    {
        auto rootElement = xmlDoc.getDocumentElement();
        juce::String appName = ProjectInfo::projectName;
        
        // make sure the file is for this plugin
        if (rootElement != nullptr && rootElement->hasTagName(appName))
        {
            auto paramElement = rootElement->getFirstChildElement();
            
            while (paramElement != nullptr)
            {
                for (int i = 0; i < paramElement->getNumAttributes(); ++i)
                {
                    juce::String paramName = paramElement->getAttributeName(i);
                    double paramValue = paramElement->getDoubleAttribute(paramName);
                    audioProcessor._treeState.getParameterAsValue(paramName) = paramValue;
                }
                
                paramElement = paramElement->getNextElement();
            }
        }
        
        // the file isnt for this plugin
        else
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                              "Import Unsuccessful",
                                              "This preset is for another plugin.");
        }
    }
}

bool NavBar::isSettingsActive()
{
    return _buttons[0]->getToggleState();
}

void NavBar::setBGColor(juce::Colour newBGColor)
{
    _bgColor = newBGColor;
    repaint();
}

#pragma mark Tooltips
void NavBar::mouseEnter(const juce::MouseEvent &event)
{
    auto* parent = dynamic_cast<ViatordenoiserAudioProcessorEditor*>(getParentComponent()->getParentComponent());
    jassert(parent != nullptr);
    
    // buttons
    for (int i = 0; i < _buttons.size(); ++i)
    {
        if (event.eventComponent == _buttons[i])
        {
            parent->setTooltipText(_buttonTooltips[i]);
        }
    }
    
    // menu
    if (event.eventComponent->getComponentID() == "Preset")
    {
        parent->setTooltipText(_menuTooltip);
    }
}

void NavBar::mouseExit(const juce::MouseEvent &event)
{
    auto* parent = dynamic_cast<ViatordenoiserAudioProcessorEditor*>(getParentComponent()->getParentComponent());
    jassert(parent != nullptr);
    
    // buttons
    for (int i = 0; i < _buttons.size(); ++i)
    {
        if (event.eventComponent == _buttons[i])
        {
            parent->setTooltipText("");
        }
    }
    
    // menu
    if (event.eventComponent->getComponentID() == "Preset")
    {
        parent->setTooltipText("");
    }
}
