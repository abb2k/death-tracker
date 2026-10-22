#include "DTDialogLayer.hpp"

void DTDialogLayer::displayDialogObject(DialogObject* dialogue) {
    DialogLayer::displayDialogObject(dialogue);

    if (m_fields->callback != NULL)
        m_fields->callback(dialogue);

    std::string dtDialogSpr = "";
    std::tuple<std::string, int, float> dtDialogSprinfo;

    for (const auto& existingIcons : m_fields->icons)
    {
        // log::info("{} | {}", dialogue->getTag(), std::get<1>(existingIcons));
        if (std::get<1>(existingIcons) == dialogue->getTag()) {
            dtDialogSpr = std::get<0>(existingIcons);
            dtDialogSprinfo = existingIcons;
            break;
        }
    }

    if (dtDialogSpr != "" && m_characterSprite != nullptr) {
        m_characterSprite->setVisible(false);

        auto icon = m_mainLayer->getChildByID("dt_portrait"_spr);
        if (icon != nullptr) icon->removeMeAndCleanup();

        auto newIcon = CCSprite::createWithSpriteFrameName(dtDialogSpr.c_str());
        if (newIcon == nullptr) newIcon = CCSprite::create(dtDialogSpr.c_str());
        if (newIcon == nullptr) return;
        newIcon->setID("dt_portrait"_spr);
        newIcon->setPosition(m_characterSprite->getPosition());
        newIcon->setZOrder(4);
        newIcon->setScale(std::get<2>(dtDialogSprinfo));
        m_mainLayer->addChild(newIcon);
    }
    else {
        auto icon = m_mainLayer->getChildByID("dt_portrait"_spr);
        if (icon != nullptr) icon->setVisible(false);
    }
}

void DTDialogLayer::useTaggedSprites(const std::vector<std::tuple<std::string, int, float>>& icons) {
    m_fields->icons = icons;
}

DialogLayer* DTDialogLayer::createWithTaggedSprites(CCArray* objects, int bg, const std::vector<std::tuple<std::string, int, float>>& icons) {
    objects->insertObject(DialogObject::create("", "", 0, 1, true, {0,0,0}), 0);
    auto ret = DialogLayer::createWithObjects(objects, bg);
    static_cast<DTDialogLayer*>(ret)->m_fields->doCallCreationCallback = true;
    static_cast<DTDialogLayer*>(ret)->useTaggedSprites(icons);
    ret->displayNextObject();

    return ret;
}

void DTDialogLayer::setProgressCallback(geode::Function<void(DialogObject*)> callback) {
    m_fields->callback = std::move(callback);
}

bool DTDialogLayer::init(DialogObject* object, cocos2d::CCArray* objects, int background) {
    if (object != nullptr) {
        m_fields->firstDialogue = object;
    }
    else if (objects != nullptr && objects->count() != 0) {
        m_fields->firstDialogue = static_cast<DialogObject*>(objects->firstObject());
    }

    if (!DialogLayer::init(object, objects, background)) return false;

    return true;
}

void DTDialogLayer::addToMainScene() {
    DialogLayer::addToMainScene();
    if (m_fields->callback != NULL && m_fields->firstDialogue != nullptr) {
        m_fields->callback(m_fields->firstDialogue);
    }
}