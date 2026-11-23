#pragma once

#include <Geode/Geode.hpp>
#include <type_traits>

using namespace geode::prelude;

template<typename E>
struct NamedEnumValue{
    E value;
    std::string name;

    NamedEnumValue(E value, std::string name)
        : value(value), name(std::move(name)) {}
};

template<typename E>
class OptionSwitcher : public CCMenu {
    protected:
        bool init(float width, const std::vector<NamedEnumValue<E>>& options);

    public:
        static OptionSwitcher<E>* create(float width, const std::vector<NamedEnumValue<E>>& options);

        E getSelectedOption();

        void setCallback(Function<void(E)> callback);

        void setValue(int index, bool runCallback = false);
        void setValue(E e, bool runCallback = false);

    private:
        std::vector<geode::Label*> labels{};
        std::vector<E> values{};

        void onLabelClicked(CCObject* sender);

        int currentOption = 0;

        Function<void(E)> callback = nullptr;
};


template<typename E>
OptionSwitcher<E>* OptionSwitcher<E>::create(float width, const std::vector<NamedEnumValue<E>>& options) {
    auto ret = new OptionSwitcher<E>();
    if (ret && ret->init(width, options)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

template<typename E>
bool OptionSwitcher<E>::init(float width, const std::vector<NamedEnumValue<E>>& options) {
    if (!CCMenu::init()) return false;

    this->setContentSize({ width, 30 });
    this->setPosition({ 0, 0 });
    this->ignoreAnchorPointForPosition(false);

    auto bg = CCScale9Sprite::create("square02_small.png");
    bg->setContentSize(this->getContentSize());
    bg->setAnchorPoint({ 0, 0 });
    bg->setOpacity(90);
    this->addChild(bg);

    auto labelsHolder = CCNode::create();
    labelsHolder->setContentSize(this->getContentSize());

    int index = 0;
    for (const NamedEnumValue<E>& opt : options) {
        auto label = geode::Label::create(opt.name.c_str(), "bigFont.fnt");
        if (label->getContentWidth() > width)
            label->setScale(width / label->getContentWidth());
        label->setPosition(labelsHolder->getContentSize() / 2);
        labelsHolder->addChild(label);

        if (index != 0) {
            label->setVisible(false);
        }

        this->labels.push_back(label);
        this->values.push_back(opt.value);
        index++;
    }

    auto btn = CCMenuItemSpriteExtra::create(
        labelsHolder,
        this,
        menu_selector(OptionSwitcher::onLabelClicked)
    );
    btn->setPosition(this->getContentSize() / 2);
    labelsHolder->setScale(.9f);
    this->addChild(btn);

    return true;
}

template<typename E>
E OptionSwitcher<E>::getSelectedOption() {
    return values[currentOption];
}

template<typename E>
void OptionSwitcher<E>::onLabelClicked(CCObject* sender) {
    auto prevNode = labels[currentOption];

    currentOption += 1;
    if (currentOption >= labels.size()) currentOption = 0;

    prevNode->setVisible(false);

    auto currNode = labels[currentOption];
    currNode->setVisible(true);

    if (callback) callback(values[currentOption]);
}

template<typename E>
void OptionSwitcher<E>::setCallback(Function<void(E)> cb) {
    this->callback = std::move(cb);
}

template<typename E>
void OptionSwitcher<E>::setValue(int index, bool runCallback) {
    auto prevNode = labels[currentOption];

    currentOption = std::clamp(index, 0, static_cast<int>(labels.size() - 1));

    prevNode->setVisible(false);

    auto currNode = labels[currentOption];
    currNode->setVisible(true);

    if (callback && runCallback) callback(values[currentOption]);
}

template<typename E>
void OptionSwitcher<E>::setValue(E e, bool runCallback) {
    for (int i = 0; i < values.size(); i++)
    {
        if (values[i] != e) continue;
        
        setValue(i, runCallback);
        return;
    }
}