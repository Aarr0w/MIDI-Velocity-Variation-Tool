/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class AarrowLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AarrowLookAndFeel()
    {
        setColour(juce::Label::textColourId, juce::Colours::cyan);
        //setColour(juce::Slider::thumbColourId, juce::Colours::antiquewhite);
        setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        setColour(juce::Slider::trackColourId, (juce::Colours::cyan).withBrightness(0.8)); // You should really change the saturation on this..        setColour(juce::Label::textColourId, juce::Colours::cyan);

        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);

        setColour(juce::TextButton::textColourOnId, juce::Colours::cyan);
        setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
        //setColour(juce::TextButton::textColourOffId, juce::Colours::darkgrey.withAlpha(0.5f));
        //setColour(juce::TextButton::buttonColourId, juce::Colours::transparentWhite);


        setColour(juce::ToggleButton::tickColourId, juce::Colours::orange);

        setDefaultSansSerifTypefaceName("Avenir Next");
        

    }



    void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override
    {
        auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
        auto fill = slider.findColour(Slider::rotarySliderFillColourId);

        auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);

        auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = jmin(8.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;

        Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            rotaryStartAngle,
            rotaryEndAngle,
            true);

        g.setColour(outline);
        g.strokePath(backgroundArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));

        if (slider.isEnabled())
        {
            Path valueArc;
            valueArc.addCentredArc(bounds.getCentreX(),
                bounds.getCentreY(),
                arcRadius,
                arcRadius,
                0.0f,
                rotaryStartAngle,
                toAngle,
                true);

            g.setColour(fill);
            g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
        }

        auto thumbWidth = lineW * 2.0f;
        Point<float> thumbPoint(bounds.getCentreX() + arcRadius * std::cos(toAngle - MathConstants<float>::halfPi),
            bounds.getCentreY() + arcRadius * std::sin(toAngle - MathConstants<float>::halfPi));

        g.setColour(slider.findColour(Slider::thumbColourId));
        g.fillEllipse(Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));
    }


    void drawLinearSlider(Graphics& g, int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const Slider::SliderStyle style, Slider& slider)override
    {
        if (slider.isBar())
        {
            // creates shadow
            auto shadowArea = slider.getLocalBounds();
            auto edge = 2;

            //sliderArea.removeFromLeft(edge);
            //sliderArea.removeFromTop(edge);
            shadowArea.translate(0, edge);

            // shadow
            g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
            g.fillRect(shadowArea.withTrimmedRight(edge*4));


            // acutal bar with gradient
            g.setColour(slider.findColour(Slider::trackColourId));

            x += edge;
            y -= edge;

            auto gradient = new ColourGradient(juce::Colours::white, static_cast<float> (x), (float)y + 0.5f, slider.findColour(Slider::trackColourId), sliderPos - (float)x, (float)height - 1.0f, true);
            //gradient->addColour(0.5f, juce::Colours::grey);
                    
            g.setFillType(*(new FillType(*gradient)));

            g.fillRect(slider.isHorizontal() ? Rectangle<float>(static_cast<float> (x), (float)y + 0.5f, juce::jmax(5.0f-(float)x ,sliderPos - (float)x), (float)height - 1.0f)
                : Rectangle<float>((float)x + 0.5f, (float)y-sliderPos, (float)width - 1.0f, (float)y + ((float)height - sliderPos)));

           
        }
        else
        {
            auto trackWidth = jmin(6.0f, slider.isHorizontal() ? (float)height * 0.25f : (float)width * 0.25f);

           //outline
           // g.setColour(juce::Colours::grey);
            auto area = new Rectangle<float>((float)x, (float)y, width, height);
           //g.fillRect(*area);

            g.setColour(findColour(PropertyComponent::backgroundColourId));
            g.fillRect(area->reduced(1));
            

            *area = area->reduced(6);
            x = area->getTopLeft().getX();
            width = area->getWidth();
            y = area->getTopLeft().getY();  // commenting this line makes a battery shape* 
            height = area->getHeight();

            sliderPos += 7; //sliderPos would reach the top of the background rectangle otherwise
                            //slider.proportionOfLengthToValue
            auto gradient = new ColourGradient(slider.findColour(Slider::thumbColourId) , static_cast<float> (x), (float)y + 0.5f, slider.findColour(Slider::trackColourId), static_cast<float> (x), (float)height - 1.0f, false);
            gradient->addColour(0.5f, juce::Colours::grey);
            g.setFillType(*(new FillType(*gradient)));

            //Rectangel.withSizeKeepingCentre() might also be useful for the gradient rectangle

            g.fillRect(slider.isHorizontal() ? Rectangle<float>(static_cast<float> (x), (float)y + 0.5f, juce::jmax(5.0f - (float)x, sliderPos - (float)x), (float)height - 1.0f)
               :  Rectangle<float>((float)x + 0.5f,sliderPos, (float)width - 1.0f, (float)y + ((float)height - sliderPos)));
            g.setColour(slider.findColour(Slider::trackColourId));
     

            g.setColour(findColour(PropertyComponent::backgroundColourId));
            y -= 5;
             for (float n = y; n < y + height; n += height/5) // might want to manually change this '5' for different value sliders, 
            {                                                 // current class doesnt have acces to the actual value of the parameter 
                g.fillRect((float)x , n, (float)width, 6.0f);
            }
           
             //need to make this if/else and add horizontal version

        }
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool, bool isButtonDown) override
    {
        auto buttonArea = button.getLocalBounds();
        auto edge = 4;

        buttonArea.removeFromLeft(edge);
        buttonArea.removeFromRight(edge);
        buttonArea.removeFromTop(edge);

        // shadow
        g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
        g.fillRect(buttonArea);

        auto offset = isButtonDown ? -edge / 2 : -edge;
        buttonArea.translate(-offset, offset);

        g.setColour(backgroundColour);
        g.fillRect(buttonArea);
    }



private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AarrowLookAndFeel)
};

// ================================================================================================================================
class AarrowAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AarrowAudioProcessorEditor(NewProjectAudioProcessor&);
    ~AarrowAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    // This constructor has been changed to take a reference instead of a pointer
    //JUCE_DEPRECATED_WITH_BODY(AarrowAudioProcessorEditor(juce::AudioProcessor* p), : AarrowAudioProcessorEditor(*p) {})
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
    AarrowLookAndFeel Aalf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AarrowAudioProcessorEditor)
};
