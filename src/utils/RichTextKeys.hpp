#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

template<class T>
class RichTextKey;

class RichTextKeys {
public:
    using WaveKeyParams = std::tuple<float, float, float, float>;
    using GradientKeyParams = std::tuple<ccColor4B, ccColor4B, float, float>;
    using CPlusKeyParams = int;
    using RunMoreKeyParams = std::tuple<Run, bool, std::optional<long long>, size_t>;

    static std::shared_ptr<RichTextKey<WaveKeyParams>> makeWaveKey(Function<bool()> donerRichTextEnabled);
    static std::shared_ptr<RichTextKey<GradientKeyParams>> makeGradientKey(Function<bool()> donerRichTextEnabled);
    static std::shared_ptr<RichTextKey<CPlusKeyParams>> makeCPlusKey();
    static std::shared_ptr<RichTextKey<RunMoreKeyParams>> makeRunMoreKey(
        Function<std::string()> font,
        Function<bool()> buttonsVisible,
        Function<void(CCMenuItemSpriteExtra*)> onClick
    );
private:
    static std::shared_ptr<RichTextKey<WaveKeyParams>> m_waveKey;
    static std::shared_ptr<RichTextKey<GradientKeyParams>> m_gradientKey;
    static std::shared_ptr<RichTextKey<CPlusKeyParams>> m_cPlusKey;
};