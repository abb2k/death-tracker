#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

template<class T>
class RichTextKey;

class RichTextKeys {
public:
    static constexpr char const* m_color = "color";
    static constexpr char const* m_flip = "flip";
    static constexpr char const* m_mirror = "mirror";
    static constexpr char const* m_workingTime = "workingTime";
    static constexpr char const* m_size = "size";
    static constexpr char const* m_spacing = "spacing";
    static constexpr char const* m_font = "font";
    static constexpr char const* m_link = "link";
    static constexpr char const* m_quickBlue = "cb";
    static constexpr char const* m_quickGreen = "cg";
    static constexpr char const* m_quickLightBlue = "cl";
    static constexpr char const* m_quickCyan = "cj";
    static constexpr char const* m_quickYellow = "cy";
    static constexpr char const* m_quickOrange = "co";
    static constexpr char const* m_quickRed = "cr";
    static constexpr char const* m_quickPurple = "cp";
    static constexpr char const* m_quickViolet = "ca";
    static constexpr char const* m_quickPink = "cd";
    static constexpr char const* m_quickPaleYellow = "cc";
    static constexpr char const* m_quickPaleCyan = "cf";
    static constexpr char const* m_quickGold = "cs";
    static constexpr char const* m_quickSpecialRed = "c_";
    static constexpr char const* m_wave = "wave";
    static constexpr char const* m_gradient = "gradient";
    static constexpr char const* m_cPlus = "cplus";
    static constexpr char const* m_runMoreButton = "runMoreBtn";

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