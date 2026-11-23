#include "RichTextKeys.hpp"

#include <actions/CCWaveAction.hpp>
#include <actions/CCGradientAction.hpp>

std::shared_ptr<RichTextKey<RichTextKeys::WaveKeyParams>> RichTextKeys::makeWaveKey(Function<bool()> donerRichTextEnabled) {
    if (m_waveKey)
        return m_waveKey;
    
    m_waveKey = std::make_shared<RichTextKey<RichTextKeys::WaveKeyParams>>(
        "wave",
        [](std::string value) -> Result<RichTextKeys::WaveKeyParams> {
            float speed = 1;
            float distanceY = 5;
            float distanceX = 0;
            float offsetPerIndex = .3f;

            auto splitStr = utils::string::split(value, ",");
            if (splitStr.size() >= 1) {
                auto speedRes = geode::utils::numFromString<float>(splitStr[0]);
                if (speedRes.isOk()) speed = speedRes.unwrap();
            }
            if (splitStr.size() >= 2) {
                auto distanceYRes = geode::utils::numFromString<float>(splitStr[1]);
                if (distanceYRes.isOk()) distanceY = distanceYRes.unwrap();
            }
            if (splitStr.size() >= 3) {
                auto distanceXRes = geode::utils::numFromString<float>(splitStr[2]);
                if (distanceXRes.isOk()) distanceX = distanceXRes.unwrap();
            }
            if (splitStr.size() >= 4) {
                auto offsetPerIndexRes = geode::utils::numFromString<float>(splitStr[3]);
                if (offsetPerIndexRes.isOk()) offsetPerIndex = offsetPerIndexRes.unwrap();
            }

            return Ok(std::make_tuple(speed, distanceY, distanceX, offsetPerIndex));
        },
        [donerRichTextEnabled = std::move(donerRichTextEnabled)](RichTextKeys::WaveKeyParams const& value, cocos2d::CCFontSprite* sprite, int localIndex, int charIndex) mutable {
            if (!donerRichTextEnabled()) return;

            sprite->runAction(
                CCRepeatForever::create(
                    CCWaveAction::create(
                        std::get<0>(value),
                        std::get<2>(value),
                        std::get<1>(value),
                        std::get<3>(value) * localIndex
                    )
                )
            );
        }
    );

    return m_waveKey;
}
std::shared_ptr<RichTextKey<RichTextKeys::GradientKeyParams>> RichTextKeys::makeGradientKey(Function<bool()> donerRichTextEnabled) {
    if (m_gradientKey)
        return m_gradientKey;
    
    m_gradientKey = std::make_shared<RichTextKey<RichTextKeys::GradientKeyParams>>(
        "gradient",
        [](std::string value) -> Result<RichTextKeys::GradientKeyParams> {
            ccColor4B colorA = {255, 255, 255, 255};
            ccColor4B colorB = {255, 255, 255, 255};
            float transitionDuration = 1.0f;
            float stagger = 0.2f;

            auto splitStr = utils::string::split(value, ",");
            if (splitStr.size() >= 1) {
                auto res = geode::cocos::cc4bFromHexString(splitStr[0]);
                if (res.isOk()) colorA = res.unwrap();
            }
            if (splitStr.size() >= 2) {
                auto res = geode::cocos::cc4bFromHexString(splitStr[1]);
                if (res.isOk()) colorB = res.unwrap();
            }
            if (splitStr.size() >= 3) {
                auto res = geode::utils::numFromString<float>(splitStr[2]);
                if (res.isOk() && std::isfinite(res.unwrap())) {
                    transitionDuration = std::clamp(res.unwrap(), 0.05f, 30.0f);
                }
            }
            if (splitStr.size() >= 4) {
                auto res = geode::utils::numFromString<float>(splitStr[3]);
                if (res.isOk() && std::isfinite(res.unwrap())) {
                    stagger = std::clamp(res.unwrap(), 0.0f, 2.0f);
                }
            }

            return Ok(std::make_tuple(colorA, colorB, transitionDuration, stagger));
        },
        [donerRichTextEnabled = std::move(donerRichTextEnabled)](RichTextKeys::GradientKeyParams const& value, cocos2d::CCFontSprite* sprite, int localIndex, int) mutable {
            if (!donerRichTextEnabled()) return;

            auto colorA = std::get<0>(value);
            auto colorB = std::get<1>(value);
            auto transitionDuration = std::get<2>(value);
            auto stagger = std::get<3>(value);

            if (sprite->getActionByTag(12) != nullptr) return;

            auto animation = CCRepeatForever::create(CCGradientAction::create(
                transitionDuration,
                colorA,
                colorB,
                -stagger * localIndex
            ));
            animation->setTag(12);

            sprite->runAction(animation);
        }
    );

    return m_gradientKey;
}
std::shared_ptr<RichTextKey<RichTextKeys::CPlusKeyParams>> RichTextKeys::makeCPlusKey() {
    if (m_cPlusKey)
        return m_cPlusKey;

    m_cPlusKey = std::make_shared<RichTextKey<RichTextKeys::CPlusKeyParams>>(
        "cplus",
        [](std::string value) -> Result<RichTextKeys::CPlusKeyParams> {
            GEODE_UNWRAP_INTO(auto num, utils::numFromString<int>(value));

            return Ok(num);
        },
        [](RichTextKeys::CPlusKeyParams const& value, cocos2d::CCFontSprite* sprite, int localIndex, int charIndex) {
            if (auto gradient = typeinfo_cast<CCGradientAction*>(sprite->getActionByTag(12))) {
                gradient->addColorOffset(value);
                return;
            }

            auto colorAdded = sprite->getColor();
            if (colorAdded == ccWHITE) colorAdded = static_cast<CCLabelBMFont*>(sprite->getParent())->getColor();

            auto r = static_cast<std::int64_t>(colorAdded.r) + value;
            auto g = static_cast<std::int64_t>(colorAdded.g) + value;
            auto b = static_cast<std::int64_t>(colorAdded.b) + value;

            r = std::clamp<std::int64_t>(r, 0, 255);
            g = std::clamp<std::int64_t>(g, 0, 255);
            b = std::clamp<std::int64_t>(b, 0, 255);

            auto newColor = ccColor3B{static_cast<GLubyte>(r), static_cast<GLubyte>(g), static_cast<GLubyte>(b)};

            sprite->setColor(newColor);
        }
    );
    
    return m_cPlusKey;
}