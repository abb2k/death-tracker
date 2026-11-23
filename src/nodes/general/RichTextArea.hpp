#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

template <class T>
class RichTextKey;

class RichTextKeyInstanceBase {
public:
    virtual ~RichTextKeyInstanceBase() = default;
    virtual void applyChangesToSprite(cocos2d::CCFontSprite* spr, int localIndex, int index) = 0;
    virtual std::string getKey() const = 0;
    virtual bool isCancellation() const = 0;
    virtual std::string runStrAddition() = 0;
    virtual bool isButton() const = 0;
    virtual void callButton(int keyState, bool hoverClickOn, cocos2d::CCFontSprite* spr, std::set<cocos2d::CCFontSprite*> const& word) = 0;
};

template <class T>
class RichTextKeyInstance final : public RichTextKeyInstanceBase {
public:
    RichTextKeyInstance(RichTextKey<T>* key, T data, bool cancellation)
        : m_key(std::move(key)), m_value(std::move(data)), m_cancellation(std::move(cancellation)) {}

    RichTextKey<T>* m_key = nullptr;
    T m_value;
    bool m_cancellation = false;

    void applyChangesToSprite(cocos2d::CCFontSprite* spr, int localIndex, int index) override {
        if (m_key->m_applyToSprite != NULL)
            m_key->m_applyToSprite(m_value, spr, localIndex, index);
    }

    std::string getKey() const override {
        return m_key->getKey();
    }

    bool isCancellation() const override {
        return m_cancellation;
    }

    std::string runStrAddition() override {
        if (m_key->m_stringAddition != NULL)
            return m_key->m_stringAddition(m_value);
        return "";
    }

    bool isButton() const override {
        return m_key->m_buttonFunctionallity != NULL;
    }

    void callButton(int keyState, bool hoverClickOn, cocos2d::CCFontSprite* spr, std::set<cocos2d::CCFontSprite*> const& word) override {
        if (m_key->m_buttonFunctionallity != NULL)
            m_key->m_buttonFunctionallity(m_value, keyState, hoverClickOn, spr, word);
    }
};

class RichTextKeyBase {
public:
    virtual ~RichTextKeyBase() = default;
    virtual Result<std::shared_ptr<RichTextKeyInstanceBase>> createInstance(std::string const& value, bool cancellation) = 0;
    virtual std::string getKey() const = 0;
    virtual std::set<std::string> getSpecialExits() const = 0;
};

template <class T>
class RichTextKey final : public RichTextKeyBase {
public:
    using ValidationCheck = geode::Function<Result<T>(std::string const& value)>;
    using ApplyToSprite = geode::Function<void(T const& value, cocos2d::CCFontSprite* sprite, int localIndex, int charIndex)>;
    using StringAddition = geode::Function<std::string(T const& value)>;
    using ButtonFunctionallity = geode::Function<void(
            T const& value,
            int keyState,
            bool hoverClickOn,
            cocos2d::CCFontSprite* specificSpriteClicked,
            std::set<cocos2d::CCFontSprite*> const& wordClicked)>;

    ValidationCheck m_validCheck = NULL;
    ApplyToSprite m_applyToSprite = NULL;
    StringAddition m_stringAddition = NULL;
    ButtonFunctionallity m_buttonFunctionallity = NULL;

    RichTextKey(
        std::string key,
        ValidationCheck validCheck,
        ButtonFunctionallity buttonFunctionallity
    ) : m_key(std::move(key)),
        m_validCheck(std::move(validCheck)),
        m_buttonFunctionallity(std::move(buttonFunctionallity)) {}
    
    RichTextKey(
        std::string key,
        ValidationCheck validCheck,
        ApplyToSprite applyToSprite
    ) : m_key(std::move(key)),
        m_validCheck(std::move(validCheck)),
        m_applyToSprite(std::move(applyToSprite)) {}
    
    RichTextKey(
        std::string key,
        ValidationCheck validCheck,
        StringAddition stringAddition
    ) : m_key(std::move(key)),
        m_validCheck(std::move(validCheck)),
        m_stringAddition(std::move(stringAddition)) {}

    RichTextKey(
        std::string key,
        ValidationCheck validCheck,
        ApplyToSprite applyToSprite,
        StringAddition stringAddition
    ) : m_key(std::move(key)),
        m_validCheck(std::move(validCheck)),
        m_applyToSprite(std::move(applyToSprite)),
        m_stringAddition(std::move(stringAddition)) {}

    Result<std::shared_ptr<RichTextKeyInstanceBase>> createInstance(std::string const& value, bool cancellation) override {
        if (cancellation){
            if (value == ""){
                return Ok(std::make_shared<RichTextKeyInstance<T>>(
                    RichTextKeyInstance<T>(this, T(), true))
                );
            }
            else return Err("Cancellation tags cannot have values");
        }

        auto res = m_validCheck(value);

        if (res.isErr()) return Err(res.unwrapErr());

        return Ok(std::make_shared<RichTextKeyInstance<T>>(
            RichTextKeyInstance<T>(this, res.unwrap(), false))
        );
    }

    std::string getKey() const override {
        return m_key;
    }

    std::set<std::string> getSpecialExits() const override {
        return m_specialExits;
    }

    void setSpecialExits(std::set<std::string> specialExists) {
        m_specialExits = std::move(specialExists);
    }

private:
    std::string m_key;
    std::set<std::string> m_specialExits{};
};


class RichTextArea : public cocos2d::CCNode, public cocos2d::CCTouchDelegate {
public:
    static RichTextArea* create(std::string text, std::string font = "chatFont.fnt", float scale = 1.0f);
    static RichTextArea* create(std::string text, std::string font, float scale, float width);

    void setFont(std::string font);
    std::string getFont();
    void setColor(const cocos2d::ccColor4B& color);
    cocos2d::ccColor4B getColor();
    void setAlignment(cocos2d::CCTextAlignment alignment);
    cocos2d::CCTextAlignment getAlignment();
    void setWrappingMode(WrappingMode mode);
    WrappingMode getWrappingMode();
    void setText(std::string text);
    std::string getText();
    void setMaxLines(size_t maxLines);
    size_t getMaxLines();
    void setWidth(float width);
    float getWidth();
    void setScale(float scale) override;
    float getScale() override;
    void setLinePadding(float padding);
    float getLinePadding();
    std::vector<cocos2d::CCLabelBMFont*> getLines();
    float getHeight();
    float getLineHeight();

    std::string getRawText();

    void updateContent();

    std::vector<std::shared_ptr<RichTextKeyBase>> getTextKeys();

    void registerRichTextKey(std::shared_ptr<RichTextKeyBase> key);
    void unregisterRichTextKey(std::string const& keyName);

protected:
    RichTextArea();
    ~RichTextArea() override;

    bool init(std::string font, std::string text, float scale, float width, const bool artificialWidth);
private:
    static RichTextArea* create(std::string font, std::string text, float scale, float width, const bool artificialWidth);

    bool m_shouldUpdate = false;
    bool m_artificialWidth = false;
    cocos2d::CCMenu* m_container = nullptr;
    std::string m_font;
    std::string m_text;
    std::vector<cocos2d::CCLabelBMFont*> m_lines;
    cocos2d::ccColor4B m_color = { 0xFF, 0xFF, 0xFF, 0xFF };
    cocos2d::CCTextAlignment m_alignment = cocos2d::kCCTextAlignmentLeft;
    geode::WrappingMode m_wrappingMode = geode::WrappingMode::WORD_WRAP;
    size_t m_maxLines = 0;
    float m_scale = 1.f;
    float m_lineHeight = 0.f;
    float m_linePadding = 0.f;

    cocos2d::CCLabelBMFont* createLabel(std::string text, float top);

    float calculateOffset(cocos2d::CCLabelBMFont* label);

    void charIteration(geode::FunctionRef<cocos2d::CCLabelBMFont*(cocos2d::CCLabelBMFont* line, char c, float top)> overflowHandling);

    void updateLinesNoWrap();

    void updateLinesWordWrap(bool spaceWrap);

    void updateLinesCutoffWrap();

    void updateContainer();

    bool ccTouchBegan(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent) override;
    void ccTouchMoved(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent) override;
    void ccTouchEnded(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent) override;
    void ccTouchCancelled(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent) override;

    void onEnter() override;
    void onExit() override;

    std::map<std::string, std::shared_ptr<RichTextKeyBase>> m_richTextKeys;
    std::map<int, std::vector<std::shared_ptr<RichTextKeyInstanceBase>>> m_richTextInstances;
    std::map<int, std::set<std::string>> m_specialExitCancellations;
    std::map<std::shared_ptr<RichTextKeyInstanceBase>, std::set<CCFontSprite*>> m_charactersForButton{};
    std::shared_ptr<RichTextKeyInstanceBase> m_currentlyHeldButton = nullptr;

    struct PendingSpriteEffect {
        CCFontSprite* sprite;
        std::shared_ptr<RichTextKeyInstanceBase> instance;
        int localIndex;
        size_t sourceCharacterIndex;
    };
    std::vector<PendingSpriteEffect> m_pendingSpriteEffects;

    std::string m_rawText;

    std::map<CCFontSprite*, ccColor3B> m_ogColorForLink{};

    void didUpdateContainer();
    void formatRichText();

    void processLinkClick(
        std::string const& link,
        int keyState,
        bool hoverClickOn,
        cocos2d::CCFontSprite* specificSpriteClicked,
        std::set<cocos2d::CCFontSprite*> const& wordClicked
    );
};