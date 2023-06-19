#include "PluginProcessor.h"
#include "PluginEditor.h"

ViatordenoiserAudioProcessorEditor::ViatordenoiserAudioProcessorEditor (ViatordenoiserAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), _headerComp(audioProcessor), _settingsComp(audioProcessor)
{    
    // header
    addAndMakeVisible(_headerComp);
    
    // plugin dials
    for (int i = 0; i < _numPluginDials; i++)
    {
        _pluginDials.add(std::make_unique<viator_gui::Dial>(_pluginDialNames[i]));
        _pluginSliderAttachments.add(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor._treeState, audioProcessor._parameterMap.getPluginSliderParams()[i].paramID, _pluginDials[i]->getSlider()));
        _pluginDials[i]->getSlider().setTextValueSuffix(" %");
        addAndMakeVisible(*_pluginDials[i]);
    }
    
    // io dials
    for (int i = 0; i < _numIODials; i++)
    {
        _ioDials.add(std::make_unique<viator_gui::Dial>(_ioDialNames[i]));
        _ioSliderAttachments.add(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor._treeState, audioProcessor._parameterMap.getIOSliderParams()[i].paramID, _ioDials[i]->getSlider()));
        addAndMakeVisible(*_ioDials[i]);
    }
    
    initDialProps();
    
    // settings
    addAndMakeVisible(_settingsComp);
    _settingsComp.addChangeListener(this);
    
    // tooltip
    initTooltipLabel();
    _tooltipLabel.setVisible(_settingsComp.getShouldUseTooltips());
    
    // window
    setWindowSizeLogic();
}

ViatordenoiserAudioProcessorEditor::~ViatordenoiserAudioProcessorEditor()
{
}

void ViatordenoiserAudioProcessorEditor::paint (juce::Graphics& g)
{
    // background style
    setBackgroundStyle(g);
    
    // io line
    g.setColour(juce::Colour::fromRGB(60, 60, 73).brighter(0.1).withAlpha(0.25f));
    g.drawLine(_ioArea.getWidth(), _ioArea.getY(), _ioArea.getWidth(), getHeight(), 2.0f);
    
    // component colors
    updateSliderColors();
    _headerComp.setBGColor(_theme.getMainBackgroundColor());
}

void ViatordenoiserAudioProcessorEditor::resized()
{
    // vars
    const auto headerHeight = getHeight() * 0.11;
    auto area = getLocalBounds();
    _headerArea = area.removeFromTop(headerHeight);
    _ioArea = area.removeFromLeft(headerHeight * 4.0);
    _pluginArea = area;
    
    // header
    _headerComp.setBounds(_headerArea);
    
    // settings
    setSettingsState(_headerComp.isSettingsActive());
    _settingsComp.setBounds(getWidth() * 0.66, _headerComp.getBottom() + 10, getWidth() * 0.34 - 10, getHeight() - _headerComp.getHeight() - 20);
    
    // main plugin
    auto dialSize = 0.5;
    for (auto& dial : _pluginDials)
    {
        dial->setBounds(_pluginArea.withSizeKeepingCentre(getWidth() * dialSize, getWidth() * dialSize));
        dial->setDialTextBoxWidth(getWidth() * dialSize);
    }
    
    // io bar
    auto ioDialSize = _ioArea.getWidth();
    auto ioDialY = _ioArea.getY();
    for (auto& dial : _ioDials)
    {
        dial->setBounds(_ioArea.getX(), ioDialY, ioDialSize, ioDialSize);
        dial->setDialTextBoxWidth(ioDialSize);
        ioDialY += ioDialSize;
    }
    
    // tooltips
    auto labelHeight = getHeight() - _pluginDials[0]->getBottom();
    auto labelFontSize = juce::jmin(labelHeight * 0.3f, 12.0f);
    _tooltipLabel.setBounds(_pluginArea.getX(), _pluginDials[0]->getBottom(), _pluginArea.getWidth(), labelHeight);
    _tooltipLabel.setFont(juce::Font("Helvetica", labelFontSize, juce::Font::FontStyleFlags::bold));
    
    // Save plugin size in value tree
    audioProcessor.variableTree.setProperty("width", getWidth(), nullptr);
    audioProcessor.variableTree.setProperty("height", getHeight(), nullptr);
    audioProcessor._width = getWidth();
    audioProcessor._height = getHeight();
}

#pragma mark Window
void ViatordenoiserAudioProcessorEditor::setWindowSizeLogic()
{
    // Grab the window instance and create a rectangle
    juce::Rectangle<float> r = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.toFloat();

    // scale plugin width with a factor that increases as screen size decreases
    // the smaller the screen, the more space the plugin takes up
    const float scaleFactor = 900.0f;
    float width = r.getWidth() * (scaleFactor / r.getWidth());
    
    // above width logic breaks down as the screen size approaches 864
    // so just set anything smaller than 1080 to a scale factor of 0.9
    if (r.getWidth() < 1080.0f)
    {
        width = r.getWidth() * 0.85;
    }
    
    const float height = width * 0.667;

    // Set the size
    if (audioProcessor._width != 0.0)
    {
        setSize(audioProcessor._width, audioProcessor._height);
    }
        
    else
    {
        setSize (width, height);
    }
    
    setResizable(true, true);
    getConstrainer()->setFixedAspectRatio(1.5);
    setResizeLimits(width * 0.5, height * 0.5, r.getWidth(), r.getWidth() * 0.85);
}

void ViatordenoiserAudioProcessorEditor::setBackgroundStyle(juce::Graphics &g)
{
    juce::Colour bgColor = _theme.getMainBackgroundColor();
    juce::Rectangle<int> rect = getLocalBounds();
    viator_utils::gui_utils::Gradient::addRadialGradient(g, bgColor, rect, viator_utils::gui_utils::Gradient::RectShape::kSquared);
}

#pragma mark Settings
void ViatordenoiserAudioProcessorEditor::setSettingsState(bool isActive)
{
    _settingsComp.setVisible(isActive);
    _settingsComp.setEnabled(isActive);
}

void ViatordenoiserAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    // high contrast
    if (_settingsComp.getIsHighContrast())
    {
        _theme.setCurrentTheme(ViatorThemes::ViatorThemeData::Theme::kHighContrast);
    }
    
    else
    {
        _settingsComp.resetToNonContrast();
    }
    
    // tooltips
    _tooltipLabel.setVisible(_settingsComp.getShouldUseTooltips());
    
    repaint();
}

#pragma mark Dials
void ViatordenoiserAudioProcessorEditor::updateSliderColors()
{
    for (auto& dial : _pluginDials)
    {
        dial->setDialColors(_theme.getWidgetFillColor().withAlpha(0.75f),
                            _theme.getWidgetFillColor().withAlpha(0.75f),
                            _theme.getAuxBackgroundColor().withAlpha(0.35f),
                            _theme.getAuxTextColor());
    }
    
    for (auto& dial : _ioDials)
    {
        dial->setDialColors(_theme.getWidgetFillColor().withAlpha(0.75f),
                            _theme.getWidgetFillColor().withAlpha(0.75f),
                            _theme.getAuxBackgroundColor().withAlpha(0.35f),
                            _theme.getAuxTextColor());
    }
}

void ViatordenoiserAudioProcessorEditor::initDialProps()
{
    // plugin
    for (int i = 0; i < _numPluginDials; i++)
    {
        _pluginDials[i]->setDialValueType(viator_gui::CustomDial::ValueType::kInt);
        _pluginDials[i]->getSlider().addMouseListener(this, false);
    }
    
    // io
    for (int i = 0; i < _numIODials; i++)
    {
        _ioDials[i]->setDialValueType(viator_gui::CustomDial::ValueType::kFloat);
        _ioDials[i]->getSlider().addMouseListener(this, false);
    }
}

#pragma mark Tooltips
void ViatordenoiserAudioProcessorEditor::initTooltipLabel()
{
    _tooltipLabel.setText("", juce::dontSendNotification);
    addAndMakeVisible(_tooltipLabel);
}

void ViatordenoiserAudioProcessorEditor::setTooltipText(const juce::String &tooltip)
{
    _tooltipLabel.setText(tooltip, juce::dontSendNotification);
}

#pragma mark Tooltips
void ViatordenoiserAudioProcessorEditor::mouseEnter(const juce::MouseEvent &event)
{
    for (int i = 0; i < _pluginDials.size(); ++i)
    {
        if (event.eventComponent == &_pluginDials[i]->getSlider())
        {
            _tooltipLabel.setText(_pluginDialTooltips[i], juce::dontSendNotification);
        }
    }
    
    for (int i = 0; i < _ioDials.size(); ++i)
    {
        if (event.eventComponent == &_ioDials[i]->getSlider())
        {
            _tooltipLabel.setText(_ioDialTooltips[i], juce::dontSendNotification);
        }
    }
}

void ViatordenoiserAudioProcessorEditor::mouseExit(const juce::MouseEvent &event)
{
    for (int i = 0; i < _pluginDials.size(); ++i)
    {
        if (event.eventComponent == &_pluginDials[i]->getSlider())
        {
            _tooltipLabel.setText("", juce::dontSendNotification);
        }
    }
    
    for (int i = 0; i < _ioDials.size(); ++i)
    {
        if (event.eventComponent == &_ioDials[i]->getSlider())
        {
            _tooltipLabel.setText("", juce::dontSendNotification);
        }
    }
}
