#pragma once

#include <nodes/general/RichTextArea.hpp>

using namespace geode::prelude;

class RichTextKeys {
public:
    using WaveKeyParams = std::tuple<float, float, float, float>;
    using GradientKeyParams = std::tuple<ccColor4B, ccColor4B, float, float>;
    using CPlusKeyParams = int;

    static std::shared_ptr<RichTextKey<WaveKeyParams>> makeWaveKey(Function<bool()> donerRichTextEnabled);
    static std::shared_ptr<RichTextKey<GradientKeyParams>> makeGradientKey(Function<bool()> donerRichTextEnabled);
    static std::shared_ptr<RichTextKey<CPlusKeyParams>> makeCPlusKey();
private:
    static std::shared_ptr<RichTextKey<WaveKeyParams>> m_waveKey;
    static std::shared_ptr<RichTextKey<GradientKeyParams>> m_gradientKey;
    static std::shared_ptr<RichTextKey<CPlusKeyParams>> m_cPlusKey;
};