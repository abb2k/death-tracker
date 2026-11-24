#pragma once

using namespace geode::prelude;

class SessionSelector : public CCMenu {
public:
    static SessionSelector* create(int count);

    void setCallback(Function<void(int)> callback);

    void setMaximumCount(int count, bool runCallback = false);
    void setCurrentCount(int count, bool ignoreIfUnchanged = true, bool runCallback = false);

    int getCurrentCount();
    int getMaximumCount() const { return m_maxCount; }

    void setEnabled(bool value);

    geode::TextInput* getTextInput() const { return m_inputNode; }

private:
    bool init(int count);

    void leftArrowClicked(CCObject*);
    void rightArrowClicked(CCObject*);

    void textInputOpened();
    void textInputClosed();
    void textChanged(std::string const& str);

    geode::TextInput* m_inputNode;
    bool m_isInputOpened = false;

    Function<void(int)> m_callback{};

    int m_maxCount = 0;
    int m_currentCount = 0;
};