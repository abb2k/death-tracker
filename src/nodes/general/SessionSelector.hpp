#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;


class SessionSelector : public CCMenu {
    public:
        static SessionSelector* create(int count);

        void setCallback(Function<void(int)> callback);

        void setMaximumCount(int count, bool runCallback = false);
        void setCurrentCount(int count, bool ignoreIfUnchanged = true, bool runCallback = false);

        int getCurrentCount();
        int getMaximumCount() const {
            return maxCount;
        }

        void setEnabled(bool value);

        geode::TextInput* getTextInput() const{
            return inputNode;
        }

    private:
        bool init(int count);

        void leftArrowClicked(CCObject*);
        void rightArrowClicked(CCObject*);

        void textInputOpened();
        void textInputClosed();
        void textChanged(std::string const& str);

        geode::TextInput* inputNode;
        bool isInputOpened = false;

        Function<void(int)> callback{};

        int maxCount = 0;
        int currentCount = 0;
};