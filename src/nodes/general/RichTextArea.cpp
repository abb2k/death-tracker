#include "RichTextArea.hpp"

#include <regex>
#include <asp/iter.hpp>

cocos2d::CCLabelBMFont* RichTextArea::createLabel(std::string text, float top) {
    if (m_maxLines && m_lines.size() >= m_maxLines) {
        cocos2d::CCLabelBMFont* last = m_lines.at(m_maxLines - 1);
        std::string_view textv = last->getString();

        last->setString(fmt::format("{}...", textv.substr(0, textv.size() - 3)).c_str());

        return nullptr;
    } else {
        cocos2d::CCLabelBMFont* label = cocos2d::CCLabelBMFont::create(std::move(text).c_str(), m_font.c_str());
        
        label->setScale(m_scale);
        label->setPosition({ 0, top });
        label->setColor({ m_color.r, m_color.g, m_color.b });
        label->setOpacity(m_color.a);

        return label;
    }
}

float RichTextArea::calculateOffset(cocos2d::CCLabelBMFont* label) {
    return m_linePadding + label->getContentSize().height * m_scale;
}

void RichTextArea::charIteration(geode::FunctionRef<cocos2d::CCLabelBMFont*(cocos2d::CCLabelBMFont* line, char c, float top)> overflowHandling) {
    float top = 0;
    m_lines.clear();
    m_pendingSpriteEffects.clear();
    cocos2d::CCLabelBMFont* line = this->createLabel("", top);
    m_lines = { line };

    std::map<std::string, std::shared_ptr<RichTextKeyInstanceBase>> appliedRichTextInstances{};
    std::map<std::string, size_t> activationOrder;
    size_t nextActivationOrder = 0;

    std::map<std::shared_ptr<RichTextKeyInstanceBase>, int> m_charIndexForInstance{};

    using CharacterEffect = std::pair<std::shared_ptr<RichTextKeyInstanceBase>, int>;

    std::vector<std::vector<CharacterEffect>> effectsForCharacter;
    std::map<cocos2d::CCLabelBMFont*, std::vector<size_t>> characterIndicesForLine{};
    characterIndicesForLine[line] = {};

    m_charactersForButton.clear();

    size_t index = 0;
    for (const char& c : m_text) {
        if (m_richTextInstances.contains(index)) {
            auto instances = m_richTextInstances[index];

            for (const auto& instance : instances) {
                auto key = instance->getKey();

                if (instance->isCancellation()) {
                    auto active = appliedRichTextInstances.find(key);
                    
                    if (active != appliedRichTextInstances.end()) {
                        m_charIndexForInstance.erase(active->second);
                        appliedRichTextInstances.erase(active);
                        activationOrder.erase(key);
                    }

                    continue;
                }

                appliedRichTextInstances[key] = instance;
                m_charIndexForInstance[instance] = 0;
                activationOrder[key] = nextActivationOrder++;
            }
        }

        if (m_specialExitCancellations.contains(index)) {
            const std::string* latestKey = nullptr;
            size_t latestOrder = 0;
            
            for (const auto& key : m_specialExitCancellations[index]) {
                auto active = appliedRichTextInstances.find(key);
                auto order = activationOrder.find(key);
                
                if (active != appliedRichTextInstances.end() && order != activationOrder.end() && (latestKey == nullptr || order->second > latestOrder)) {
                    latestKey = &key;
                    latestOrder = order->second;
                }
            }

            if (latestKey != nullptr) {
                auto active = appliedRichTextInstances.find(*latestKey);
                m_charIndexForInstance.erase(active->second);
                appliedRichTextInstances.erase(active);
                activationOrder.erase(*latestKey);
            }
        }

        std::vector<CharacterEffect> characterEffects{};
        characterEffects.reserve(appliedRichTextInstances.size());

        for (const auto& [key, instancePtr] : appliedRichTextInstances) {
            characterEffects.emplace_back(instancePtr, m_charIndexForInstance[instancePtr]);
        }
        
        effectsForCharacter.push_back(std::move(characterEffects));
        auto characterIndex = index;
        index++;

        if (c == '\n') {
            line = this->createLabel("", top -= this->calculateOffset(line));

            if (line == nullptr) {
                break;
            }
            else {
                m_lines.push_back(line);
                characterIndicesForLine[line] = {};
            }
        }
        else if (m_artificialWidth && line->getContentWidth() * m_scale >= getWidth()) {
            cocos2d::CCLabelBMFont* oldLine = line;
            std::string oldText = line->getString();
            auto oldCharacterIndices = characterIndicesForLine[line];
            line = overflowHandling(line, c, top -= this->calculateOffset(line));

            if (line == nullptr) {
                break;
            }
            else {
                m_lines.push_back(line);

                std::string newText = line->getString();
                bool currentCharacterWasRendered = c != ' ' && !newText.empty() && newText.back() == c;

                auto movedText = newText;
                
                if (currentCharacterWasRendered) {
                    movedText.pop_back();
                }

                auto movedCharacterCount = std::min(oldText.size(), movedText.size());
                while (movedCharacterCount > 0 && oldText.compare(oldText.size() - movedCharacterCount, movedCharacterCount, movedText, 0, movedCharacterCount) != 0) {
                    movedCharacterCount--;
                }
                movedCharacterCount = std::min(movedCharacterCount, oldCharacterIndices.size());

                std::vector<size_t>& remainingCharacterIndices = characterIndicesForLine[oldLine];
                remainingCharacterIndices.resize(std::min(oldCharacterIndices.size(), std::string(oldLine->getString()).size()));

                std::vector<size_t>& newCharacterIndices = characterIndicesForLine[line];
                newCharacterIndices.insert(
                    newCharacterIndices.end(),
                    oldCharacterIndices.end() - movedCharacterCount,
                    oldCharacterIndices.end()
                );

                if (currentCharacterWasRendered) {
                    newCharacterIndices.push_back(characterIndex);
                }

                if (currentCharacterWasRendered) {
                    for (const auto& [key, instancePtr] : appliedRichTextInstances) {
                        m_charIndexForInstance[instancePtr]++;
                    }
                }
            }
        }
        else {
            line->setString((std::string(line->getString()) + c).c_str());

            characterIndicesForLine[line].push_back(characterIndex);
            if (line->getChildren()->lastObject() != nullptr) {
                for (const auto& [key, instancePtr] : appliedRichTextInstances) {
                    m_charIndexForInstance[instancePtr]++;
                }
            }
        }
    }

    for (auto* line : m_lines) {
        auto const& characterIndices = characterIndicesForLine[line];
        auto children = line->getChildren();
        auto childCount = std::min<size_t>(children->count(), characterIndices.size());

        for (size_t i = 0; i < childCount; i++) {
            auto* sprite = static_cast<cocos2d::CCFontSprite*>(children->objectAtIndex(i));
            const size_t sourceCharacterIndex = characterIndices[i];
            for (const auto& [instancePtr, localIndex] : effectsForCharacter[sourceCharacterIndex]) {
                m_pendingSpriteEffects.push_back({
                    sprite,
                    instancePtr,
                    localIndex,
                    sourceCharacterIndex
                });
            }
        }
    }
}

void RichTextArea::updateLinesNoWrap() {
    std::stringstream stream(m_text);
    std::string part;
    float top = 0;
    m_lines.clear();

    while (std::getline(stream, part)) {
        cocos2d::CCLabelBMFont* line = createLabel(std::move(part), top);
        part.clear();

        if (line == nullptr) {
            break;
        } else {
            top -= calculateOffset(line);

            m_lines.push_back(line);
        }
    }
}

void RichTextArea::updateLinesWordWrap(bool spaceWrap) {
    charIteration([this, spaceWrap](cocos2d::CCLabelBMFont* line, char c, float top) {
        const std::string_view delimiters(spaceWrap ? " " : " `~!@#$%^&*()-_=+[{}];:'\",<.>/?\\|");

        if (!delimiters.contains(c)) {
            const std::string& text = line->getString();
            const size_t position = text.find_last_of(delimiters) + 1;
            cocos2d::CCLabelBMFont* newLine = createLabel(text.substr(position) + c, top);

            if (newLine != nullptr) {
                line->setString(text.substr(0, position).c_str());
            }

            return newLine;
        } else {
            return createLabel(std::string(c != ' ', c).c_str(), top);
        }
    });
}

void RichTextArea::updateLinesCutoffWrap() {
    charIteration([this](cocos2d::CCLabelBMFont* line, char c, float top) {
        const std::string& text = line->getString();
        const char back = text.back();
        const bool lastIsSpace = back == ' ';
        cocos2d::CCLabelBMFont* newLine = createLabel(std::string(!lastIsSpace, back).append(1, c).c_str(), top);

        if (newLine != nullptr && !lastIsSpace) {
            line->setString(text.substr(0, text.size() - 1).c_str());
        } else if (newLine == nullptr && !lastIsSpace) {
            if (text[text.size() - 2] == ' ') {
                line->setString(text.substr(0, text.size() - 1).c_str());
            } else {
                line->setString((text.substr(0, text.size() - 1) + '-').c_str());
            }
        }

        return newLine;
    });
}

void RichTextArea::updateContainer() {
    switch (m_wrappingMode) {
        case geode::NO_WRAP: {
            updateLinesNoWrap();
        } break;
        case geode::WORD_WRAP: {
            updateLinesWordWrap(false);
        } break;
        case geode::SPACE_WRAP: {
            updateLinesWordWrap(true);
        } break;
        case geode::CUTOFF_WRAP: {
            updateLinesCutoffWrap();
        } break;
    }

    const size_t lineCount = m_lines.size();
    const float width = getWidth();

    if (lineCount > 0) {
        m_lineHeight = m_lines.back()->getContentSize().height * m_scale;
    } else {
        m_lineHeight = 0;
    }

    const float height = m_lineHeight * lineCount + m_linePadding * (lineCount - 1);

    setContentSize({ width, height });
    m_container->setContentSize(getContentSize());
    m_container->removeAllChildren();

    for (cocos2d::CCLabelBMFont* line : m_lines) {
        const float y = height + line->getPositionY();

        switch (m_alignment) {
            case cocos2d::kCCTextAlignmentLeft: {
                line->setAnchorPoint({ 0, 1 });
                line->setPosition({ 0, y });
            } break;
            case cocos2d::kCCTextAlignmentCenter: {
                line->setAnchorPoint({ 0.5f, 1 });
                line->setPosition({ width / 2, y });
            } break;
            case cocos2d::kCCTextAlignmentRight: {
                line->setAnchorPoint({ 1, 1 });
                line->setPosition({ width, y });
            } break;
        }

        m_container->addChild(line);
    }

    didUpdateContainer();
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
        "color",
        [](std::string value) -> Result<ccColor4B> {
            auto colorRes = cc4bFromHexString(value);
            if (colorRes.isErr()) return Err(colorRes.unwrapErr());

            return Ok(colorRes.unwrap());
        },
        [](ccColor4B const& value, cocos2d::CCFontSprite* sprite, int localIndex, int charIndex) {
            sprite->setColor({ value.r, value.g, value.b });
            sprite->setOpacity(value.a);
        }
    ));

    registerRichTextKey(std::make_shared<RichTextKey<bool>>(
        "flip",
        [](std::string value) -> Result<bool> {
            if (value == "") return Ok(true);

            if (value != "true" && value != "false") {
                return Err("Value must be 'true' or 'false'");
            }

            return Ok(value == "true");
        },
        [](bool const& value, cocos2d::CCFontSprite* sprite, int localIndex, int charIndex) {
            sprite->setFlipY(value);
        }
    ));

    registerRichTextKey(std::make_shared<RichTextKey<bool>>(
        "mirror",
        [](std::string value) -> Result<bool> {
            if (value == "") return Ok(true);

            if (value != "true" && value != "false") {
                return Err("Value must be 'true' or 'false'");
            }

            return Ok(value == "true");
        },
        [](bool const& value, cocos2d::CCFontSprite* sprite, int localIndex, int charIndex) {
            sprite->setFlipX(value);
        }
    ));

    registerRichTextKey(std::make_shared<RichTextKey<std::time_t>>(
        "workingTime",
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
        "size",
        [](std::string value) -> Result<float> {
            auto numRes = geode::utils::numFromString<float>(value);
            if (numRes.isErr()) return Err("size invalid!");

            return Ok(numRes.unwrap());
        },
        [](float const& value, cocos2d::CCFontSprite* sprite, int localIndex, int charIndex) {
            sprite->setScale(value);
        }
    ));

    auto makeQuickColorKey = [&](std::string const& key, cocos2d::ccColor3B const& color) {
        auto colorKey = std::make_shared<RichTextKey<std::monostate>>(
            key,
            [](std::string value) -> Result<std::monostate> {
                if (!value.empty()) return Err("Must be empty!");

                return Ok(std::monostate());
            },
            [color](std::monostate const& value, cocos2d::CCFontSprite* sprite, int localIndex, int charIndex) {
                sprite->setColor(color);
            }
        );
        colorKey->setSpecialExits({"c"});
        
        registerRichTextKey(colorKey);
    };

    makeQuickColorKey("cb", { 74, 82, 225 });
    makeQuickColorKey("cg", { 64, 227, 72 });
    makeQuickColorKey("cl", { 96, 171, 239 });
    makeQuickColorKey("cj", { 50, 200, 255 });
    makeQuickColorKey("cy", { 255, 255, 0 });
    makeQuickColorKey("co", { 255, 90, 75 });
    makeQuickColorKey("cr", { 255, 90, 90 });
    makeQuickColorKey("cp", { 255, 0, 255 });
    makeQuickColorKey("ca", { 150, 50, 255 });
    makeQuickColorKey("cd", { 255, 150, 255 });
    makeQuickColorKey("cc", { 255, 255, 150 });
    makeQuickColorKey("cf", { 150, 255, 255 });
    makeQuickColorKey("cs", { 255, 220, 65 });
    makeQuickColorKey("c_", { 255, 0, 0 });

    // maybe one day someone will make this work but its fine :(
    // registerRichTextKey(std::make_shared<RichTextKey<std::string>>(
    //     "font",
    //     [](std::string value) -> Result<std::string> {
    //         auto temp = cocos2d::CCLabelBMFont::create("Test", value.c_str());
    //         if (temp == nullptr) return Err("Font file isn't valid!");

    //         return Ok(value);
    //     },
    //     [&](std::string const& value, CCFontSprite* const applyToSpr) {
    //         applyToSpr->retain();
    //         applyToSpr->removeFromParent();
    //         this->addChild(applyToSpr);
    //         applyToSpr->release();
    //     }
    // ));

    registerRichTextKey(std::make_shared<RichTextKey<std::string>>(
        "link",
        [](std::string value) -> Result<std::string> {
            return Ok(value);
        },
        [&](std::string const& value, int keyState, bool hoverClickOn, cocos2d::CCFontSprite* specificSpriteClicked, std::set<cocos2d::CCFontSprite*> const& wordClicked) {
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
    updateContainer();

    this->setContentSize({ width, this->getContentSize().height });
    m_container->setContentSize(this->getContentSize());
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

std::vector<cocos2d::CCLabelBMFont*> RichTextArea::getLines() {
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
    auto pendingSpriteEffects = std::move(m_pendingSpriteEffects);
    for (const auto& effect : pendingSpriteEffects) {
        effect.m_instance->applyChangesToSprite(
            effect.m_sprite,
            effect.m_localIndex,
            static_cast<int>(effect.m_sourceCharacterIndex + 1)
        );

        if (effect.m_instance->isButton() && !effect.m_instance->isCancellation()) {
            m_charactersForButton[effect.m_instance].insert(effect.m_sprite);
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
    cocos2d::CCFontSprite* specificSpriteClicked,
    std::set<cocos2d::CCFontSprite*> const& wordClicked
){
    if (hoverClickOn){
        if (keyState == 1) {
            for (const auto& linkCharacter : wordClicked)
            {
                this->m_ogColorForLink.insert({
                    linkCharacter,
                    linkCharacter->getColor() == ccColor3B{255, 255, 255} ?
                        ccColor3B{getColor().r, getColor().g, getColor().b} :
                        linkCharacter->getColor()
                    }
                );

                linkCharacter->setColor({ 78, 78, 255 });
            }
        }

        if (specificSpriteClicked != nullptr && keyState == 3)
            web::openLinkInBrowser(link);
    }

    if (!hoverClickOn || keyState == 3) {
        for (const auto& linkCharacter : wordClicked) {
            if (this->m_ogColorForLink.contains(linkCharacter)) {
                linkCharacter->setColor(this->m_ogColorForLink[linkCharacter]);
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