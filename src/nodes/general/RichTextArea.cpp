#include "RichTextArea.hpp"

#include <numeric>
#include <regex>
#include <asp/iter.hpp>

std::vector<RichTextArea::RichCharacter> RichTextArea::buildCharacters() {
    std::vector<RichCharacter> characters;
    std::map<std::string, std::shared_ptr<RichTextKeyInstanceBase>> activeInstances;
    std::map<std::string, size_t> activationOrder;
    std::map<std::shared_ptr<RichTextKeyInstanceBase>, int> localIndices;
    size_t nextActivationOrder = 0;
    size_t byteIndex = 0;
    size_t characterIndex = 0;

    while (byteIndex < m_text.size()) {
        int sourceIndex = static_cast<int>(byteIndex);
        if (m_richTextInstances.contains(sourceIndex)) {
            for (const auto& instance : m_richTextInstances[sourceIndex]) {
                auto key = instance->getKey();
                if (instance->isCancellation()) {
                    auto active = activeInstances.find(key);

                    if (active != activeInstances.end()) {
                        localIndices.erase(active->second);
                        activeInstances.erase(active);
                        activationOrder.erase(key);
                    }

                    continue;
                }

                activeInstances[key] = instance;
                localIndices[instance] = 0;
                activationOrder[key] = nextActivationOrder++;
            }
        }

        if (m_specialExitCancellations.contains(sourceIndex)) {
            const std::string* latestKey = nullptr;
            size_t latestOrder = 0;
            for (const auto& key : m_specialExitCancellations[sourceIndex]) {
                auto active = activeInstances.find(key);
                auto order = activationOrder.find(key);
                if (active != activeInstances.end() && order != activationOrder.end() && (latestKey == nullptr || order->second > latestOrder)) {
                    latestKey = &key;
                    latestOrder = order->second;
                }
            }

            if (latestKey != nullptr) {
                auto active = activeInstances.find(*latestKey);
                localIndices.erase(active->second);
                activeInstances.erase(active);
                activationOrder.erase(*latestKey);
            }
        }

        auto lead = static_cast<unsigned char>(m_text[byteIndex]);
        size_t length = 1;
        if (lead >= 0xC0 && lead <= 0xDF) {
            length = 2;
        } else if (lead >= 0xE0 && lead <= 0xEF) {
            length = 3;
        } else if (lead >= 0xF0 && lead <= 0xF7) {
            length = 4;
        }
        if (byteIndex + length > m_text.size()) {
            length = 1;
        }

        auto codepoint = lead;
        auto appendContinuationByte = [&](char32_t value, size_t offset) {
            auto byte = static_cast<unsigned char>(m_text[byteIndex + offset]);
            return value * 64 + byte % 64;
        };
        if (length == 2) {
            codepoint = appendContinuationByte(lead - 0xC0, 1);
        } else if (length == 3) {
            codepoint = appendContinuationByte(lead - 0xE0, 1);
            codepoint = appendContinuationByte(codepoint, 2);
        } else if (length == 4) {
            codepoint = appendContinuationByte(lead - 0xF0, 1);
            codepoint = appendContinuationByte(codepoint, 2);
            codepoint = appendContinuationByte(codepoint, 3);
        }

        RichCharacter character{
            m_text.substr(byteIndex, length),
            codepoint,
            byteIndex,
            characterIndex,
            {}
        };
        character.m_effects.reserve(activeInstances.size());
        for (const auto& [key, instance] : activeInstances) {
            character.m_effects.push_back({instance, localIndices[instance]});
        }
        characters.push_back(std::move(character));

        if (m_text.compare(byteIndex, length, "\n") != 0) {
            for (const auto& [key, instance] : activeInstances) {
                localIndices[instance]++;
            }
        }

        byteIndex += length;
        characterIndex++;
    }

    return characters;
}

geode::Label* RichTextArea::createLabel(std::vector<RichCharacter> const& characters, std::vector<PendingNodeEffect>& pendingEffects) {
    auto label = geode::Label::create(m_font.c_str());
    if (!label) {
        log::error("Failed to create rich text label using font '{}'", m_font);
        return nullptr;
    }

    std::vector<geode::Label::Run> runs;
    std::string plainText;
    char32_t previousCodepoint = 0;
    geode::BitmapFont* previousFont = label->getFont();
    auto flushPlainText = [&] {
        if (plainText.empty()) return;
        geode::Label::Run run;
        run.text = std::move(plainText);
        runs.push_back(std::move(run));
        plainText.clear();
    };

    for (const auto& character : characters) {
        if (character.m_effects.empty() || character.m_text == "\n") {
            plainText += character.m_text;
            if (character.m_text == "\n") {
                previousCodepoint = 0;
                previousFont = nullptr;
            } else {
                previousCodepoint = character.m_codepoint;
                previousFont = label->getFont();
            }
            continue;
        }

        flushPlainText();
        std::string characterFont = m_font;
        for (const auto& effect : character.m_effects) {
            if (const auto font = effect.m_instance->getCharacterFont()) {
                characterFont = font.value();
            }
        }

        auto glyphLabel = geode::Label::create(character.m_text, characterFont.c_str());
        auto glyphNode = cocos2d::CCNode::create();
        if (!glyphLabel || !glyphNode) {
            log::error("Failed to create rich text glyph using font '{}'", m_font);
            return nullptr;
        }

        glyphLabel->setAnchorPoint({ 0.f, 0.f });
        glyphLabel->setColor({ m_color.r, m_color.g, m_color.b });
        glyphLabel->setOpacity(m_color.a);

        auto glyphSize = glyphLabel->getContentSize();
        auto cursor = glyphLabel->getCursorPosition(glyphLabel->getCodepointCount());
        float glyphAdvance = cursor ? cursor->x : glyphSize.width;
        auto font = glyphLabel->getFont();
        float kerning = previousCodepoint && font && previousFont == font ? font->getKerning(previousCodepoint, character.m_codepoint) : 0.f;
        float characterSpacing = std::accumulate(
            character.m_effects.begin(),
            character.m_effects.end(),
            0.f,
            [](float spacing, const CharacterEffect& effect) {
                return spacing + effect.m_instance->getCharacterSpacing().value_or(0.f);
            }
        );
        float sizeScale = 1.f;
        for (const auto& effect : character.m_effects) {
            if (const auto size = effect.m_instance->getCharacterSize()) {
                sizeScale = std::abs(size.value());

                effect.m_instance->applyChangesToSprite(
                    glyphLabel,
                    effect.m_localIndex,
                    static_cast<int>(character.m_sourceCharacterIndex + 1)
                );
            }
        }

        float baseAdvance = std::max(0.f, glyphAdvance + kerning + characterSpacing);
        float baseHeight = std::max(glyphLabel->getLineHeight(), glyphSize.height);
        glyphNode->setContentSize({
            baseAdvance * sizeScale,
            baseHeight * sizeScale
        });
        glyphNode->setAnchorPoint({ 0.5f, 0.5f });
        glyphLabel->setPosition({ kerning * sizeScale, 0.f });
        glyphNode->addChild(glyphLabel);
        previousFont = glyphLabel->getFont();

        for (const auto& effect : character.m_effects) {
            if (effect.m_instance->getCharacterSize() || effect.m_instance->getCharacterSpacing() || effect.m_instance->getCharacterFont()) {
                continue;
            } else {
                pendingEffects.push_back({
                    glyphLabel,
                    effect.m_instance,
                    effect.m_localIndex,
                    character.m_sourceCharacterIndex
                });
            }
        }

        Ref<cocos2d::CCNode> glyphNodeRef(glyphNode);
        geode::Label::Run run;
        run.node = [glyphNodeRef]() -> cocos2d::CCNode* {
            return glyphNodeRef.data();
        };
        runs.push_back(std::move(run));
        previousCodepoint = character.m_codepoint;
    }
    flushPlainText();

    label->setRuns(std::move(runs));
    label->setColor({ m_color.r, m_color.g, m_color.b });
    label->setOpacity(m_color.a);
    label->setScale(m_scale);
    label->setLineSpacing(m_scale != 0.f ? m_linePadding / m_scale : 0.f);
    label->setBreakWords(m_wrappingMode == geode::WORD_WRAP || m_wrappingMode == geode::CUTOFF_WRAP);
    label->setMaxWidth(
        m_artificialWidth && m_wrappingMode != geode::NO_WRAP && m_scale > 0.f ? getWidth() / m_scale : 0.f
    );

    switch (m_alignment) {
        case cocos2d::kCCTextAlignmentCenter:
            label->setAlignment(geode::Label::Alignment::Center);
            break;
        case cocos2d::kCCTextAlignmentRight:
            label->setAlignment(geode::Label::Alignment::Right);
            break;
        default:
            label->setAlignment(geode::Label::Alignment::Left);
            break;
    }

    label->validate();
    return label;
}

void RichTextArea::updateContainer() {
    auto characters = this->buildCharacters();
    m_pendingNodeEffects.clear();
    m_charactersForButton.clear();
    m_currentlyHeldButton = nullptr;

    auto label = this->createLabel(characters, m_pendingNodeEffects);
    if (label && m_maxLines) {
        auto cutoff = characters.size();
        auto codepointCount = std::min(label->getCodepointCount(), characters.size());
        auto lineRects = label->getRangeRects(0, codepointCount);

        if (lineRects.size() > m_maxLines) {
            const float overflowLineY = lineRects[m_maxLines].origin.y;
            for (size_t i = 0; i < codepointCount; i++) {
                auto cursor = label->getCursorPosition(i);
                if (cursor && std::abs(cursor->y - overflowLineY) < 0.01f) {
                    cutoff = i;
                    break;
                }
            }
        }

        if (cutoff < characters.size()) {
            characters.resize(cutoff);
            while (!characters.empty() && characters.back().m_text == "\n") {
                characters.pop_back();
            }
            for (int i = 0; i < 3; i++) {
                characters.push_back({
                    ".",
                    U'.',
                    m_text.size(),
                    characters.size(),
                    {}
                });
            }

            m_pendingNodeEffects.clear();
            label = this->createLabel(characters, m_pendingNodeEffects);
        }
    }

    m_lines.clear();
    m_container->removeAllChildren();
    if (!label) {
        m_lineHeight = 0.f;
        setContentSize({ getWidth(), 0.f });
        m_container->setContentSize(getContentSize());
        return;
    }

    m_lines.push_back(label);
    auto labelSize = label->getContentSize();
    float height = std::max(0.f, labelSize.height * m_scale - (labelSize.height > 0.f ? m_linePadding : 0.f));
    m_lineHeight = label->getLineHeight() * m_scale - m_linePadding;
    setContentSize({ getWidth(), height });
    m_container->setContentSize(getContentSize());

    float anchorX = 0.f;
    float positionX = 0.f;
    if (m_alignment == cocos2d::kCCTextAlignmentCenter) {
        anchorX = 0.5f;
        positionX = getWidth() / 2.f;
    } else if (m_alignment == cocos2d::kCCTextAlignmentRight) {
        anchorX = 1.f;
        positionX = getWidth();
    }
    label->setAnchorPoint({ anchorX, 1.f });
    label->setPosition({ positionX, height });
    m_container->addChild(label);

    this->didUpdateContainer();
}

RichTextArea* RichTextArea::create(std::string text, std::string font, float scale) {
    return RichTextArea::create(
        std::move(font), std::move(text), scale,
        cocos2d::CCDirector::sharedDirector()->getWinSize().width / 2,
        false
    );
}

RichTextArea* RichTextArea::create(std::string text, std::string font, float scale, float width) {
    return RichTextArea::create(
        std::move(font), std::move(text), scale, width, true
    );
}

RichTextArea* RichTextArea::create(std::string font, std::string text, float scale, float width, bool artificialWidth) {
    RichTextArea* instance = new RichTextArea();

    if (instance->init(std::move(font), std::move(text), scale, width, artificialWidth)) {
        instance->autorelease();
        return instance;
    }

    delete instance;
    return nullptr;
}

bool RichTextArea::init(std::string font, std::string text, float scale, float width, bool artificialWidth) {
    m_font = std::move(font);
    m_text = std::move(text);
    m_rawText = m_text;
    m_scale = scale;
    m_artificialWidth = artificialWidth;
    m_container = cocos2d::CCMenu::create();

    this->setAnchorPoint({ 0.5f, 0.5f });
    m_container->setPosition({ 0, 0 });
    m_container->setAnchorPoint({ 0, 1 });
    m_container->setContentSize({ width, 0 });
    this->addChild(m_container);

    updateContent();
    
    registerRichTextKey(std::make_shared<RichTextKey<ccColor4B>>(
        RichTextKeys::m_color,
        [](std::string value) -> Result<ccColor4B> {
            auto colorRes = cc4bFromHexString(value);
            if (colorRes.isErr()) return Err(colorRes.unwrapErr());

            return Ok(colorRes.unwrap());
        },
        [](ccColor4B const& value, cocos2d::CCNode* sprite, int localIndex, int charIndex) {
            if (auto rgba = typeinfo_cast<cocos2d::CCRGBAProtocol*>(sprite)) {
                auto baseOpacity = rgba->getOpacity();
                rgba->setColor(cocos2d::ccColor3B{ value.r, value.g, value.b });
                rgba->setOpacity(static_cast<GLubyte>(
                    (static_cast<unsigned int>(baseOpacity) * value.a + 127) / 255
                ));
            }
        }
    ));

    registerRichTextKey(std::make_shared<RichTextKey<bool>>(
        RichTextKeys::m_flip,
        [](std::string value) -> Result<bool> {
            if (value == "") return Ok(true);

            if (value != "true" && value != "false") {
                return Err("Value must be 'true' or 'false'");
            }

            return Ok(value == "true");
        },
        [](bool const& value, cocos2d::CCNode* sprite, int localIndex, int charIndex) {
            const float scale = std::abs(sprite->getScaleY());
            sprite->setScaleY(value ? -scale : scale);
        }
    ));

    registerRichTextKey(std::make_shared<RichTextKey<bool>>(
        RichTextKeys::m_mirror,
        [](std::string value) -> Result<bool> {
            if (value == "") return Ok(true);

            if (value != "true" && value != "false") {
                return Err("Value must be 'true' or 'false'");
            }

            return Ok(value == "true");
        },
        [](bool const& value, cocos2d::CCNode* sprite, int localIndex, int charIndex) {
            const float scale = std::abs(sprite->getScaleX());
            sprite->setScaleX(value ? -scale : scale);
        }
    ));

    registerRichTextKey(std::make_shared<RichTextKey<std::time_t>>(
        RichTextKeys::m_workingTime,
        [](std::string value) -> Result<std::time_t> {
            auto timeRes = geode::utils::numFromString<time_t>(value);
            if (timeRes.isErr()) return Ok(std::time(nullptr));

            return Ok(timeRes.unwrap());
        },
        [](std::time_t const& value) -> std::string {
            return fmt::format("{:%Y-%m-%d %H:%M:%S}", geode::localtime(value));
        }
    ));

    registerRichTextKey(std::make_shared<RichTextKey<float>>(
        RichTextKeys::m_size,
        [](std::string value) -> Result<float> {
            auto numRes = geode::utils::numFromString<float>(value);
            if (numRes.isErr()) return Err("size invalid!");

            return Ok(numRes.unwrap());
        },
        [](float const& value, cocos2d::CCNode* sprite, int localIndex, int charIndex) {
            sprite->setScale(value);
        }
    ));

    registerRichTextKey(std::make_shared<RichTextKey<float>>(
        RichTextKeys::m_spacing,
        [](std::string value) -> Result<float> {
            auto spacing = geode::utils::numFromString<float>(value);
            if (spacing.isErr() || !std::isfinite(spacing.unwrap())) {
                return Err("spacing invalid!");
            }

            return Ok(spacing.unwrap());
        },
        [](float const&, cocos2d::CCNode*, int, int) {}
    ));

    registerRichTextKey(std::make_shared<RichTextKey<std::string>>(
        RichTextKeys::m_font,
        [](std::string value) -> Result<std::string> {
            const auto hasFntExtension = value.size() >= 4
                && value[value.size() - 4] == '.'
                && std::tolower(static_cast<unsigned char>(value[value.size() - 3])) == 'f'
                && std::tolower(static_cast<unsigned char>(value[value.size() - 2])) == 'n'
                && std::tolower(static_cast<unsigned char>(value[value.size() - 1])) == 't';
            if (!hasFntExtension) {
                value += ".fnt";
            }

            if (!geode::Label::create(value.c_str())) {
                return Err("font invalid!");
            }

            return Ok(std::move(value));
        },
        [](std::string const&, cocos2d::CCNode*, int, int) {}
    ));

    auto makeQuickColorKey = [&](std::string const& key, cocos2d::ccColor3B const& color) {
        auto colorKey = std::make_shared<RichTextKey<std::monostate>>(
            key,
            [](std::string value) -> Result<std::monostate> {
                if (!value.empty()) return Err("Must be empty!");

                return Ok(std::monostate());
            },
            [color](std::monostate const& value, cocos2d::CCNode* sprite, int localIndex, int charIndex) {
                if (auto rgba = typeinfo_cast<cocos2d::CCRGBAProtocol*>(sprite)) {
                    rgba->setColor(color);
                }
            }
        );
        colorKey->setSpecialExits({"c"});
        
        registerRichTextKey(colorKey);
    };

    makeQuickColorKey(RichTextKeys::m_quickBlue, { 74, 82, 225 });
    makeQuickColorKey(RichTextKeys::m_quickGreen, { 64, 227, 72 });
    makeQuickColorKey(RichTextKeys::m_quickLightBlue, { 96, 171, 239 });
    makeQuickColorKey(RichTextKeys::m_quickCyan, { 50, 200, 255 });
    makeQuickColorKey(RichTextKeys::m_quickYellow, { 255, 255, 0 });
    makeQuickColorKey(RichTextKeys::m_quickOrange, { 255, 90, 75 });
    makeQuickColorKey(RichTextKeys::m_quickRed, { 255, 90, 90 });
    makeQuickColorKey(RichTextKeys::m_quickPurple, { 255, 0, 255 });
    makeQuickColorKey(RichTextKeys::m_quickViolet, { 150, 50, 255 });
    makeQuickColorKey(RichTextKeys::m_quickPink, { 255, 150, 255 });
    makeQuickColorKey(RichTextKeys::m_quickPaleYellow, { 255, 255, 150 });
    makeQuickColorKey(RichTextKeys::m_quickPaleCyan, { 150, 255, 255 });
    makeQuickColorKey(RichTextKeys::m_quickGold, { 255, 220, 65 });
    makeQuickColorKey(RichTextKeys::m_quickSpecialRed, { 255, 0, 0 });

    registerRichTextKey(std::make_shared<RichTextKey<std::string>>(
        RichTextKeys::m_link,
        [](std::string value) -> Result<std::string> {
            return Ok(value);
        },
        [&](std::string const& value, int keyState, bool hoverClickOn, cocos2d::CCNode* specificSpriteClicked, std::set<cocos2d::CCNode*> const& wordClicked) {
            this->processLinkClick(value, keyState, hoverClickOn, specificSpriteClicked, wordClicked);
        }
    ));

    updateContent();

    return true;
}

void RichTextArea::setFont(std::string font) {
    m_font = std::move(font);
    updateContainer();
}

std::string RichTextArea::getFont() {
    return m_font;
}

void RichTextArea::setColor(const cocos2d::ccColor4B& color) {
    m_color = color;
    updateContainer();
}

cocos2d::ccColor4B RichTextArea::getColor() {
    return m_color;
}

void RichTextArea::setAlignment(cocos2d::CCTextAlignment alignment) {
    m_alignment = alignment;
    updateContainer();
}

cocos2d::CCTextAlignment RichTextArea::getAlignment() {
    return m_alignment;
}

void RichTextArea::setWrappingMode(WrappingMode mode) {
    m_wrappingMode = mode;
    updateContainer();
}

WrappingMode RichTextArea::getWrappingMode() {
    return m_wrappingMode;
}

void RichTextArea::setText(std::string text) {
    m_text = std::move(text);
    m_rawText = m_text;
    formatRichText();
    updateContainer();
}

std::string RichTextArea::getText() {
    return m_text;
}

void RichTextArea::setMaxLines(size_t maxLines) {
    m_maxLines = maxLines;
    updateContainer();
}

size_t RichTextArea::getMaxLines() {
    return m_maxLines;
}

void RichTextArea::setWidth(float width) {
    m_artificialWidth = true;
    this->setContentSize({ width, this->getContentSize().height });
    m_container->setContentSize(this->getContentSize());
    updateContainer();
}

float RichTextArea::getWidth() {
    return m_container->getContentSize().width;
}

void RichTextArea::setScale(float scale) {
    m_scale = scale;
    updateContainer();
}

float RichTextArea::getScale() {
    return m_scale;
}

void RichTextArea::setLinePadding(float padding) {
    m_linePadding = padding;
    updateContainer();
}

float RichTextArea::getLinePadding() {
    return m_linePadding;
}

std::vector<geode::Label*> RichTextArea::getLines() {
    return m_lines;
}

float RichTextArea::getHeight() {
    return m_container->getContentSize().height;
}

float RichTextArea::getLineHeight() {
    return m_lineHeight;
}

std::string RichTextArea::getRawText(){
    return m_rawText;
}

void RichTextArea::updateContent(){
    formatRichText();
    updateContainer();
}

std::vector<std::shared_ptr<RichTextKeyBase>> RichTextArea::getTextKeys(){
    if (!m_richTextKeys.size()) return {};

    std::vector<std::shared_ptr<RichTextKeyBase>> toReturn{};
    toReturn.reserve(m_richTextKeys.size());

    for (auto const& [key, val] : m_richTextKeys) {
        toReturn.push_back(val);
    }

    return std::move(toReturn);
}

void RichTextArea::didUpdateContainer() {
    auto pendingEffects = std::move(m_pendingNodeEffects);
    for (const auto& effect : pendingEffects) {
        effect.m_instance->applyChangesToSprite(
            effect.m_node,
            effect.m_localIndex,
            static_cast<int>(effect.m_sourceCharacterIndex + 1)
        );
        if (effect.m_instance->isButton() && !effect.m_instance->isCancellation()) {
            m_charactersForButton[effect.m_instance].insert(effect.m_node);
        }
    }
}

void RichTextArea::formatRichText() {
    std::regex pattern(R"(<(\/)?([^=<>]+)(?:\s*=\s*([^<>]+))?>)");
    std::smatch match;

    m_richTextInstances.clear();
    m_specialExitCancellations.clear();

    // theres probably a better way to do this lol idk aa
    struct MatchInfo {
        int m_position;
        ptrdiff_t m_length;
        std::string m_key;
        std::string m_value;
        int m_overallOffset;
        bool m_cancellation;
    };
    std::vector<MatchInfo> matches;

    auto textToModify = m_rawText;

    auto begin = textToModify.cbegin();
    auto end = textToModify.cend();

    int offset = 0;

    while (std::regex_search(begin, end, match, pattern)) {
        int matchStartPos = std::distance(textToModify.cbegin(), match[0].first);

        std::string value = "";
        if (match.size() >= 4 && match[3].matched) value = match[3];

        matches.push_back({matchStartPos, match[0].length(), match[2], value, offset, match[1] == '/'});
        offset += match[0].length();

        begin = match.suffix().first;
    }

    std::map<int, std::vector<std::shared_ptr<RichTextKeyInstanceBase>>> richTextInstancesBeforeExtraOffset{};
    std::map<int, std::set<std::string>> specialExitCancellationsBeforeExtraOffset{};

    for (auto& m : asp::iter::fromReverse(matches)) {
        bool keyIsContained = m_richTextKeys.contains(m.m_key);

        if (!keyIsContained && m.m_cancellation) {
            const int effectIndex = m.m_position - m.m_overallOffset;
            for (const auto& [keyStr, keyBaseRef] : m_richTextKeys) {
                if (keyBaseRef && keyBaseRef->getSpecialExits().contains(m.m_key)) {
                    specialExitCancellationsBeforeExtraOffset[effectIndex].insert(keyStr);
                }
            }
        }
        else if (!keyIsContained) {
            continue;
        }
        else {
            int effectIndex = m.m_position - m.m_overallOffset;
            auto it = m_richTextKeys.find(m.m_key);
            if (it != m_richTextKeys.end() && it->second) {
                auto result = it->second->createInstance(m.m_value, m.m_cancellation);
                if (result.isOk()) {
                    richTextInstancesBeforeExtraOffset[effectIndex].push_back(result.unwrap());
                }
            }
        }

        textToModify.erase(m.m_position, m.m_length);
    }

    int textAdditionOverallOffset = 0;
    int prevExtraOffset = 0;

    std::set<int> effectIndices;
    for (const auto& [index, keys] : richTextInstancesBeforeExtraOffset) {
        effectIndices.insert(index);
    }
    for (const auto& [index, keys] : specialExitCancellationsBeforeExtraOffset) {
        effectIndices.insert(index);
    }

    for (const int index : effectIndices) {
        auto keyInstances = richTextInstancesBeforeExtraOffset.find(index);
        if (keyInstances != richTextInstancesBeforeExtraOffset.end()) {
            for (const auto& keyRef : keyInstances->second) {
                auto currentAddition = keyRef->runStrAddition();
                if (currentAddition.empty()) continue;
                textToModify.insert(index + textAdditionOverallOffset, currentAddition);
                textAdditionOverallOffset += currentAddition.length();
            }

            m_richTextInstances[index + prevExtraOffset] = std::move(keyInstances->second);
        }

        auto specialExit = specialExitCancellationsBeforeExtraOffset.find(index);
        if (specialExit != specialExitCancellationsBeforeExtraOffset.end()) {
            m_specialExitCancellations[index + prevExtraOffset] = std::move(specialExit->second);
        }

        prevExtraOffset = textAdditionOverallOffset;
    }

    m_text = std::move(textToModify);
}

void RichTextArea::processLinkClick(
    std::string const& link,
    int keyState,
    bool hoverClickOn,
    cocos2d::CCNode* specificSpriteClicked,
    std::set<cocos2d::CCNode*> const& wordClicked
){
    if (hoverClickOn){
        if (keyState == 1) {
            for (const auto& linkCharacter : wordClicked)
            {
                auto rgba = typeinfo_cast<cocos2d::CCRGBAProtocol*>(linkCharacter);
                if (!rgba) continue;

                this->m_ogColorForLink.insert({
                    linkCharacter,
                    rgba->getColor() == ccColor3B{255, 255, 255} ?
                        ccColor3B{getColor().r, getColor().g, getColor().b} :
                        rgba->getColor()
                    }
                );

                rgba->setColor(cocos2d::ccColor3B{ 78, 78, 255 });
            }
        }

        if (specificSpriteClicked != nullptr && keyState == 3)
            web::openLinkInBrowser(link);
    }

    if (!hoverClickOn || keyState == 3) {
        for (const auto& linkCharacter : wordClicked) {
            auto rgba = typeinfo_cast<cocos2d::CCRGBAProtocol*>(linkCharacter);
            if (rgba && this->m_ogColorForLink.contains(linkCharacter)) {
                rgba->setColor(this->m_ogColorForLink[linkCharacter]);
            }
        }
    }
}

void RichTextArea::registerRichTextKey(std::shared_ptr<RichTextKeyBase> key){
    if (this->m_richTextKeys.contains(key->getKey())) return;

    this->m_richTextKeys.insert({key->getKey(), key});

    this->formatRichText();
    this->updateContainer();
}

void RichTextArea::unregisterRichTextKey(std::string const& keyName) {
    if (!this->m_richTextKeys.contains(keyName)) return;

    this->m_richTextKeys.erase(keyName);
    
    this->formatRichText();
    this->updateContainer();
}

bool RichTextArea::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent){
    for (const auto& [btnKey, textForBtn] : this->m_charactersForButton)
    {
        for (const auto& fontSpr : textForBtn)
        {
            if (fontSpr == nullptr) continue;

            auto touchInSpace = fontSpr->convertTouchToNodeSpace(pTouch);
            if (touchInSpace.x > 0 && touchInSpace.y > 0 && touchInSpace.x <= fontSpr->getContentWidth() && touchInSpace.y <= fontSpr->getContentHeight()){
                btnKey->callButton(1, true, fontSpr, textForBtn);
                this->m_currentlyHeldButton = btnKey;
                return true;
            }
        }
    }

    return false;
}

void RichTextArea::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent){
    if (this->m_charactersForButton.contains(this->m_currentlyHeldButton)){
        auto textForBtn = this->m_charactersForButton[this->m_currentlyHeldButton];

        for (const auto& fontSpr : textForBtn)
        {
            if (fontSpr == nullptr) continue;

            auto touchInSpace = fontSpr->convertTouchToNodeSpace(pTouch);
            if (touchInSpace.x > 0 && touchInSpace.y > 0 && touchInSpace.x <= fontSpr->getContentWidth() && touchInSpace.y <= fontSpr->getContentHeight()){
                this->m_currentlyHeldButton->callButton(2, true, fontSpr, textForBtn);
                return;
            }
        }

        this->m_currentlyHeldButton->callButton(2, false, nullptr, textForBtn);
    }
}

void RichTextArea::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent){
    if (this->m_charactersForButton.contains(this->m_currentlyHeldButton)){
        auto textForBtn = this->m_charactersForButton[this->m_currentlyHeldButton];

        for (const auto& fontSpr : textForBtn)
        {
            if (fontSpr == nullptr) continue;

            auto touchInSpace = fontSpr->convertTouchToNodeSpace(pTouch);
            if (touchInSpace.x > 0 && touchInSpace.y > 0 && touchInSpace.x <= fontSpr->getContentWidth() && touchInSpace.y <= fontSpr->getContentHeight()){
                this->m_currentlyHeldButton->callButton(3, true, fontSpr, textForBtn);
                this->m_currentlyHeldButton = nullptr;
                return;
            }
        }

        this->m_currentlyHeldButton->callButton(3, false, nullptr, textForBtn);
    }

    this->m_currentlyHeldButton = nullptr;
}
void RichTextArea::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent){
    RichTextArea::ccTouchEnded(pTouch, pEvent);
}

RichTextArea::RichTextArea() = default;

RichTextArea::~RichTextArea() = default;

void RichTextArea::onEnter(){
    CCNode::onEnter();
    CCTouchDispatcher::get()->addTargetedDelegate(this, 0, true);
}

void RichTextArea::onExit(){
    CCTouchDispatcher::get()->removeDelegate(this);
    CCNode::onExit();
}