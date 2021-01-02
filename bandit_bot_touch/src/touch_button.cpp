#include "touch_button.h"


TouchButton::TouchButton(String string) :
    my_string(string)
{}

TouchButton::TouchButton(String string, int16_t x_min_touch, int16_t x_max_touch, int16_t y_min_touch, int16_t y_max_touch) :
    my_string(string),
    touch_x_min(x_min_touch),
    touch_x_max(x_max_touch),
    touch_y_min(y_min_touch),
    touch_y_max(y_max_touch)
{
    //tft.drawCircle(60, 60, 60, ILI9341_BLUE);
}


void TouchButton::SetColors(uint16_t outline, uint16_t fill, uint16_t text)
{
    color_outline = outline;
    color_fill = fill;
    color_text = text;
}

void TouchButton::SetColorsPressed(uint16_t outline, uint16_t fill, uint16_t text)
{
    color_outline_pressed = outline;
    color_fill_pressed = fill;
    color_text_pressed = text;
}

void TouchButton::SetColorFill(uint16_t fill)
{
    color_fill = fill;
}

void TouchButton::SetColorOutline(uint16_t outline)
{
    color_outline = outline;
}

void TouchButton::SetColorText(uint16_t text)
{
    color_text = text;
}

void TouchButton::SetColorFillPressed(uint16_t fill)
{
    color_fill_pressed = fill;
}

void TouchButton::SetColorOutlinePressed(uint16_t outline)
{
    color_outline_pressed = outline;
}

void TouchButton::SetColorTextPressed(uint16_t text)
{
    color_text_pressed = text;
}

void TouchButton::SetOutlineStrength(uint8_t strength)
{
    outline_thickness = strength;
}

void TouchButton::registerCallback(callbackFunc func)
{
    callback = func;
}

bool TouchButton::isPressed(int16_t x, int16_t y)
{
    bool retval = false;
    unsigned long act_millis = millis();
    if ((x >= touch_area_x_min) && (x <= touch_area_x_max) && (y >= touch_area_y_min) && (y <= touch_area_y_max))
    {
        retval = true;
        if (callback && ((last_pressed_millis + press_duration) < act_millis))
        {
            callback(my_string);
        }
        last_pressed_millis = act_millis;
    }

    if (retval && state != ButtonState::pressed)
    {
        state = ButtonState::pressed;
        drawButton(color_outline_pressed, color_fill_pressed, color_text_pressed);
    }
    else if (!retval && state != ButtonState::released && ((last_pressed_millis + press_duration) < act_millis))
    {
        state = ButtonState::released;
        drawButton(color_outline, color_fill, color_text);
    }
    return retval;
}

void TouchButton::draw(Adafruit_GFX* tft, int16_t x, int16_t y, int16_t width, int16_t height)
{
    m_x = x;
    m_y = y;
    m_h = height;
    m_w = width;
    state = ButtonState::released;
    //tft.drawRoundRect

    touch_area_x_min = x;
    touch_area_y_min = y;
    touch_area_x_max = x + width;
    touch_area_y_max = y + height;

    if (touch_x_min != -1)
    {
        touch_area_x_min = map(x, 0, tft->width(), touch_x_min, touch_x_max);
        touch_area_x_max = map(x + width, 0, tft->width(), touch_x_min, touch_x_max);
        touch_area_y_min = map(y, 0, tft->height(), touch_y_min, touch_y_max);
        touch_area_y_max = map(y + height, 0, tft->height(), touch_y_min, touch_y_max);
    }
    
    if (my_tft == nullptr)
    {
        my_tft = tft;
    }

    drawButton(color_outline, color_fill, color_text);
}

void TouchButton::drawButton(uint16_t ColorOutline, uint16_t ColorFill, uint16_t ColorText)
{
    // fill zeichnen
    my_tft->fillRoundRect(m_x, m_y, m_w, m_h, radius, ColorFill);
    
    // outline zeichnen
    for (int i = 0; i < outline_thickness; i++) {
        my_tft->drawRoundRect(m_x + i, m_y + i, m_w - i*2, m_h - i*2, radius, ColorOutline);
    }

    // text schreiben
    drawCentreString(my_string.c_str(), ColorText);
}

String TouchButton::UpdateText(String text)
{
    my_string = text;
    drawButton(color_outline, color_fill, color_text);
    return my_string;
}

String TouchButton::GetText()
{
    return my_string;
}

void TouchButton::SetRadius(uint8_t rad)
{
    radius = rad;
}

void TouchButton::drawCentreString(const char *buf, uint16_t color)
{
    int16_t x1, y1;
    uint16_t w, h;
    my_tft->setTextSize(textsize);
    my_tft->getTextBounds(buf, m_x + (m_w / 2), m_y + (m_h / 2), &x1, &y1, &w, &h); //calc width of new string
    my_tft->setCursor(m_x + (m_w / 2) - (w / 2), m_y + (m_h / 2) - (h / 2));
    my_tft->setTextColor(color);
    my_tft->print(buf);
}

// String TouchButton::getTouchArea()
// {
//     String retval = "";

//     retval += "Toucharea x: ";
//     retval += String(touch_area_x_min);
//     retval += " - ";
//     retval += String(touch_area_x_max);
//     retval += "\n";
//     retval += "Toucharea y: ";
//     retval += String(touch_area_y_min);
//     retval += " - ";
//     retval += String(touch_area_y_max);
//     retval += "\n";

//     return retval;
// }
