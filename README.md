# ledunia-fastled-webserver

### Tutorial:
![alt text](https://github.com/ledunia/ledunia-fastled-webserver/blob/master/ledcontroller.PNG)

* Prepare the Arduino IDE for ledunia support: http://ledunia.de/index.php/2017-07-24-21-18-46/start
* Import the "Fastled.h" library: ->Sketch->Include Libraries->Manage Libraries (use version 3.1.3)
* Don't forget to change "SSID", "Password" and the "DNS Name" in the sketch
* Upload the sketch: ledunia-fastled-webserver.ino
* Navigate to: ->Tools->ESP8266 Sketch Data Upload (this upload process will take a while)
* Browse to http://Your-IP-or-DNS-Name (first load will take a while, be patient)

You can additional WS2812B (aka Neopixel) LEDs by using GPIO DOUT.
