#include "DTDialogLayer.hpp"

#include <timestepyt.heisbehindthetree/api/API.hpp>

void DTDialogLayer::addStoredEggBadge(
    CCNode* dialogueMainLayer,
    CCScale9Sprite* dialogueBackground,
    bool isDeltaruneTextboxes
) {
    if (dialogueMainLayer == nullptr || dialogueBackground == nullptr ||
        !Loader::get()->isModLoaded("timestepyt.heisbehindthetree") ||
        !heisbehindthetree::wasEggStored().unwrapOr(false) ||
        (!isDeltaruneTextboxes && !Mod::get()->getSavedValue<bool>("eggTutorialCompleted", false)))
        return;

    if (dialogueMainLayer->getChildByID("dt_egg_badge"_spr) != nullptr)
        return;

    // @geode-ignore(unknown-resource)
    auto egg = CCSprite::create("timestepyt.heisbehindthetree/egg.png");
    if (egg == nullptr) {
        return;
    }

    auto backgroundBounds = dialogueBackground->boundingBox();
    auto backgroundSize = backgroundBounds.size;
    auto eggSize = egg->getContentSize();
    if (backgroundSize.width <= 0.f || backgroundSize.height <= 0.f || eggSize.height <= 0.f) {
        return;
    }

    egg->setScale(backgroundSize.height * (isDeltaruneTextboxes ? 0.2f : 0.25f) / eggSize.height);
    auto badgeSize = eggSize * egg->getScale();
    auto padding = isDeltaruneTextboxes ? 26.f : 8.f;
    egg->setPosition({
        backgroundBounds.origin.x + padding + badgeSize.width * 0.5f,
        backgroundBounds.origin.y + padding + badgeSize.height * 0.5f
    });
    egg->setID("dt_egg_badge"_spr);
    dialogueMainLayer->addChild(egg, 10);
}

DialogLayer* DTDialogLayer::createWithTaggedSprites(CCArray* objects, int bg, std::vector<std::tuple<std::string, int, float>> icons) {
    auto const hasDeltaruneTextboxes = Loader::get()->isModLoaded("timestepyt.deltarune_textboxes");
    if (!hasDeltaruneTextboxes)
        objects->insertObject(DialogObject::create("", "", 0, 1, true, {0,0,0}), 0);
    
    auto ret = DialogLayer::createWithObjects(objects, bg);
    static_cast<DTDialogLayer*>(ret)->m_fields->m_doCallCreationCallback = true;
    static_cast<DTDialogLayer*>(ret)->useTaggedSprites(std::move(icons));
    if (!hasDeltaruneTextboxes)
        ret->displayNextObject();

    return ret;
}

void DTDialogLayer::displayDialogObject(DialogObject* dialogue) {
    DialogLayer::displayDialogObject(dialogue);

    if (m_fields->m_hideCharacter) {
        if (m_characterLabel != nullptr) m_characterLabel->setVisible(false);
        if (m_characterSprite != nullptr) m_characterSprite->setVisible(false);
    }

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
        auto background = static_cast<CCScale9Sprite*>(m_mainLayer->getChildren()->objectAtIndex(0));
        addStoredEggBadge(m_mainLayer, background);
        
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

void DTDialogLayer::setProgressOnZ(bool enabled) {
    m_fields->m_progressOnZ = enabled;
}

void DTDialogLayer::setCharacterHidden(bool hidden) {
    m_fields->m_hideCharacter = hidden;
}

void DTDialogLayer::setFastForwardDisabled(bool disabled) {
    m_fields->m_disableFastForward = disabled;
}

void DTDialogLayer::handleDialogTap() {
    if (m_fields->m_disableFastForward && m_animating && m_skippable)
        return;

    DialogLayer::handleDialogTap();
}

void DTDialogLayer::keyDown(enumKeyCodes key, double timestamp) {
    if (m_fields->m_progressOnZ && key == KEY_Z) {
        this->handleDialogTap();
        return;
    }

    DialogLayer::keyDown(key, timestamp);
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