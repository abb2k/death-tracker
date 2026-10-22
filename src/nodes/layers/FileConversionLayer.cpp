#include <nodes/layers/FileConversionLayer.hpp>
#include <managers/StatsManager.hpp>
#include <utils/Save.hpp>

FileConversionLayer* FileConversionLayer::create(bool exitIfNone) {
    auto popup = new FileConversionLayer;
    if (popup->init(exitIfNone)) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool FileConversionLayer::init(bool exitIfNone) {
    // @geode-ignore(unknown-resource)
    if (!Popup::init(250.f, 150.f, "geode.loader/GE_square01.png"))
        return false;
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    this->exitIfNone = exitIfNone;

    this->setTitle("Conversion Layer");

    textArea = SimpleTextArea::create("");
    textArea->setPosition(m_size / 2);
    textArea->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_mainLayer->addChild(textArea);

    progressBar = Slider::create(nullptr, NULL);
    progressBar->m_touchLogic->setVisible(false);
    progressBar->m_touchLogic->setEnabled(false);
    progressBar->setValue(0);
    progressBar->setVisible(false);
    progressBar->setPosition(m_size / 2 - ccp(0, 35));
    m_mainLayer->addChild(progressBar);

    this->setKeypadEnabled(true);
    this->setKeyboardEnabled(true);
    this->m_closeBtn->setVisible(false);

    auto startBtnSpr = ButtonSprite::create("Start");
    startBtnSpr->setScale(.75f);
    startBtn = CCMenuItemSpriteExtra::create(
        startBtnSpr,
        this,
        menu_selector(FileConversionLayer::onStart)
    );
    startBtn->setVisible(false);
    startBtn->setEnabled(false);
    startBtn->setPosition(m_size.width / 2, startBtn->getContentHeight() / 2 + 10);
    m_buttonMenu->addChild(startBtn);

    this->setTitle("Working...");
    textArea->setText("Looking for old save files...");

    lookupListener.spawn(
        "DT-convert-lookup-task",
        lookupFuture(),
        [&](LookupFuture::Output val) {
            this->lookupComplete(val);
        }
    );

    return true;
}

void FileConversionLayer::lookupComplete(LookupFuture::Output result) {
    lookupResult = std::move(result);
    if (!lookupResult.size()) {
        if (exitIfNone) {
            didComplete = true;
            keyBackClicked();
            return;
        }
        this->setTitle("No old files found!");
        textArea->setText("It looks like you're all set!\nNo old files were found!");
        if (!std::filesystem::exists(StatsManager::getSavesFolderPath())) {
            this->setTitle("Conversion Failed!");
            textArea->setText("Invalid save location\nPlease change your save location in\nThe death tracker mod settings");
        }
        this->m_closeBtn->setVisible(true);
        didComplete = true;
        return;
    }
    
    this->setTitle("Save Conversion");
    textArea->setText(fmt::format("{} old files from death tracker V2\nwere detected, we will convert them\nto the new version!", lookupResult.size()));
    startBtn->setVisible(true);
    startBtn->setEnabled(true);
}

void FileConversionLayer::setTitle(const std::string& text) {
    geode::Popup::setTitle(text, "goldFont.fnt", .8f, 15);
}

ConversionFuture FileConversionLayer::conversionFuture(std::vector<std::string> allLevels, geode::Function<void(ConversionProgress)> progress) {
    int filesWentOverAmount = 0;
    int successfulConversionAmount = 0;
    std::vector<std::string> failedConversions{};
    
    for (const auto& levelKey : allLevels)
    {
        auto progressObj = ConversionProgress{};
        progressObj.filesWentOverAmount = filesWentOverAmount;
        progressObj.convertedAmount = successfulConversionAmount;
        progressObj.maxAmount = allLevels.size();
        progressObj.currentConvertingFile = levelKey;
        
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
    conversionRes.convertedAmount = successfulConversionAmount;
    conversionRes.maxAmount = allLevels.size();
    conversionRes.failedConversions = failedConversions;
    
    co_return conversionRes;
}

LookupFuture FileConversionLayer::lookupFuture() {
    co_return StatsManager::allV2FileLevelKeys();
}

void FileConversionLayer::conversionComplete(ConversionFuture::Output output) {
    this->setTitle("Conversion complete!");
    textArea->setText(fmt::format(
        "Converted {}/{} files!\n{} files failed to convert.",
        output.convertedAmount,
        output.maxAmount,
        output.failedConversions.size()
    ));

    progressBar->setVisible(true);
    progressBar->setValue(1);

    for (const auto& level : output.failedConversions) log::error("Failed to convert level: {}", level);

    this->setKeypadEnabled(true);
    this->setKeyboardEnabled(true);
    this->m_closeBtn->setVisible(true);

    didComplete = true;
}

void FileConversionLayer::onClose(cocos2d::CCObject* sender) {
    if (didComplete) {
        Save::setDidConvertToV3(true);
        Popup::onClose(sender);
    }
}

void FileConversionLayer::onStart(cocos2d::CCObject* sender) {
    static_cast<CCMenuItemSpriteExtra*>(sender)->setVisible(false);
    static_cast<CCMenuItemSpriteExtra*>(sender)->setEnabled(false);

    conversionListener.spawn(
        "DT-convert-process-task",
        conversionFuture(lookupResult, [&](ConversionProgress progress) {
            geode::queueInMainThread([&, progress]() {
                progressBar->setVisible(true);
                float dev = static_cast<float>(progress.filesWentOverAmount) / static_cast<float>(progress.maxAmount);
                progressBar->setValue(dev);
                progressBar->updateBar();
                
                this->setTitle("Working...");
                textArea->setText(fmt::format(
                    "Converting files ({}/{})\nCurrently working on\n'{}'",
                    progress.convertedAmount,
                    progress.maxAmount,
                    progress.currentConvertingFile
                ));
            });
        }),
        [&](ConversionFuture::Output val) {
            this->conversionComplete(val);
        }
    );
}