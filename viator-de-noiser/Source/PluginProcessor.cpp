#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ViatordenoiserAudioProcessor::ViatordenoiserAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
, _treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        _treeState.addParameterListener(_parameterMap.getSliderParams()[i].paramID, this);
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        _treeState.addParameterListener(_parameterMap.getButtonParams()[i]._id, this);
    }
    
    // init var states
    variableTree.setProperty("width", 0, nullptr);
    variableTree.setProperty("heigt", 0, nullptr);
    variableTree.setProperty("colorMenu", 1, nullptr);
    variableTree.setProperty("tooltipState", 1, nullptr);
}

ViatordenoiserAudioProcessor::~ViatordenoiserAudioProcessor()
{
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        _treeState.removeParameterListener(_parameterMap.getSliderParams()[i].paramID, this);
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        _treeState.removeParameterListener(_parameterMap.getButtonParams()[i]._id, this);
    }
}

//==============================================================================
const juce::String ViatordenoiserAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ViatordenoiserAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ViatordenoiserAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ViatordenoiserAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ViatordenoiserAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ViatordenoiserAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ViatordenoiserAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ViatordenoiserAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ViatordenoiserAudioProcessor::getProgramName (int index)
{
    return {};
}

void ViatordenoiserAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ViatordenoiserAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // sliders
    for (int i = 0; i < _parameterMap.getSliderParams().size(); i++)
    {
        auto param = _parameterMap.getSliderParams()[i];
        
        if (param.isInt == ViatorParameters::SliderParameterData::NumericType::kInt || param.isSkew == ViatorParameters::SliderParameterData::SkewType::kSkew)
        {
            auto range = juce::NormalisableRange<float>(param.min, param.max);
            
            if (param.isSkew == ViatorParameters::SliderParameterData::SkewType::kSkew)
            {
                range.setSkewForCentre(param.center);
            }
            
            params.push_back (std::make_unique<juce::AudioProcessorValueTreeState::Parameter>(juce::ParameterID { param.paramID, 1 }, param.name, param.name, range, param.initial, valueToTextFunction, textToValueFunction));
        }
        
        else
        {
            params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { param.paramID, 1 }, param.name, param.min, param.max, param.initial));
        }
    }
    
    // buttons
    for (int i = 0; i < _parameterMap.getButtonParams().size(); i++)
    {
        auto param = _parameterMap.getButtonParams()[i];
        params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID { param._id, 1 }, param._name, false));
    }
        
    return { params.begin(), params.end() };
}

void ViatordenoiserAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)

{
    updateParameters();
}

void ViatordenoiserAudioProcessor::updateParameters()
{
    // params
    auto volume = _treeState.getRawParameterValue(ViatorParameters::outputID)->load();
    auto reduction = _treeState.getRawParameterValue(ViatorParameters::reductionID)->load();
    
    // update
    auto gainCompensation = reduction * 0.166;
    auto reductionScaled = juce::jmap(reduction, 0.0f, 100.0f, 1.0f, 0.5f);
    _expanderModule.setRatio(reductionScaled);
    _expanderModule.setThreshold(0.0);
    _expanderModule.setAttack(50.0);
    _expanderModule.setRelease(150.0);
    _compensationModule.setGainDecibels(gainCompensation);
    _volumeModule.setGainDecibels(volume);
    
}

//==============================================================================
void ViatordenoiserAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    _spec.sampleRate = sampleRate;
    _spec.maximumBlockSize = samplesPerBlock;
    _spec.numChannels = getTotalNumInputChannels();
    
    _expanderModule.prepare(_spec);
    
    _compensationModule.prepare(_spec);
    _compensationModule.setRampDurationSeconds(0.02);
    _volumeModule.prepare(_spec);
    _volumeModule.setRampDurationSeconds(0.02);
    
    updateParameters();
}

void ViatordenoiserAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ViatordenoiserAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
#endif

void ViatordenoiserAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::dsp::AudioBlock<float> block {buffer};
    
    _expanderModule.process(juce::dsp::ProcessContextReplacing<float>(block));
    _compensationModule.process(juce::dsp::ProcessContextReplacing<float>(block));
    _volumeModule.process(juce::dsp::ProcessContextReplacing<float>(block));
}

//==============================================================================
bool ViatordenoiserAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ViatordenoiserAudioProcessor::createEditor()
{
    return new ViatordenoiserAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void ViatordenoiserAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    _treeState.state.appendChild(variableTree, nullptr);
    juce::MemoryOutputStream stream(destData, false);
    _treeState.state.writeToStream (stream);
}

void ViatordenoiserAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, size_t(sizeInBytes));
    variableTree = tree.getChildWithName("Variables");
    
    if (tree.isValid())
    {
        _treeState.state = tree;
        _width = variableTree.getProperty("width");
        _height = variableTree.getProperty("height");
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViatordenoiserAudioProcessor();
}
