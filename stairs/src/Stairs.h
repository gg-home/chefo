#include <Arduino.h>
#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 1000
#define DATA_PIN 2

// Define the array of leds
CRGB leds[NUM_LEDS];

class StairsClass{

bool calibrating = false;
uint32_t color = 16711680;
uint8_t brightness = 20;

public:
    void calibrate(uint32_t newColor, uint8_t newBrightness){
        calibrating = true;
        color = newColor;
        brightness = newBrightness;

        litAll(color, brightness);
    }

    void initialize()
    {
        FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
        FastLED.setBrightness(brightness);
        leds[0] = color;
        FastLED.show();
    }

    void live()
    {
        if(waitForCalibrationToFinish(10000) == false)
        {
            litAll(color, brightness);
            delay(500);
            offAll();
            delay(500);
        }
    }

private:
    void litAll(uint32_t p_color, uint8_t p_brightness)
    {
        FastLED.setBrightness(p_brightness);

        for(int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = p_color;
        }

        FastLED.show();
    }

    void offAll()
    {
        for(int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CRGB::Black;
        }

        FastLED.show();
    }

    bool waitForCalibrationToFinish(uint32_t wait)
    {
        if(calibrating)
        {
            calibrating = false;
            delay(wait);
            
            if(calibrating == false)
            {
                offAll();
            }
        }

        return calibrating;
    }
};

StairsClass Stairs;