#include "Parameters.h"


ViatorParameters::Params::Params()
{
    initSliderParams();
    initButtonParams();
}

std::vector<ViatorParameters::SliderParameterData>& ViatorParameters::Params::getSliderParams()
{
    return _sliderParams;
}

std::vector<ViatorParameters::SliderParameterData>& ViatorParameters::Params::getPluginSliderParams()
{
    return _pluginSliderParams;
}

std::vector<ViatorParameters::SliderParameterData>& ViatorParameters::Params::getIOSliderParams()
{
    return _ioSliderParams;
}

void ViatorParameters::Params::initSliderParams()
{
    using skew = SliderParameterData::SkewType;
    using type = SliderParameterData::NumericType;
    _sliderParams.push_back({ViatorParameters::reductionID, ViatorParameters::reductionName, 0.0, 100.0, 0.0, skew::kSkew, 60.0, type::kInt});
    _sliderParams.push_back({ViatorParameters::inputID, ViatorParameters::inputName, -20.0f, 20.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _sliderParams.push_back({ViatorParameters::outputID, ViatorParameters::outputName, -20.0f, 20.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    
    // for attachments
    _pluginSliderParams.push_back({ViatorParameters::reductionID, ViatorParameters::reductionName, 0.0, 100.0, 0.0, skew::kSkew, 60.0, type::kInt});
    _ioSliderParams.push_back({ViatorParameters::inputID, ViatorParameters::inputName, -20.0f, 20.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
    _ioSliderParams.push_back({ViatorParameters::outputID, ViatorParameters::outputName, -20.0f, 20.0f, 0.0f, skew::kNoSkew, 0.0, type::kFloat});
}

std::vector<ViatorParameters::ButtonParameterData>& ViatorParameters::Params::getButtonParams()
{
    return _buttonParams;
}

void ViatorParameters::Params::initButtonParams()
{
}
