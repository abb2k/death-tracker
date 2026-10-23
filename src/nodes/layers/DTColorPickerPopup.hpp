#pragma once

#include <Geode/Geode.hpp>

#include <nodes/FloatingList.hpp>
#include <nodes/SimpleToggler.hpp>

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class DTColorPickerPopup : public geode::ColorPickPopup {
    public:
        static DTColorPickerPopup* create(bool isRGBA);
        static DTColorPickerPopup* create(cocos2d::ccColor3B const& color);
        static DTColorPickerPopup* create(cocos2d::ccColor4B const& color);
        static DTColorPickerPopup* create(CustomizationData const& color);

        enum class Pages {
            Color,
            Gradient,
            Wave
        };

        void setGradientChangedCallback(geode::Function<void(GradientData const& data)> callback);
        void setGradientEnabledChangedCallback(geode::Function<void(bool enabled)> callback);

    protected:
        class GradientColorPickerDelegate : public ColorPickerDelegate {
        public:
            DTColorPickerPopup* popup = nullptr;
            bool updatesColorA = false;

            void colorValueChanged(cocos2d::ccColor3B color) override;
        };

        bool init(CustomizationData const& data, bool isRGBA);

        static DTColorPickerPopup* create(CustomizationData const& data, bool isRGBA);

        cocos2d::CCNode* m_normalContent;

        void onSwitcherBtnClick(cocos2d::CCObject* sender);
        
        FloatingList* m_floatingList;

        int m_currentPage;

        void createGradientPage(bool isRGBA, GradientData const& data);
        void createWavePage();
        void onGradientResetA(cocos2d::CCObject*);
        void onGradientResetB(cocos2d::CCObject*);
        void opacitySliderAChanged(cocos2d::CCObject*);
        void opacitySliderBChanged(cocos2d::CCObject*);
        void updateGradientOpacity(int tag);

        CCControlColourPicker* m_gradientPickerA = nullptr;
        CCControlColourPicker* m_gradientPickerB = nullptr;
        GradientColorPickerDelegate m_gradientPickerADelegate;
        GradientColorPickerDelegate m_gradientPickerBDelegate;

        ccColor4B m_gradientColorA = { 92, 252, 255, 255 };
        ccColor4B m_gradientColorB = { 224, 248, 67, 255 };
        ccColor4B m_originalGradientColorA = { 92, 252, 255, 255 };
        ccColor4B m_originalGradientColorB = { 224, 248, 67, 255 };

        float m_duration = 1.f;
        const CCPoint DURATION_LIMITS = { 0, 10 };
        float m_stagger = 0.2f;
        const CCPoint STAGGER_LIMITS = { -2, 2 };

        CCSprite* m_gradientNewColorASpr = nullptr;
        CCSprite* m_gradientNewColorBSpr = nullptr;
        CCMenuItemSpriteExtra* m_gradientResetBtnA = nullptr;
        CCMenuItemSpriteExtra* m_gradientResetBtnB = nullptr;
        CCMenuItemSpriteExtra* m_resetBtn = nullptr;

        int m_gradientSquareCount = 16;
        std::vector<CCSprite*> m_gradientOldSquares;
        std::vector<CCSprite*> m_gradientNewSquares;

        void setGradient(std::vector<CCSprite*> const& squares, ccColor4B const& a, ccColor4B const& b);
        void updateGradientColorA(cocos2d::ccColor3B color, bool doUpdateHexInput = true);
        void updateGradientColorB(cocos2d::ccColor3B color, bool doUpdateHexInput = true);

        void setAllChildrenEnabled(CCNode* node, bool enabled);

        void colorValueChanged(cocos2d::ccColor3B color) override;

        CCNode* makeRGBA(ccColor4B& color, bool isRGBA, int tag);
        std::map<int, std::tuple<TextInput*, ccColor4B*, GLubyte*>> m_gradientInputs;
        std::map<int, Slider*> m_gradientOpacitySliders;

        CCNode* m_gradientPage = nullptr;
        CCMenu* m_gradientEnabledMenu = nullptr;
        SimpleToggler* m_gradientEnabledToggle = nullptr;
        bool m_gradientEnabled = false;

        void textChanged(CCTextInputNode* input) override;
        void updateGradientEnabledPosition();

        const int TAG_GRADIENT_A_COLOR_R_INPUT = 10;
        const int TAG_GRADIENT_A_COLOR_G_INPUT = TAG_GRADIENT_A_COLOR_R_INPUT + 1;
        const int TAG_GRADIENT_A_COLOR_B_INPUT = TAG_GRADIENT_A_COLOR_R_INPUT + 2;
        const int TAG_GRADIENT_A_COLOR_HEX_INPUT = TAG_GRADIENT_A_COLOR_R_INPUT + 3;
        const int TAG_GRADIENT_A_COLOR_A_INPUT = TAG_GRADIENT_A_COLOR_R_INPUT + 4;

        const int TAG_GRADIENT_B_COLOR_R_INPUT = 20;
        const int TAG_GRADIENT_B_COLOR_G_INPUT = TAG_GRADIENT_B_COLOR_R_INPUT + 1;
        const int TAG_GRADIENT_B_COLOR_B_INPUT = TAG_GRADIENT_B_COLOR_R_INPUT + 2;
        const int TAG_GRADIENT_B_COLOR_HEX_INPUT = TAG_GRADIENT_B_COLOR_R_INPUT + 3;
        const int TAG_GRADIENT_B_COLOR_A_INPUT = TAG_GRADIENT_B_COLOR_R_INPUT + 4;

        geode::Function<void(GradientData const& data)> m_gradientChangedCallback;
        geode::Function<void(bool enabled)> m_gradientEnabledChangedCallback;

        void onGradientPageOpen(CCNode* page);

        NineSlice* m_gradientDisabledFG = nullptr;
};