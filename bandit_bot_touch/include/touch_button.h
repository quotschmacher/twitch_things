#ifndef _TOUCH_BUTTON_H_
#define _TOUCH_BUTTON_H_
#include <Arduino.h>
#include "Adafruit_GFX.h"


class TouchButton
{
private:
    enum class ButtonState {undefined, pressed, released};

    String my_string;
    ButtonState state = ButtonState::undefined;
    int16_t touch_x_min = -1;
    int16_t touch_x_max = -1;
    int16_t touch_y_min = -1;
    int16_t touch_y_max = -1;
    uint16_t color_outline = 0xFFFF;
    uint16_t color_fill = 0;
    uint16_t color_text = 0xFFFF;
    uint16_t color_outline_pressed = 0xFFFF;
    uint16_t color_fill_pressed = 0xFFFF;
    uint16_t color_text_pressed = 0;
    uint8_t radius = 3;
    int16_t m_x = -1;
    int16_t m_y = -1;
    int16_t m_w = -1;
    int16_t m_h = -1;
    int16_t touch_area_x_min = -1;
    int16_t touch_area_x_max = -1;
    int16_t touch_area_y_min = -1;
    int16_t touch_area_y_max = -1;
    uint8_t outline_thickness = 1;
    uint8_t textsize = 2;
    unsigned long last_pressed_millis = 0;
    unsigned long press_duration = 750;

    Adafruit_GFX* my_tft = nullptr;

    void drawCentreString(const char *buf, uint16_t color);
    void drawButton(uint16_t ColorOutline, uint16_t ColorFill, uint16_t ColorText);
    using callbackFunc = void (*)(String);
    callbackFunc callback = nullptr;
public:
    TouchButton(String string);
    TouchButton(String string, int16_t x_min_touch, int16_t x_max_touch, int16_t y_min_touch, int16_t y_max_touch);
    void SetColors(uint16_t outline, uint16_t fill, uint16_t text);
    void SetColorFill(uint16_t fill);
    void SetColorOutline(uint16_t outline);
    void SetColorText(uint16_t text);
    void SetColorFillPressed(uint16_t fill);
    void SetColorOutlinePressed(uint16_t outline);
    void SetColorTextPressed(uint16_t text);
    void SetColorsPressed(uint16_t outline, uint16_t fill, uint16_t text);
    void SetOutlineStrength(uint8_t strength);
    void draw(Adafruit_GFX* tft, int16_t x, int16_t y, int16_t width, int16_t height);
    bool isPressed(int16_t x, int16_t y);
    //String getTouchArea();
    void registerCallback(callbackFunc func);
    String UpdateText(String text);
    String GetText();
    void SetRadius(uint8_t rad);
};

#endif // _TOUCH_BUTTON_H_