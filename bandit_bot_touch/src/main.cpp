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

#define HOSTNAME "BanditTouchBot"

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

//The name of the channel that you want the bot to join
const String twitchChannelName = "projektiontv";

String ircChannel = "";

WiFiClient wiFiClient;
IRCClient client(IRC_SERVER, IRC_PORT, wiFiClient);


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

#define NUMBER_OF_BUTTONS 12
TouchButton *numbers[NUMBER_OF_BUTTONS];

uint8_t radius = 5;

void updateBanditCmd(String param)
{
    numbers[10]->UpdateText(numbers[10]->GetText() + param);
}

void sendBanditCmd(String param)
{
    client.sendMessage(ircChannel, param);
    numbers[10]->UpdateText("!bandit");
}

void clearBanditCmd(String param)
{
    numbers[10]->UpdateText("!bandit");
}

void drawButtons()
{
    int16_t x_coord = 10;
    for (int i = 0; i < 10; i++)
    {
        numbers[i] = new TouchButton(String(i), TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
        numbers[i]->SetOutlineStrength(3);
        numbers[i]->SetColorFill(ILI9341_RED);
        numbers[i]->registerCallback(updateBanditCmd);
        numbers[i]->SetRadius(radius);
        numbers[i]->draw(&tft, x_coord, i < 5 ? 20 : 90, 50, 60);
        x_coord += 60;
        if (i == 4)
            x_coord = 10;
    }

    numbers[10] = new TouchButton("!bandit", TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
    numbers[10]->SetOutlineStrength(3);
    numbers[10]->SetColorFill(ILI9341_RED);
    numbers[10]->registerCallback(sendBanditCmd);
    numbers[10]->SetRadius(radius);
    numbers[10]->draw(&tft, 10, 160, 170, 60);

    numbers[11] = new TouchButton("X", TS_MINX, TS_MAXX, TS_MINY, TS_MAXY);
    numbers[11]->SetOutlineStrength(3);
    numbers[11]->SetColorFill(ILI9341_RED);
    numbers[11]->registerCallback(clearBanditCmd);
    numbers[11]->SetRadius(radius);
    numbers[11]->draw(&tft, 250, 160, 50, 60);
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

    WiFi.setHostname(HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
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
            tft.fillScreen(ILI9341_BLACK);// clear screen 
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