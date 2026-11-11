#pragma once

#include <Geode/Geode.hpp>
#include <nodes/optionNodes/OptionsNode.hpp>

using namespace geode::prelude;


class SessionOptions : public OptionsNode {
    public:
        static SessionOptions* create(const CCSize& size);
    private:
        virtual bool setup();
    public:
        virtual void onOpened();
        virtual void onClosed();
};