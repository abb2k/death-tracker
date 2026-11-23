#pragma once

#include <Geode/Geode.hpp>
#include <nodes/optionNodes/OptionsNode.hpp>

using namespace geode::prelude;

struct ConversionProgress{
    int maxAmount;
    int convertedAmount;
    int filesWentOverAmount;
    std::string currentConvertingFile;
};
struct ConversionResult{
    std::vector<std::string> failedConversions;
    int maxAmount;
    int convertedAmount;
};

using LookupFuture = arc::Future<std::vector<std::string>>;
using ConversionFuture = arc::Future<ConversionResult>;

class FileConversionLayer : public geode::Popup {
    public:
        static FileConversionLayer* create(bool exitIfNone);
    private:
        bool init(bool exitIfNone);

        SimpleTextArea* textArea;
        Slider* progressBar;

        LookupFuture lookupFuture();
        void lookupComplete(LookupFuture::Output event);
        async::TaskHolder<LookupFuture::Output> lookupListener;

        ConversionFuture conversionFuture(std::vector<std::string> allLevels, geode::Function<void(ConversionProgress)> progress);
        void conversionComplete(ConversionFuture::Output event);
        async::TaskHolder<ConversionFuture::Output> conversionListener;
        
        void setTitle(const std::string& text);

        void onClose(cocos2d::CCObject*) override;

        void onStart(cocos2d::CCObject*);

        LookupFuture::Output lookupResult;
        CCMenuItemSpriteExtra* startBtn;

        bool didComplete;

        bool exitIfNone;
};