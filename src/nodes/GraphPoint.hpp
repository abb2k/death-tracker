#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class GraphPoint;
class DTGraphNode;

class GraphPointDelegate {
    public:
        virtual void OnPointSelected(GraphPoint* point) {};

        virtual void OnPointDeselected(GraphPoint* point) {};
};

class GraphPoint : public CCMenuItem {
    protected:
        bool init(const std::string& run, const float& rate, const ccColor4B& color);
        
    public:
        static GraphPoint* create(const std::string& run, const float& rate, const ccColor4B& color);

        //set the delegate for the point
        void setDelegate(GraphPointDelegate* Delegate);

        std::string m_Run;

        float m_Rate;

        DTGraphNode* relatedGraph;

        ~GraphPoint();

    private:
        //check for inputs
        void update(float delta) override;

        CCSprite* c;

        GraphPointDelegate* m_Delegate = nullptr;
        bool m_oneTimeCall;
};