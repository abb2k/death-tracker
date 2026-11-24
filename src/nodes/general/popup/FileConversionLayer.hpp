#pragma once

using namespace geode::prelude;

struct ConversionProgress {
    int m_maxAmount;
    int m_convertedAmount;
    int m_filesWentOverAmount;
    std::string m_currentConvertingFile;
};
struct ConversionResult {
    std::vector<std::string> m_failedConversions;
    int m_maxAmount;
    int m_convertedAmount;
};

using LookupFuture = arc::Future<std::vector<std::string>>;
using ConversionFuture = arc::Future<ConversionResult>;

class FileConversionLayer : public geode::Popup {
public:
    static FileConversionLayer* create(bool exitIfNone);

private:
    bool init(bool exitIfNone);

    SimpleTextArea* m_textArea;
    Slider* m_progressBar;

    LookupFuture lookupFuture();
    void lookupComplete(LookupFuture::Output event);
    async::TaskHolder<LookupFuture::Output> m_lookupListener;

    ConversionFuture conversionFuture(std::vector<std::string> allLevels, Function<void(ConversionProgress)> progress);
    void conversionComplete(ConversionFuture::Output event);
    async::TaskHolder<ConversionFuture::Output> m_conversionListener;
    
    void setTitle(std::string_view text);

    void onClose(cocos2d::CCObject*) override;

    void onStart(cocos2d::CCObject*);

    LookupFuture::Output m_lookupResult;
    CCMenuItemSpriteExtra* m_startBtn;

    bool m_didComplete;

    bool m_exitIfNone;
};