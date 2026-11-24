#include "FileConversionLayer.hpp"

#include <managers/StatsManager.hpp>
#include <utils/Save.hpp>

FileConversionLayer* FileConversionLayer::create(bool exitIfNone) {
    auto popup = new FileConversionLayer();

    if (popup && popup->init(exitIfNone)) {
        popup->autorelease();
        return popup;
    }

    delete popup;
    return nullptr;
}

bool FileConversionLayer::init(bool exitIfNone) {
    if (!Popup::init(250.f, 150.f, "geode.loader/GE_square01.png"))
        return false;

    this->m_exitIfNone = exitIfNone;
    this->m_didComplete = false;

    this->setTitle("Conversion Layer");

    m_textArea = SimpleTextArea::create("");
    m_textArea->setPosition(m_size / 2);
    m_textArea->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_mainLayer->addChild(m_textArea);

    m_progressBar = Slider::create(nullptr, NULL);
    m_progressBar->m_touchLogic->setVisible(false);
    m_progressBar->m_touchLogic->setEnabled(false);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    m_progressBar->setPosition(m_size / 2 - ccp(0, 35));
    m_mainLayer->addChild(m_progressBar);

    this->setKeypadEnabled(true);
    this->setKeyboardEnabled(true);
    this->m_closeBtn->setVisible(false);

    auto startBtnSpr = ButtonSprite::create("Start");
    startBtnSpr->setScale(.75f);
    m_startBtn = CCMenuItemSpriteExtra::create(
        startBtnSpr,
        this,
        menu_selector(FileConversionLayer::onStart)
    );
    m_startBtn->setVisible(false);
    m_startBtn->setEnabled(false);
    m_startBtn->setPosition(m_size.width / 2, m_startBtn->getContentHeight() / 2 + 10);
    m_buttonMenu->addChild(m_startBtn);

    this->setTitle("Working...");
    m_textArea->setText("Looking for old save files...");

    m_lookupListener.spawn(
        "DT-convert-lookup-task",
        lookupFuture(),
        [&](LookupFuture::Output val) {
            this->lookupComplete(val);
        }
    );

    return true;
}

void FileConversionLayer::lookupComplete(LookupFuture::Output result) {
    m_lookupResult = std::move(result);
    if (!m_lookupResult.size()) {
        if (m_exitIfNone) {
            m_didComplete = true;
            keyBackClicked();
            return;
        }
        this->setTitle("No old files found!");
        m_textArea->setText("It looks like you're all set!\nNo old files were found!");
        if (!StatsManager::getSavesFolderPath().exists()) {
            this->setTitle("Conversion Failed!");
            m_textArea->setText("Invalid save location\nPlease change your save location in\nThe death tracker mod settings");
        }
        this->m_closeBtn->setVisible(true);
        m_didComplete = true;
        return;
    }
    
    this->setTitle("Save Conversion");
    m_textArea->setText(fmt::format("{} old files from death tracker V2\nwere detected, we will convert them\nto the new version!", m_lookupResult.size()));
    m_startBtn->setVisible(true);
    m_startBtn->setEnabled(true);
}

void FileConversionLayer::setTitle(std::string_view text) {
    geode::Popup::setTitle(std::string(text), "goldFont.fnt", .8f, 15);
}

ConversionFuture FileConversionLayer::conversionFuture(std::vector<std::string> allLevels, Function<void(ConversionProgress)> progress) {
    int filesWentOverAmount = 0;
    int successfulConversionAmount = 0;
    std::vector<std::string> failedConversions{};
    
    for (const auto& levelKey : allLevels)
    {
        auto progressObj = ConversionProgress{};
        progressObj.m_filesWentOverAmount = filesWentOverAmount;
        progressObj.m_convertedAmount = successfulConversionAmount;
        progressObj.m_maxAmount = allLevels.size();
        progressObj.m_currentConvertingFile = levelKey;
        
        progress(progressObj);
        
        auto conversionRes = StatsManager::convertV2SaveToV3(levelKey);
        
        if (conversionRes.isOk()) {
            successfulConversionAmount++;
        }
        else if (conversionRes.isErr()) {
            failedConversions.push_back(levelKey);
        }
        
        filesWentOverAmount++;
    }
    
    auto conversionRes = ConversionResult{};
    conversionRes.m_convertedAmount = successfulConversionAmount;
    conversionRes.m_maxAmount = allLevels.size();
    conversionRes.m_failedConversions = failedConversions;
    
    co_return conversionRes;
}

LookupFuture FileConversionLayer::lookupFuture() {
    co_return StatsManager::allV2FileLevelKeys();
}

void FileConversionLayer::conversionComplete(ConversionFuture::Output output) {
    this->setTitle("Conversion complete!");
    m_textArea->setText(fmt::format(
        "Converted {}/{} files!\n{} files failed to convert.",
        output.m_convertedAmount,
        output.m_maxAmount,
        output.m_failedConversions.size()
    ));

    m_progressBar->setVisible(true);
    m_progressBar->setValue(1);

    for (const auto& level : output.m_failedConversions) log::error("Failed to convert level: {}", level);

    this->setKeypadEnabled(true);
    this->setKeyboardEnabled(true);
    this->m_closeBtn->setVisible(true);

    m_didComplete = true;
}

void FileConversionLayer::onClose(cocos2d::CCObject* sender) {
    if (m_didComplete) {
        Save::setDidConvertToV3(true);
        Popup::onClose(sender);
    }
}

void FileConversionLayer::onStart(cocos2d::CCObject* sender) {
    static_cast<CCMenuItemSpriteExtra*>(sender)->setVisible(false);
    static_cast<CCMenuItemSpriteExtra*>(sender)->setEnabled(false);

    m_conversionListener.spawn(
        "DT-convert-process-task",
        conversionFuture(m_lookupResult, [&](ConversionProgress progress) {
            geode::queueInMainThread([&, progress]() {
                m_progressBar->setVisible(true);
                float dev = static_cast<float>(progress.m_filesWentOverAmount) / static_cast<float>(progress.m_maxAmount);
                m_progressBar->setValue(dev);
                m_progressBar->updateBar();
                
                this->setTitle("Working...");
                m_textArea->setText(fmt::format(
                    "Converting files ({}/{})\nCurrently working on\n'{}'",
                    progress.m_convertedAmount,
                    progress.m_maxAmount,
                    progress.m_currentConvertingFile
                ));
            });
        }),
        [&](ConversionFuture::Output val) {
            this->conversionComplete(val);
        }
    );
}