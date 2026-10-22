#include <nodes/optionNodes/SessionOptions.hpp>

SessionOptions* SessionOptions::create(const CCSize& size) {
    auto ret = new SessionOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SessionOptions::setup() {

    


    return true;
}

void SessionOptions::onOpened() {
    
}
void SessionOptions::onClosed() {
    
}