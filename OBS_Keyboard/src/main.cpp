#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include <touch_button.h>
#include <BleKeyboard.h>

// my own file containing wlan secrets and twitch oauth
#include "../../security.h"

char ssid[] = WLAN_SSID;
char password[] = WLAN_PASS;

/*__Pin definitions for the Arduino MKR__*/
#define TFT_CS   5
#define TFT_DC   4
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_RST  22
#define TFT_MISO 19
#define TFT_LED  15  


#define HAVE_TOUCHPAD
#define TOUCH_CS 14
#define TOUCH_IRQ 2 
/*_______End of definitions______*/

/*____Calibrate Touchscreen_____*/
#define MINPRESSURE 10      // minimum required force for touch event
#define TS_MINX 420
#define TS_MINY 260
#define TS_MAXX 3850
#define TS_MAXY 3850
/*______End of Calibration______*/

#define HOSTNAME "OBSkeyboard"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
BleKeyboard bleKeyboard("OBS Keyboard", "Sven Kicherer", 100);

#define NUMBER_OF_BUTTONS 2
TouchButton *buttons[NUMBER_OF_BUTTONS];

uint8_t radius = 5;

void ApplyLookAndFeel(TouchButton *btn)
{
    btn->SetOutlineStrength(3);
    btn->SetColorFill(ILI9341_RED);
    btn->SetRadius(radius);
}

void setup() {
    pinMode(TFT_LED, OUTPUT); // define as output for backlight control
    tft.begin();          
    tft.setRotation(3);   // landscape mode  
    ts.begin();
    ts.setRotation(tft.getRotation());
    tft.fillScreen(ILI9341_BLACK);// clear screen 
    tft.setTextSize(1);
    tft.setCursor(0, 0);



    tft.println("Starting BLE work!");
    bleKeyboard.begin();

}

bool BtConnectedFirstTime = false;

void SetSceneF13(String param)
{  
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press(KEY_LEFT_SHIFT);
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press(KEY_F10);
    delay(200);
    bleKeyboard.releaseAll();
}

void SetSceneF14(String param)
{
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press(KEY_LEFT_SHIFT);
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press(KEY_F11);
    delay(200);
    bleKeyboard.releaseAll();
}

void loop() {
    if (bleKeyboard.isConnected())
    {
        if (!BtConnectedFirstTime)
        {
            BtConnectedFirstTime = true;
            tft.fillScreen(ILI9341_BLACK);// clear screen 
            buttons[0] = new TouchButton("Szene 1", TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
            ApplyLookAndFeel(buttons[0]);
            buttons[0]->registerCallback(SetSceneF13);
            buttons[0]->draw(&tft, 10, 10, 300, 105);
            buttons[1] = new TouchButton("Szene 2", TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
            ApplyLookAndFeel(buttons[1]);
            buttons[1]->registerCallback(SetSceneF14);
            buttons[1]->draw(&tft, 10, 125, 300, 105);
        }
    }
    else{
        BtConnectedFirstTime = false;
    }
    if (BtConnectedFirstTime)
    {
        int16_t x, y;
        if (ts.touched())
        {
            TS_Point point = ts.getPoint();

            x = point.x;
            y = point.y;
        }
        else
        {
            x = 0;
            y = 0;
        }
        for (int i = 0; i < NUMBER_OF_BUTTONS; i++)
        {
            buttons[i]->isPressed(x, y);
        }
    }
    delay(50);
}