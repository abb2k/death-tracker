#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

template<class T>
class RichTextKey;

class RichTextKeys {
public:
    static constexpr std::string m_color = "color";
    static constexpr std::string m_flip = "flip";
    static constexpr std::string m_mirror = "mirror";
    static constexpr std::string m_workingTime = "workingTime";
    static constexpr std::string m_size = "size";
    static constexpr std::string m_spacing = "spacing";
    static constexpr std::string m_font = "font";
    static constexpr std::string m_link = "link";
    static constexpr std::string m_quickBlue = "cb";
    static constexpr std::string m_quickGreen = "cg";
    static constexpr std::string m_quickLightBlue = "cl";
    static constexpr std::string m_quickCyan = "cj";
    static constexpr std::string m_quickYellow = "cy";
    static constexpr std::string m_quickOrange = "co";
    static constexpr std::string m_quickRed = "cr";
    static constexpr std::string m_quickPurple = "cp";
    static constexpr std::string m_quickViolet = "ca";
    static constexpr std::string m_quickPink = "cd";
    static constexpr std::string m_quickPaleYellow = "cc";
    static constexpr std::string m_quickPaleCyan = "cf";
    static constexpr std::string m_quickGold = "cs";
    static constexpr std::string m_quickSpecialRed = "c_";
    static constexpr std::string m_wave = "wave";
    static constexpr std::string m_gradient = "gradient";
    static constexpr std::string m_cPlus = "cplus";
    static constexpr std::string m_runMoreButton = "runMoreBtn";

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
};