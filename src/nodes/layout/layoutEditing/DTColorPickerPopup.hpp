#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class FloatingList;
class SimpleToggler;
class RichTextArea;

class DTColorPickerPopup : public geode::ColorPickPopup {
public:
    static DTColorPickerPopup* create(bool isRGBA);
    static DTColorPickerPopup* create(ccColor3B const& color);
    static DTColorPickerPopup* create(ccColor4B const& color);
    static DTColorPickerPopup* create(CustomizationData const& color);

    enum class Pages {
        Color,
        Gradient,
        Wave
    };

    void setGradientChangedCallback(Function<void(GradientData const& data)> callback);
    void setGradientEnabledChangedCallback(Function<void(bool enabled)> callback);
    void setWaveChangedCallback(Function<void(WaveData const& data)> callback);
    void setWaveEnabledChangedCallback(Function<void(bool enabled)> callback);

private:
    class GradientColorPickerDelegate : public ColorPickerDelegate {
    public:
        DTColorPickerPopup* m_popup = nullptr;
        bool m_updatesColorA = false;

        void colorValueChanged(ccColor3B color) override;
    };

    bool init(CustomizationData const& data, bool isRGBA);

    static DTColorPickerPopup* create(CustomizationData const& data, bool isRGBA);

    cocos2d::CCNode* m_normalContent;

    void onSwitcherBtnClick(cocos2d::CCObject* sender);
    
    FloatingList* m_floatingList;

    int m_currentPage;

    void createGradientPage(bool isRGBA, GradientData const& data);
    void createWavePage(WaveData const& data);
    void updateWaveData();
    void updateWavePreview();
    void waveInputChanged(int tag, std::string const& value);
    void waveSliderChanged(CCObject* sender);
    void setWaveValue(int tag, float value, bool updateInput);
    void onWavePageOpen(CCNode* page);
    void onGradientResetA(CCObject*);
    void onGradientResetB(CCObject*);
    void opacitySliderAChanged(CCObject*);
    void opacitySliderBChanged(CCObject*);
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
    static constexpr CCPoint m_DURATION_LIMITS = { 0, 10 };
    float m_stagger = 0.2f;
    static constexpr CCPoint m_STAGGER_LIMITS = { -2, 2 };

    CCSprite* m_gradientNewColorASpr = nullptr;
    CCSprite* m_gradientNewColorBSpr = nullptr;
    CCMenuItemSpriteExtra* m_gradientResetBtnA = nullptr;
    CCMenuItemSpriteExtra* m_gradientResetBtnB = nullptr;
    CCMenuItemSpriteExtra* m_resetBtn = nullptr;

    static constexpr int m_gradientSquareCount = 16;
    std::vector<CCSprite*> m_gradientOldSquares;
    std::vector<CCSprite*> m_gradientNewSquares;

    void setGradient(std::vector<CCSprite*> const& squares, ccColor4B const& a, ccColor4B const& b);
    void updateGradientColorA(ccColor3B color, bool doUpdateHexInput = true);
    void updateGradientColorB(ccColor3B color, bool doUpdateHexInput = true);

    void setAllChildrenEnabled(CCNode* node, bool enabled);

    void colorValueChanged(cocos2d::ccColor3B color) override;

    CCNode* makeRGBA(ccColor4B& color, bool isRGBA, int tag);
    std::map<int, std::tuple<TextInput*, ccColor4B*, GLubyte*>> m_gradientInputs;
    std::map<int, Slider*> m_gradientOpacitySliders;

    CCNode* m_gradientPage = nullptr;
    CCMenu* m_gradientEnabledMenu = nullptr;
    SimpleToggler* m_gradientEnabledToggle = nullptr;
    bool m_gradientEnabled = false;

    CCNode* m_wavePage = nullptr;
    CCMenu* m_waveEnabledMenu = nullptr;
    SimpleToggler* m_waveEnabledToggle = nullptr;
    bool m_waveEnabled = false;
    WaveData m_waveData;
    RichTextArea* m_wavePreview = nullptr;
    std::map<int, TextInput*> m_waveInputs;
    std::map<int, Slider*> m_waveSliders;

    Function<void(WaveData const& data)> m_waveChangedCallback;
    Function<void(bool enabled)> m_waveEnabledChangedCallback;

    void textChanged(TextInput* input);
    void updateGradientEnabledPosition();
    void updateWaveEnabledPosition();

    static constexpr int m_TAG_GRADIENT_A_COLOR_R_INPUT = 10;
    static constexpr int m_TAG_GRADIENT_A_COLOR_G_INPUT = m_TAG_GRADIENT_A_COLOR_R_INPUT + 1;
    static constexpr int m_TAG_GRADIENT_A_COLOR_B_INPUT = m_TAG_GRADIENT_A_COLOR_R_INPUT + 2;
    static constexpr int m_TAG_GRADIENT_A_COLOR_HEX_INPUT = m_TAG_GRADIENT_A_COLOR_R_INPUT + 3;
    static constexpr int m_TAG_GRADIENT_A_COLOR_A_INPUT = m_TAG_GRADIENT_A_COLOR_R_INPUT + 4;

    static constexpr int m_TAG_GRADIENT_B_COLOR_R_INPUT = 20;
    static constexpr int m_TAG_GRADIENT_B_COLOR_G_INPUT = m_TAG_GRADIENT_B_COLOR_R_INPUT + 1;
    static constexpr int m_TAG_GRADIENT_B_COLOR_B_INPUT = m_TAG_GRADIENT_B_COLOR_R_INPUT + 2;
    static constexpr int m_TAG_GRADIENT_B_COLOR_HEX_INPUT = m_TAG_GRADIENT_B_COLOR_R_INPUT + 3;
    static constexpr int m_TAG_GRADIENT_B_COLOR_A_INPUT = m_TAG_GRADIENT_B_COLOR_R_INPUT + 4;

    Function<void(GradientData const& data)> m_gradientChangedCallback;
    Function<void(bool enabled)> m_gradientEnabledChangedCallback;

    void onGradientPageOpen(CCNode* page);

    NineSlice* m_gradientDisabledFG = nullptr;
    NineSlice* m_waveDisabledFG = nullptr;

    RichTextArea* m_donorLabel;
    RichTextArea* m_waveDonorLabel = nullptr;

    static constexpr CCPoint m_WAVE_SPEED_LIMITS = { .05f, 5.f };
    static constexpr CCPoint m_WAVE_DISTANCE_LIMITS = { -100.f, 100.f };
    static constexpr CCPoint m_WAVE_OFFSET_LIMITS = { -2.f, 2.f };
    static constexpr int m_TAG_WAVE_SPEED = 1;
    static constexpr int m_TAG_WAVE_DISTANCE_Y = 2;
    static constexpr int m_TAG_WAVE_DISTANCE_X = 3;
    static constexpr int m_TAG_WAVE_OFFSET = 4;
};