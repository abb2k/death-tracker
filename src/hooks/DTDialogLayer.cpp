#include "DTDialogLayer.hpp"

DialogLayer* DTDialogLayer::createWithTaggedSprites(CCArray* objects, int bg, std::vector<std::tuple<std::string, int, float>> icons) {
    objects->insertObject(DialogObject::create("", "", 0, 1, true, {0,0,0}), 0);
    
    auto ret = DialogLayer::createWithObjects(objects, bg);
    static_cast<DTDialogLayer*>(ret)->m_fields->m_doCallCreationCallback = true;
    static_cast<DTDialogLayer*>(ret)->useTaggedSprites(std::move(icons));
    ret->displayNextObject();

    return ret;
}

void DTDialogLayer::displayDialogObject(DialogObject* dialogue) {
    DialogLayer::displayDialogObject(dialogue);

    if (m_fields->m_callback != NULL)
        m_fields->m_callback(dialogue);

    std::string dtDialogSpr = "";
    std::tuple<std::string, int, float> dtDialogSprinfo;

    for (const auto& existingIcons : m_fields->m_icons)
    {
        if (std::get<1>(existingIcons) != dialogue->getTag()) continue;

        dtDialogSpr = std::get<0>(existingIcons);
        dtDialogSprinfo = existingIcons;
        break;
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

void DTDialogLayer::useTaggedSprites(std::vector<std::tuple<std::string, int, float>> icons) {
    m_fields->m_icons = std::move(icons);
}

void DTDialogLayer::setProgressCallback(Function<void(DialogObject*)> callback) {
    m_fields->m_callback = std::move(callback);
}

bool DTDialogLayer::init(DialogObject* object, CCArray* objects, int background) {
    if (object != nullptr) {
        m_fields->m_firstDialogue = object;
    }
    else if (objects != nullptr && objects->count() != 0) {
        m_fields->m_firstDialogue = static_cast<DialogObject*>(objects->firstObject());
    }

    if (!DialogLayer::init(object, objects, background)) return false;

    return true;
}

void DTDialogLayer::addToMainScene() {
    DialogLayer::addToMainScene();
    if (m_fields->m_callback != NULL && m_fields->m_firstDialogue != nullptr) {
        m_fields->m_callback(m_fields->m_firstDialogue);
    }
}