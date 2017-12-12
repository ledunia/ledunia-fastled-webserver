# ledunia-fastled-webserver

### Tutorial:
![alt text](https://github.com/ledunia/ledunia-fastled-webserver/blob/master/ledcontroller.PNG)

Prepare the Arduino IDE for ledunia support: http://ledunia.de/index.php/2017-07-24-21-18-46/start

The app depends on the following libraries. They must either be downloaded from GitHub and placed in the Arduino 'libraries' folder, or installed as described here by using the Arduino library manager (->Sketch->Include Libraries->Manage Libraries):

* FastLED (Version 3.1.3)
* IRremoteESP8266
* Arduino WebSockets

Download the app code from GitHub using the green Clone or Download button from the GitHub project main page and click Download ZIP. Decompress the ZIP file in your Arduino sketch folder.

The web app needs to be uploaded to the ledunia's SPIFFS. You can do this within the Arduino IDE after installing the Arduino ESP8266FS tool: https://github.com/ledunia/arduino-esp8266fs-plugin

* Don't forget to change "SSID", "Password" and the "DNS Name" in the sketch
* Navigate to: ->Tools->ESP8266 Sketch Data Upload (this upload process will take a while)
* Upload the sketch using the Upload button
* Browse to http://Your-IP-or-DNS-Name (first load will take a while, be patient)

You can use additional WS2812b (aka Neopixel) LEDs by using ledunia's GPIO "DOUT".
