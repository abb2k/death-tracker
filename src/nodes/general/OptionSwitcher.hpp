#pragma once

using namespace geode::prelude;

template<typename E>
struct NamedEnumValue {
public:
    E m_value;
    std::string m_name;

    NamedEnumValue(E value, std::string name)
        : m_value(value), m_name(std::move(name)) {}
};

template<typename E>
class OptionSwitcher : public CCMenu {
public:
    static OptionSwitcher<E>* create(float width, std::vector<NamedEnumValue<E>> options);

    E getSelectedOption();

    void setCallback(Function<void(E)> callback);

    void setValue(int index, bool runCallback = false);
    void setValue(E e, bool runCallback = false);

private:
    bool init(float width, std::vector<NamedEnumValue<E>> options);

    std::vector<geode::Label*> m_labels{};
    std::vector<E> m_values{};

    void onLabelClicked(CCObject* sender);

    int m_currentOption = 0;

    Function<void(E)> m_callback = nullptr;
};


template<typename E>
OptionSwitcher<E>* OptionSwitcher<E>::create(float width, std::vector<NamedEnumValue<E>> options) {
    auto ret = new OptionSwitcher<E>();

    if (ret && ret->init(std::move(width), std::move(options))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

template<typename E>
bool OptionSwitcher<E>::init(float width, std::vector<NamedEnumValue<E>> options) {
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
        auto label = geode::Label::create(opt.m_name.c_str(), "bigFont.fnt");
        if (label->getContentWidth() > width)
            label->setScale(width / label->getContentWidth());
        label->setPosition(labelsHolder->getContentSize() / 2);
        labelsHolder->addChild(label);

        if (index != 0) {
            label->setVisible(false);
        }

        this->m_labels.push_back(label);
        this->m_values.push_back(opt.m_value);
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
    return m_values[m_currentOption];
}

template<typename E>
void OptionSwitcher<E>::onLabelClicked(CCObject* sender) {
    auto prevNode = m_labels[m_currentOption];

    m_currentOption += 1;
    if (m_currentOption >= m_labels.size()) m_currentOption = 0;

    prevNode->setVisible(false);

    auto currNode = m_labels[m_currentOption];
    currNode->setVisible(true);

    if (m_callback) m_callback(m_values[m_currentOption]);
}

template<typename E>
void OptionSwitcher<E>::setCallback(Function<void(E)> cb) {
    this->m_callback = std::move(cb);
}

template<typename E>
void OptionSwitcher<E>::setValue(int index, bool runCallback) {
    auto prevNode = m_labels[m_currentOption];

    m_currentOption = std::clamp(index, 0, static_cast<int>(m_labels.size() - 1));

    prevNode->setVisible(false);

    auto currNode = m_labels[m_currentOption];
    currNode->setVisible(true);

    if (m_callback && runCallback) m_callback(m_values[m_currentOption]);
}

template<typename E>
void OptionSwitcher<E>::setValue(E e, bool runCallback) {
    for (int i = 0; i < m_values.size(); i++)
    {
        if (m_values[i] != e) continue;
        
        setValue(i, runCallback);
        return;
    }
}