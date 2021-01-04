## Hardware

Dieses Projekt ist ausgelegt für den [AZ-Touch ESP](https://www.hwhardsoft.de/deutsch/projekte/arduitouch-esp/) in Verbindung mit einem ESP32-Board.

## First Setup

Da dieses Projekt für OTA ausgelegt ist, muss vor dem ersten Flashen etwas geändert werden - die Datei [platformio.ini](https://github.com/quotschmacher/twitch_things/blob/master/bandit_bot_touch/platformio.ini) muss bearbeitet werden. Dazu einfach die Zeile
```upload_port = BanditTouchBot.local```
auskommentieren, ESP flashen und danach wieder einkommentieren.
