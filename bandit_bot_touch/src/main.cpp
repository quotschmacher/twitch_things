#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include <touch_button.h>
#include <WiFi.h>
#include <IRCClient.h>
#include <ArduinoOTA.h>

// my own file containing wlan secrets and twitch oauth
#include "../../security.h"

#define IRC_SERVER   "irc.chat.twitch.tv"
#define IRC_PORT     6667

#ifndef WLAN_SSID
#define WLAN_SSID "ssid"
#endif

#ifndef WLAN_PASS
#define WLAN_PASS "passwort"
#endif

#ifndef TWITCH_OAUTH_TOKEN
#define TWITCH_OAUTH_TOKEN "" // https://twitchapps.com/tmi/
#endif

#ifndef TWITCH_BOT_NAME
#define TWITCH_BOT_NAME "addyourusernamehere"
#endif

char ssid[] = WLAN_SSID;
char password[] = WLAN_PASS;

// 'refresh-256', 50x50px
const unsigned char myBitmap [] PROGMEM = {
	0x07, 0x80, 0x00, 0x00, 0x00, 0x07, 0xc1, 0xff, 0x80, 0x00, 0x07, 0xdf, 0xff, 0xf0, 0x00, 0x0f, 
	0xff, 0xff, 0xfe, 0x00, 0x0f, 0xff, 0xff, 0xff, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xff, 
	0xff, 0xff, 0xe0, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff, 
	0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0x80, 0x7f, 0x7f, 0xff, 0xf8, 0x00, 
	0x0f, 0x7f, 0xff, 0xc0, 0x00, 0x03, 0xff, 0xff, 0x80, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x00, 
	0x7f, 0xff, 0xc0, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 
	0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 
	0x03, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x00, 0x01, 
	0xff, 0xff, 0xc0, 0x00, 0x03, 0xff, 0xff, 0xf0, 0x00, 0x1f, 0xff, 0xfe, 0xfe, 0x00, 0xff, 0xff, 
	0xfc, 0x7f, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xff, 0xfc, 
	0x0f, 0xff, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xff, 0xf8, 0x00, 
	0xff, 0xff, 0xff, 0xf0, 0x00, 0x7f, 0xff, 0xff, 0xf0, 0x00, 0x0f, 0xff, 0xfb, 0xe0, 0x00, 0x01, 
	0xff, 0x83, 0xe0, 0x00, 0x00, 0x00, 0x01, 0xe0
};

#define HOSTNAME "TwitchBot"

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

String last_send = "";

//The name of the channel that you want the bot to join
const String twitchChannelName = "projektiontv";

String ircChannel = "";

WiFiClient wiFiClient;
IRCClient client(IRC_SERVER, IRC_PORT, wiFiClient);


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

#define NUMBER_OF_BUTTONS 13
TouchButton *numbers[NUMBER_OF_BUTTONS];

uint8_t radius = 5;

void ApplyLookAndFeel(TouchButton *btn)
{
    btn->SetOutlineStrength(3);
    btn->SetColorFill(ILI9341_RED);
    btn->SetRadius(radius);
}

void updateBanditCmd(String param)
{
    if (numbers[10]->GetText().length() < 10)
        numbers[10]->UpdateText(numbers[10]->GetText() + param);
}

void sendBanditCmd(String param)
{
    if (numbers[10]->GetText().length() == 10)
    {
        client.sendMessage(ircChannel, param);
        last_send = param;
        numbers[10]->UpdateText("!bandit");
    }
}

void resendBanditCmd(String param)
{
    if (last_send != "")
        client.sendMessage(ircChannel, last_send);
}

void clearBanditCmd(String param)
{
    numbers[10]->UpdateText("!bandit");
}

void drawButtons()
{
    tft.fillScreen(ILI9341_BLACK);// clear screen 
    int16_t x_coord = 10;
    for (int i = 0; i < 10; i++)
    {
        numbers[i] = new TouchButton(String(i), TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
        numbers[i]->registerCallback(updateBanditCmd);
        ApplyLookAndFeel(numbers[i]);
        numbers[i]->draw(&tft, x_coord, i < 5 ? 20 : 90, 50, 60);
        x_coord += 60;
        if (i == 4)
            x_coord = 10;
    }

    numbers[10] = new TouchButton("!bandit", TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
    ApplyLookAndFeel(numbers[10]);
    numbers[10]->registerCallback(sendBanditCmd);
    numbers[10]->draw(&tft, 10, 160, 170, 60);

    numbers[11] = new TouchButton("X", TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
    ApplyLookAndFeel(numbers[11]);
    numbers[11]->registerCallback(clearBanditCmd);
    numbers[11]->draw(&tft, 250, 160, 50, 60);

    numbers[12] = new TouchButton((uint8_t *)myBitmap, 40, 40, TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
    ApplyLookAndFeel(numbers[12]);
    numbers[12]->registerCallback(resendBanditCmd);
    numbers[12]->draw(&tft, 190, 160, 50, 60);
    //tft.drawBitmap(255, 170, myBitmap, 40, 40, ILI9341_WHITE);
}

bool buttons_drawn = false;

void setup() {
    pinMode(TFT_LED, OUTPUT); // define as output for backlight control

    tft.begin();          
    tft.setRotation(3);   // landscape mode  
    ts.begin();
    ts.setRotation(tft.getRotation());
    tft.fillScreen(ILI9341_BLACK);// clear screen 
    tft.setTextSize(1);
    tft.setCursor(0, 0);

    WiFi.disconnect();
    WiFi.setHostname(HOSTNAME);
    WiFi.mode(WIFI_STA);
    delay(100);
    
    ircChannel = "#" + twitchChannelName;

    // Attempt to connect to Wifi network:
    tft.print("Connecting Wifi: ");
    tft.println(ssid);
    WiFi.begin(ssid, password);
    int cnt = 0;
    while (WiFi.status() != WL_CONNECTED) {
        tft.print(".");
        cnt++;
        if (cnt > 10)
        {
            WiFi.begin(ssid, password);
            cnt = 0;
        }
        delay(500);
    }
    tft.println("");
    tft.println("WiFi connected");
    tft.println("IP address: ");
    IPAddress ip = WiFi.localIP();
    tft.println(ip);

    //delay(5000);

    //tft.fillScreen(ILI9341_BLACK);// clear screen 
    //drawButtons();

    ArduinoOTA.onStart([]() {
        tft.fillScreen(ILI9341_BLACK);// clear screen 
        tft.setTextSize(2);
        tft.setCursor(16, 16);
        tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
        int cmd = ArduinoOTA.getCommand();
        switch (cmd)
        {
        case U_FLASH:
            tft.println("Start uploading sketch...");
            break;
        case U_SPIFFS:
            tft.println("Start uploading fs...");
            break;
        default:
            tft.println("Start uploading...");
            break;
        }
    });
    ArduinoOTA.onEnd([]() {
        tft.setTextSize(2);
        tft.setCursor(16, 52);
        tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
        tft.println("Upload finished...");
        delay(2000);
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        tft.setTextSize(2);
        tft.setCursor(16, 34);
        tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
        tft.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        
        tft.setCursor(16, 68);
        tft.setTextColor(ILI9341_RED);
        tft.printf("Error[%u]: ", error);
        tft.setCursor(16, 86);
        if (error == OTA_AUTH_ERROR)
            tft.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
            tft.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
            tft.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
            tft.println("Receive Failed");
        else if (error == OTA_END_ERROR)
            tft.println("End Failed");

        TouchButton *reboot = new TouchButton("Reboot", TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
        reboot->registerCallback([](String t){ ESP.restart(); });
        ApplyLookAndFeel(reboot);
        reboot->draw(&tft, 40, 120, 240, 60);

        // delay(10000);
        // ESP.restart();
    });

    ArduinoOTA.setHostname(HOSTNAME);
    //ArduinoOTA.setPassword("test123");
    ArduinoOTA.begin();
}

void loop() {
    if (!client.connected())
    {
        if (buttons_drawn)
        {
            buttons_drawn = false;
            tft.fillScreen(ILI9341_BLACK);// clear screen 
            tft.setTextSize(1);
            tft.setCursor(0, 0);
        }
        tft.println("Attempting to connect to " + ircChannel );
        // Attempt to connect
        // Second param is not needed by Twtich
        if (client.connect(TWITCH_BOT_NAME, "", TWITCH_OAUTH_TOKEN))
        {
            client.sendRaw("JOIN " + ircChannel);
            tft.println("connected and ready to rock");
            delay(2000);
            //sendTwitchMessage("Ready to go Boss!");
        } else {
            tft.println("failed... try again in 2 seconds");
            // Wait 5 seconds before retrying
            delay(2000);
        }
    }
    else
    {
        if (!buttons_drawn)
        {
            //tft.fillScreen(ILI9341_BLACK);// clear screen 
            drawButtons();
            buttons_drawn = true;
        }
        // put your main code here, to run repeatedly:
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
            numbers[i]->isPressed(x, y);
        }
        delay(50);
    }
    ArduinoOTA.handle();
}