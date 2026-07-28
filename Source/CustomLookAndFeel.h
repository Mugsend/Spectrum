#pragma once
#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        setDefaultSansSerifTypefaceName("Helvetica");
    }

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override
    {
        return juce::LookAndFeel_V4::getTypefaceForFont(font);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        bool isOn = button.getToggleState();

        juce::Colour bgBase = juce::Colour(0xff333333);
        juce::Colour bgHover = juce::Colour(0xff444444);
        juce::Colour bgActive = juce::Colour(0xffffac00);

        juce::Colour currentBg = isOn ? bgActive : (shouldDrawButtonAsHighlighted ? bgHover : bgBase);


        g.setColour(currentBg);
        g.fillRect(bounds);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(bounds, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::Font font(12.0f);
        g.setFont(font);

        g.setColour(button.getToggleState() ? juce::Colour(0xff111111) : juce::Colours::lightgrey);
        g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
        int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float>(0, 0, width, height);

        g.setColour(juce::Colour(0xff333333));
        g.fillRect(bounds);
        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(bounds, 1.0f);

        juce::Path triangle;
        float centerX = buttonX + buttonW * 0.5f;
        float centerY = buttonY + buttonH * 0.5f;
        triangle.addTriangle(centerX - 4.0f, centerY - 2.0f,
            centerX + 4.0f, centerY - 2.0f,
            centerX, centerY + 3.0f);
        g.setColour(juce::Colours::grey);
        g.fillPath(triangle);
    }


    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRect(0, 0, width, height);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(0, 0, width, height, 1);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
        bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
        const juce::String& text, const juce::String& shortcutKeyText,
        const juce::Drawable* icon, const juce::Colour* textColour) override
    {
        if (isSeparator)
        {
            auto r = area.reduced(5, 0);
            r.removeFromTop(juce::roundToInt((r.getHeight() * 0.5f) - 0.5f));
            g.setColour(juce::Colour(0xff1a1a1a));
            g.fillRect(r.withHeight(1));
            return;
        }

        if (isHighlighted && isActive)
        {
            g.setColour(juce::Colour(0xffffac00));
            g.fillRect(area);
        }

        g.setColour(isHighlighted && isActive ? juce::Colour(0xff111111) : juce::Colours::lightgrey);
        g.setFont(juce::Font(12.0f, juce::Font::bold));

        auto r = area.reduced(10, 0);

        if (isTicked)
        {
            auto tickArea = r.removeFromLeft(15).withSizeKeepingCentre(6, 6).toFloat();

            g.setColour(isHighlighted && isActive ? juce::Colour(0xff111111) : juce::Colour(0xffffac00));
            g.fillRect(tickArea);
        }
        else
        {
            r.removeFromLeft(15);
        }

        g.drawText(text, r, juce::Justification::centredLeft, true);
    }

    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
        int standardMenuItemHeight, int& idealWidth, int& idealHeight) override
    {
        juce::LookAndFeel_V4::getIdealPopupMenuItemSize(text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight);

        idealHeight = isSeparator ? 8 : 24;

        idealWidth += 20;
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackRect = juce::Rectangle<float>(static_cast<float>(x), y + height * 0.5f - 2.0f, static_cast<float>(width), 4.0f);
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRect(trackRect);


        auto thumbWidth = 8.0f;
        auto thumbRect = juce::Rectangle<float>(sliderPos - thumbWidth * 0.5f, static_cast<float>(y + 2), thumbWidth, static_cast<float>(height - 4));

        g.setColour(slider.isMouseOverOrDragging() ? juce::Colour(0xffcccccc) : juce::Colour(0xff999999));
        g.fillRect(thumbRect);
    }
};