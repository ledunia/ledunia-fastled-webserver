/*
 __         __            _
 / /__  ____/ /_  ______  (_)___ _
 / / _ \/ __  / / / / __ \/ / __ `/
 / /  __/ /_/ / /_/ / / / / / /_/ /
 /_/\___/\__,_/\__,_/_/ /_/_/\__,_/


 ledunia + FastLED + IR Remote + MSGEQ7

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.


 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "FastLED.h" // Tested Version 3.1.3

FASTLED_USING_NAMESPACE

extern "C" {
#include "user_interface.h"
}

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <IRremoteESP8266.h>
#include "GradientPalettes.h"
#include <IRrecv.h>

#define RECV_PIN 12
IRrecv irReceiver(RECV_PIN);
MDNSResponder mdns;

#include "Commands.h"

const bool apMode = true;

// AP mode password
const char WiFiAPPSK[] = "";

// Wi-Fi network to connect to (if not in AP mode)
const char* ssid = "Heimnetz";
const char* password = "RM0uCWD3GL5JyXYGFKdNQdYit";

ESP8266WebServer server(80);

#define DATA_PIN            4     // for ledunia
#define LED_TYPE            WS2812
#define COLOR_ORDER         GRB
#define NUM_LEDS            4    // number of on-board WS2812b leds
#define MILLI_AMPS         1000     // IMPORTANT: set here the max milli-Amps of your power supply 5V 2A = 2000
#define FRAMES_PER_SECOND  120 // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.

CRGB leds[NUM_LEDS];
// int temp=0;
// int i=0;
uint8_t patternIndex = 0;

//--------- Brightness   ------------------------------------------
const uint8_t brightnessCount = 251;
uint8_t brightnessMap[brightnessCount] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
		66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83,
		84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100,
		101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
		115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128,
		129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
		143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156,
		157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170,
		171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184,
		185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198,
		199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212,
		213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
		227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240,
		241, 242, 243, 244, 245, 246, 247, 248, 249, 250 };
int brightnessIndex = 190;
uint8_t brightness = brightnessMap[brightnessIndex];

//--------- BPM is close ------------------------------------------
const uint8_t bpmCount = 251;
uint8_t bpmMap[bpmCount] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
		33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
		51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68,
		69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
		87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103,
		104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117,
		118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
		132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145,
		146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
		160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173,
		174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187,
		188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201,
		202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215,
		216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
		230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243,
		244, 245, 246, 247, 248, 249, 250 };
int bpmIndex = 14;
uint8_t bpm = bpmMap[bpmIndex];

//--------- SPEED ------------------------------------------
const uint8_t speedCount = 251;
uint8_t speedMap[speedCount] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
		14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
		50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
		68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
		86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
		103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
		117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
		131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
		145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
		159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172,
		173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186,
		187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
		201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
		215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228,
		229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242,
		243, 244, 245, 246, 247, 248, 249, 250 };
int speedIndex = 120;
uint8_t speed = speedMap[speedIndex];

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
// ten seconds per color palette makes a good demo
// 20-120 is better for deployment
#define SECONDS_PER_PALETTE 10

///////////////////////////////////////////////////////////////////////

// Forward declarations of an array of cpt-city gradient palettes, and
// a count of how many there are.  The actual color palette definitions
// are at the bottom of this file.

extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;
extern const TProgmemPalette16 myRedBluePalette_p;
extern const TProgmemPalette16 myBlueRedPalette_p;
extern const TProgmemPalette16 myRedWhiteBluePalette_p;
extern const TProgmemPalette16 myRedWhiteBluePalette_p;
// Current palette number from the 'playlist' of color palettes
uint8_t gCurrentPaletteNumber = 0;
CRGBPalette16 palette;
CRGBPalette16 currentPalette;
CRGBPalette16 bluePalette;
CRGBPalette16 redPalette;
CRGBPalette16 gCurrentPalette(CRGB::Black);
CRGBPalette16 gTargetPalette(gGradientPalettes[0]);
//CRGBPalette16 frankreich(gGradientPalettes[0]);
//CRGBPalette16 hellblau(gGradientPalettes[1]);
//CRGBPalette16 hellrot(gGradientPalettes[2]);

uint8_t currentPatternIndex = 0; // Index number of which pattern is current
uint8_t autoplay = 0;
uint8_t forward = 1;
uint8_t autoPlayDurationSeconds = 10;
unsigned int autoPlayTimeout = 0;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGB solidColor = CRGB::Blue;

uint8_t power = 1;

void setup(void) {
	Serial.begin(115200);
	delay(100);
	Serial.setDebugOutput(true);

	//#define DATA_PIN      4     // for ledunia
	//#define LED_TYPE      WS2812
	//#define COLOR_ORDER   GRB
	//#define NUM_LEDS      4     // number of on-board WS2812b leds
	//#define MILLI_AMPS         1000     // IMPORTANT: set here the max milli-Amps of your power supply 5V 2A = 2000
	//#define FRAMES_PER_SECOND  120 // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.
//  speed = 60;
//  bpm = 12;
	FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS); // for WS2812 (Neopixel)
	//FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS); // for APA102 (Dotstar)
	FastLED.setCorrection(TypicalLEDStrip);
	FastLED.setBrightness(brightness);

	FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
	fill_solid(leds, NUM_LEDS, solidColor);
	setBpm(bpm);
	setSpeed(speed);
	FastLED.show();
	EEPROM.begin(512);

	//---------------------------------------------

	loadSettings();

	//----------------------------------------------
	irReceiver.enableIRIn(); // Start the receiver

	Serial.println();
	Serial.print(F("Heap: "));
	Serial.println(system_get_free_heap_size());
	Serial.print(F("Boot Vers: "));
	Serial.println(system_get_boot_version());
	Serial.print(F("CPU: "));
	Serial.println(system_get_cpu_freq());
	Serial.print(F("SDK: "));
	Serial.println(system_get_sdk_version());
	Serial.print(F("Chip ID: "));
	Serial.println(system_get_chip_id());
	Serial.print(F("Flash ID: "));
	Serial.println(spi_flash_get_id());
	Serial.print(F("Flash Size: "));
	Serial.println(ESP.getFlashChipRealSize());
	Serial.print(F("Vcc: "));
	Serial.println(ESP.getVcc());
	Serial.println();

	SPIFFS.begin();
	{
		Dir dir = SPIFFS.openDir("/");
		while (dir.next()) {
			String fileName = dir.fileName();
			size_t fileSize = dir.fileSize();
			Serial.printf("FS File: %s, size: %s\n", fileName.c_str(),
					String(fileSize).c_str());
		}
		Serial.printf("\n");
	}

	if (apMode) {
		WiFi.mode(WIFI_AP);

		// Do a little work to get a unique-ish name. Append the
		// last two bytes of the MAC (HEX'd) to "Thing-":
		uint8_t mac[WL_MAC_ADDR_LENGTH];
		WiFi.softAPmacAddress(mac);
		String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX)
				+ String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
		macID.toUpperCase();
		//String AP_NameString = "ESP8266 Thing " + macID;
		String AP_NameString = "Auge_links";

		char AP_NameChar[AP_NameString.length() + 1];
		memset(AP_NameChar, 0, AP_NameString.length() + 1);

		for (int i = 0; i < AP_NameString.length(); i++)
			AP_NameChar[i] = AP_NameString.charAt(i);

		WiFi.softAP(AP_NameChar, WiFiAPPSK);

		Serial.printf("Connect to Wi-Fi access point: %s\n", AP_NameChar);
		Serial.println("and open http://192.168.4.1 in your browser");
	} else {
		Serial.printf("Connecting to %s\n", ssid);
		if (String(WiFi.SSID()) != String(ssid)) {
			WiFi.begin(ssid, password);
		}

		while (WiFi.status() != WL_CONNECTED) {
			delay(500);
			Serial.print(".");
		}
		Serial.println(WiFi.localIP());
		if (mdns.begin("ledunia", WiFi.localIP())) {
			Serial.println("MDNS responder started");
		}
		Serial.print("Connected! Open http://");
		Serial.print(WiFi.localIP());
		Serial.println(" in your browser");
	}

//  server.serveStatic("/", SPIFFS, "/index.htm"); // ,"max-age=86400"

	server.on("/all", HTTP_GET, []() {
		sendAll();
	});

	server.on("/power", HTTP_GET, []() {
		sendPower();
	});

	server.on("/power", HTTP_POST, []() {
		String value = server.arg("value");
		setPower(value.toInt());
		sendPower();
	});

	server.on("/autoplay", HTTP_GET, []() {
		sendAutoplay();
	});

	server.on("/autoplay", HTTP_POST, []() {
		String value = server.arg("value");
		setAutoplay(value.toInt());
		sendAutoplay();
	});

	server.on("/forward", HTTP_GET, []() {
		sendForward();
	});

	server.on("/forward", HTTP_POST, []() {
		String value = server.arg("value");
		setForward(value.toInt());
		sendForward();
	});

	server.on("/solidColor", HTTP_GET, []() {
		sendSolidColor();
	});

	server.on("/solidColor", HTTP_POST, []() {
		String r = server.arg("r");
		String g = server.arg("g");
		String b = server.arg("b");
		setSolidColor(r.toInt(), g.toInt(), b.toInt());
		sendSolidColor();
	});

	server.on("/pattern", HTTP_GET, []() {
		sendPattern();
	});

	server.on("/pattern", HTTP_POST, []() {
		String value = server.arg("value");
		setPattern(value.toInt());
		sendPattern();
	});

	server.on("/patternUp", HTTP_POST, []() {
		adjustPattern(true);
		sendPattern();
	});

	server.on("/patternDown", HTTP_POST, []() {
		adjustPattern(false);
		sendPattern();
	});

	server.on("/brightness", HTTP_GET, []() {
		sendBrightness();
	});

	server.on("/brightness", HTTP_POST, []() {
		String value = server.arg("value");
		setBrightness(value.toInt());
		sendBrightness();
	});

	server.on("/brightnessUp", HTTP_POST, []() {
		adjustBrightness(true);
		sendBrightness();
	});

	server.on("/brightnessDown", HTTP_POST, []() {
		adjustBrightness(false);
		sendBrightness();
	});

	server.on("/bpm", HTTP_GET, []() {
		sendBpm();
	});

	server.on("/bpm", HTTP_POST, []() {
		String value = server.arg("value");
		setBpm(value.toInt());
		sendBpm();
	});

	server.on("/bpmUp", HTTP_POST, []() {
		adjustBpm(true);
		sendBpm();
	});

	server.on("/bpmDown", HTTP_POST, []() {
		adjustBpm(false);
		sendBpm();
	});
	server.on("/speed", HTTP_GET, []() {
		sendSpeed();
	});

	server.on("/speed", HTTP_POST, []() {
		String value = server.arg("value");
		setSpeed(value.toInt());
		sendSpeed();
	});

	server.on("/speedUp", HTTP_POST, []() {
		adjustSpeed(true);
		sendSpeed();
	});

	server.on("/speedDown", HTTP_POST, []() {
		adjustSpeed(false);
		sendSpeed();
	});

	server.serveStatic("/index.htm", SPIFFS, "/index.htm");
	server.serveStatic("/fonts", SPIFFS, "/fonts", "max-age=86400");
	server.serveStatic("/js", SPIFFS, "/js");
	server.serveStatic("/css", SPIFFS, "/css", "max-age=86400");
	server.serveStatic("/images", SPIFFS, "/images", "max-age=86400");
	server.serveStatic("/", SPIFFS, "/index.htm");

	server.begin();

	Serial.println("HTTP server started");

	autoPlayTimeout = millis() + (autoPlayDurationSeconds * 1000);

}
typedef void (*Pattern)();
typedef Pattern PatternList[];
typedef struct {
	Pattern pattern;
	String name;
} PatternAndName;
typedef PatternAndName PatternAndNameList[];

// List of patterns to cycle through.  Each is defined as a separate function below.
PatternAndNameList patterns = { { colorwaves, "Color Waves" }, { palettetest,
		"Palette Test" }, { pride, "Pride" }, { rainbow, "Rainbow" }, {
		rainbowWithGlitter, "Rainbow With Glitter" }, { confetti, "Confetti" },
		{ sinelon, "Sinelon" }, { juggle, "Juggle" }, { bpm_pal, "BPM" }, {
				uerdingen, "KFC Uerdingen 05" }, { uerdingenWithGlitter,
				"KFC Uerdingen 05 Glitter" }, { redWithGlitter,
				"Rot mit Glitter" }, { blueWithGlitter, "Blau mit Glitter" }, {
				lakers, "L.A. Lakers" }, { rotweissblau, "Rot Weiss Blau" }, {
				schwarzweiss, "Schwarz Weiss" },
		{ showSolidColor, "Solid Color" },
    {bl_blue_blueplus_blue_bl ,"Uerdingen Blau Gradient"},{bl_red_redplus_red_bl,"Uerdingen Rot Gradient"},
		{ frankreich, "Frankreich Gradient" },
		{ hellblau, "Hellblau Gradient" }, { hellrot, "Hellrot Gradient" }, {
				backAndForth, "function Back and Forth" },
				{  phase,"Phase Gradient"},{  gay_flag_1978,"Gay Gradient"},
        {  gay_flag_1978_gHue,"Gay Gradient gHue"},
				{  schwarzrotblau,"Schwarz Rot Blau Gradient"},{  schwarzrotblautight,"Schwarz Rot Blau Schnell"},
        {  schwarzrotblautight_gHue,"Schwarz Rot Blau Schnell gHue"},
				  { one_gp,"es_rivendell_15_gp" }, { two_gp, "es_ocean_breeze_036_gp" }, {
				three_gp, "rgi_15_gp" }, { four_gp, "retro2_16_gp" }, { five_gp,
				"Analogous_1_gp" }, { six_gp, "es_pinksplash_08_gp" }, {
				seven_gp, "Coral_reef_gp" }, { eight_gp,
				"es_ocean_breeze_068_gp" }, { nine_gp, "es_pinksplash_07_gp" },
		{ ten_gp, "es_vintage_01_gp" }, { eleven_gp, "departure_gp" }, {
				twelve_gp, "es_landscape_64_gp" }, { thirteen_gp,
				"es_landscape_33_gp" }, { fourteen_gp, "rainbowsherbet_gp" }, {
				fifteen_gp, "gr65_hult_gp" }, { sixteen_gp, "gr64_hult_gp" }, {
				seventeen_gp, "GMT_drywet_gp" }, { eighteen_gp, "ib_jul01_gp" },
		{ twenty_gp, "es_vintage_57_gp" }, { twentyone_gp, "ib15_gp" }, {
				twentytwo_gp, "Fuschia_7_gp" }, { twentythree_gp,
				"es_emerald_dragon_08_gp" }, { twentyfour_gp, "lava_gp" }, {
				twentyfive_gp, "fire_gp" }, { twentysix_gp, "Colorfull_gp" }, {
				twentyseven_gp, "Magenta_Evening_gp" }, { twentyeight_gp,
				"Pink_Purple_gp" }, { twentynine_gp, "es_autumn_19_gp" }, {
				thirty_gp, "Sunset_Real_gp" }, { thirtyone_gp,
				"BlacK_Blue_Magenta_White_gp" }, { thirtytwo_gp,
				"BlacK_Magenta_Red_gp" }, { thirtythree_gp,
				"BlacK_Red_Magenta_Yellow_gp" }, { thirtyfour_gp,
				"Blue_Cyan_Yellow_gp" } };

const uint8_t patternCount = ARRAY_SIZE(patterns);

void loop(void) {
	// Add entropy to random number generator; we use a lot of it.
	random16_add_entropy(random(65535));

	server.handleClient();

	// handleIrInput();

	if (power == 0) { // Power off : schwarz schalten
		fill_solid(leds, NUM_LEDS, CRGB::Black);
		FastLED.show();
		FastLED.delay(15);
		return;
	}

	// EVERY_N_SECONDS(10) {
	//   Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
	// }

	EVERY_N_MILLISECONDS(20)
	{
		//gHue++;  // slowly cycle the "base color" through the rainbow
		gHue = forward_gHue(forward, gHue); // gHue++ or gHue-- (Boolean forward)
	}

	// change to a new cpt-city gradient palette
	EVERY_N_SECONDS( SECONDS_PER_PALETTE)
	{
		gCurrentPaletteNumber = addmod8(gCurrentPaletteNumber, 1,
				gGradientPaletteCount);
		gTargetPalette = gGradientPalettes[gCurrentPaletteNumber];
	}

	// slowly blend the current cpt-city gradient palette to the next
	EVERY_N_MILLISECONDS(40)
	{
		nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, 16);
	}

	if (autoplay == 1 && millis() > autoPlayTimeout) {
		adjustPattern(true);
		autoPlayTimeout = millis() + (autoPlayDurationSeconds * 1000);
	}

	// Call the current pattern function once, updating the 'leds' array
	patterns[currentPatternIndex].pattern();

	FastLED.show();

	// insert a delay to keep the framerate modest --Speed variable ---
	FastLED.delay(1000 / speed);

}
//void handleIrInput()
//{
//  InputCommand command = readCommand(defaultHoldDelay);
//
//  if (command != InputCommand::None) {
//    Serial.print("command: ");
//    Serial.println((int) command);
//  }
//
//  switch (command) {
//    case InputCommand::Up: {
//        adjustPattern(true);
//        break;
//      }
//    case InputCommand::Down: {
//        adjustPattern(false);
//        break;
//      }
//    case InputCommand::Power: {
//        power = power == 0 ? 1 : 0;
//        break;
//      }
//    case InputCommand::BrightnessUp: {
//        adjustBrightness(true);
//        break;
//      }
//    case InputCommand::BrightnessDown: {
//        adjustBrightness(false);
//        break;
//      }
//     case InputCommand::BpmUp: {
//        adjustBpm(true);
//        break;
//      }
//    case InputCommand::BpmDown: {
//        adjustBpm(false);
//        break;
//      }
//
//    case InputCommand::PlayMode: { // toggle pause/play
//        autoplayEnabled = !autoplayEnabled;
//        break;
//      }
//
//    // pattern buttons
//
//    case InputCommand::Pattern1: {
//        setPattern(0);
//        break;
//      }
//    case InputCommand::Pattern2: {
//        setPattern(1);
//        break;
//      }
//    case InputCommand::Pattern3: {
//        setPattern(2);
//        break;
//      }
//    case InputCommand::Pattern4: {
//        setPattern(3);
//        break;
//      }
//    case InputCommand::Pattern5: {
//        setPattern(4);
//        break;
//      }
//    case InputCommand::Pattern6: {
//        setPattern(5);
//        break;
//      }
//    case InputCommand::Pattern7: {
//        setPattern(6);
//        break;
//      }
//    case InputCommand::Pattern8: {
//        setPattern(7);
//        break;
//      }
//    case InputCommand::Pattern9: {
//        setPattern(8);
//        break;
//      }
//    case InputCommand::Pattern10: {
//        setPattern(9);
//        break;
//      }
//    case InputCommand::Pattern11: {
//        setPattern(10);
//        break;
//      }
//    case InputCommand::Pattern12: {
//        setPattern(11);
//        break;
//      }
//
//    // custom color adjustment buttons
//
//    case InputCommand::RedUp: {
//        solidColor.red += 8;
//        setSolidColor(solidColor);
//        break;
//      }
//    case InputCommand::RedDown: {
//        solidColor.red -= 8;
//        setSolidColor(solidColor);
//        break;
//      }
//    case InputCommand::GreenUp: {
//        solidColor.green += 8;
//        setSolidColor(solidColor);
//        break;
//      }
//    case InputCommand::GreenDown: {
//        solidColor.green -= 8;
//        setSolidColor(solidColor);
//        break;
//      }
//    case InputCommand::BlueUp: {
//        solidColor.blue += 8;
//        setSolidColor(solidColor);
//        break;
//      }
//    case InputCommand::BlueDown: {
//        solidColor.blue -= 8;
//        setSolidColor(solidColor);
//        break;
//      }
//
//    // color buttons
//      case InputCommand::White: {
//        setSolidColor(CRGB::White);
//        break;
//      }
//    case InputCommand::Red: {
//        setSolidColor(CRGB::Red);
//        break;
//      }
//    case InputCommand::RedOrange: {
//        setSolidColor(CRGB::OrangeRed);
//        break;
//      }
//    case InputCommand::Orange: {
//        setSolidColor(CRGB::Orange);
//        break;
//      }
//    case InputCommand::YellowOrange: {
//        setSolidColor(CRGB::Goldenrod);
//        break;
//      }
//    case InputCommand::Yellow: {
//        setSolidColor(CRGB::Yellow);
//        break;
//      }
//
//    case InputCommand::Green: {
//        setSolidColor(CRGB::Green);
//        break;
//      }
//    case InputCommand::Lime: {
//        setSolidColor(CRGB::Lime);
//        break;
//      }
//    case InputCommand::Aqua: {
//        setSolidColor(CRGB::Aqua);
//        break;
//      }
//    case InputCommand::Teal: {
//        setSolidColor(CRGB::Teal);
//        break;
//      }
//    case InputCommand::Navy: {
//        setSolidColor(CRGB::Navy);
//        break;
//      }
//
//    case InputCommand::Blue: {
//        setSolidColor(CRGB::Blue);
//        break;
//      }
//    case InputCommand::RoyalBlue: {
//        setSolidColor(CRGB::RoyalBlue);
//        break;
//      }
//    case InputCommand::Purple: {
//        setSolidColor(CRGB::Purple);
//        break;
//      }
//    case InputCommand::Indigo: {
//        setSolidColor(CRGB::Indigo);
//        break;
//      }
//    case InputCommand::Magenta: {
//        setSolidColor(CRGB::Magenta);
//        break;
//      }
//
//
//
//    case InputCommand::Pink: {
//        setSolidColor(CRGB::Pink);
//        break;
//      }
//    case InputCommand::LightPink: {
//        setSolidColor(CRGB::LightPink);
//        break;
//      }
//    case InputCommand::BabyBlue: {
//        setSolidColor(CRGB::CornflowerBlue);
//        break;
//      }
//    case InputCommand::LightBlue: {
//        setSolidColor(CRGB::LightBlue);
//        break;
//      }
//  }
//}

void loadSettings() {
	brightness = EEPROM.read(0);

	currentPatternIndex = EEPROM.read(1);
	if (currentPatternIndex < 0) {
		currentPatternIndex = 0;
	} else if (currentPatternIndex >= patternCount) {
		currentPatternIndex = patternCount - 1;
	}

	byte r = EEPROM.read(2);
	byte g = EEPROM.read(3);
	byte b = EEPROM.read(4);
	bpm = EEPROM.read(5);
	speed = EEPROM.read(6);
	power = EEPROM.read(7);
	//autoplay = EEPROM.read(8);
	forward = EEPROM.read(9);

	if (r == 0 && g == 0 && b == 0) {
	} else {
		solidColor = CRGB(r, g, b);
	}
}

void sendAll() {
	String json = "{";

	json += "\"power\":" + String(power) + ",";
	json += "\"autoplay\":" + String(autoplay) + ",";
	json += "\"forward\":" + String(forward) + ",";
	json += "\"brightness\":" + String(brightness) + ",";
	json += "\"bpm\":" + String(bpm) + ",";
	json += "\"speed\":" + String(speed) + ",";

	json += "\"currentPattern\":{";
	json += "\"index\":" + String(currentPatternIndex);
	json += ",\"name\":\"" + patterns[currentPatternIndex].name + "\"}";

	json += ",\"solidColor\":{";
	json += "\"r\":" + String(solidColor.r);
	json += ",\"g\":" + String(solidColor.g);
	json += ",\"b\":" + String(solidColor.b);
	json += "}";

	json += ",\"patterns\":[";
	for (uint8_t i = 0; i < patternCount; i++) {
		json += "\"" + patterns[i].name + "\"";
		if (i < patternCount - 1)
			json += ",";
	}
	json += "]";

	json += "}";

	server.send(200, "text/json", json);
	json = String();
}

void sendPower() {
	String json = String(power);
	server.send(200, "text/json", json);
	json = String();
}

void sendAutoplay() {
	String json = String(autoplay);
	server.send(200, "text/json", json);
	json = String();
}

void sendForward() {
	String json = String(forward);
	server.send(200, "text/json", json);
	json = String();
}

void sendPattern() {
	String json = "{";
	json += "\"index\":" + String(currentPatternIndex);
	json += ",\"name\":\"" + patterns[currentPatternIndex].name + "\"";
	json += "}";
	server.send(200, "text/json", json);
	json = String();
}

void sendBrightness() {
	String json = String(brightness);
	server.send(200, "text/json", json);
	json = String();
}

void sendBpm() {
	String json = String(bpm);
	server.send(200, "text/json", json);
	json = String();
}

void sendSpeed() {
	String json = String(speed);
	server.send(200, "text/json", json);
	json = String();
}

void sendSolidColor() {
	String json = "{";
	json += "\"r\":" + String(solidColor.r);
	json += ",\"g\":" + String(solidColor.g);
	json += ",\"b\":" + String(solidColor.b);
	json += "}";
	server.send(200, "text/json", json);
	json = String();
}

void setPower(uint8_t value) {
	power = value == 0 ? 0 : 1;
	EEPROM.write(7, power);
	EEPROM.commit();
}

void setAutoplay(uint8_t value) {
	autoplay = value == 0 ? 0 : 1;
	EEPROM.write(8, autoplay);
	EEPROM.commit();

}

void setForward(uint8_t value) {
	forward = value == 0 ? 0 : 1;
	EEPROM.write(9, forward);
	EEPROM.commit();
}

void setSolidColor(CRGB color) {
	setSolidColor(color.r, color.g, color.b);
}

void setSolidColor(uint8_t r, uint8_t g, uint8_t b) {
	solidColor = CRGB(r, g, b);

	EEPROM.write(2, r);
	EEPROM.write(3, g);
	EEPROM.write(4, b);

// Solidcolor index nr. 13
	setPattern(16);
}

// increase or decrease the current pattern number, and wrap around at the ends
void adjustPattern(bool up) {
	if (up)
		currentPatternIndex++;
	else
		currentPatternIndex--;

	// wrap around at the ends
	if (currentPatternIndex < 0)
		currentPatternIndex = patternCount - 1;
	if (currentPatternIndex >= patternCount)
		currentPatternIndex = 0;

	EEPROM.write(1, currentPatternIndex);
	EEPROM.commit();
}

void setPattern(int value) {
	// don't wrap around at the ends
	if (value < 0)
		value = 0;
	else if (value >= patternCount)
		value = patternCount - 1;

	currentPatternIndex = value;

	EEPROM.write(1, currentPatternIndex);
	EEPROM.commit();
}

// adjust the brightness, and wrap around at the ends
void adjustBrightness(bool up) {
	if (up)
		brightnessIndex++;
	else
		brightnessIndex--;

	// wrap around at the ends
	if (brightnessIndex < 0)
		brightnessIndex = brightnessCount - 1;
	else if (brightnessIndex >= brightnessCount)
		brightnessIndex = 0;

	brightness = brightnessMap[brightnessIndex];

	FastLED.setBrightness(brightness);

	EEPROM.write(0, brightness);
	EEPROM.commit();
}

void setBrightness(int value) {
	// don't wrap around at the ends
	if (value > 250)
		value = 250;
	else if (value < 0)
		value = 0;

	brightness = value;

	FastLED.setBrightness(brightness);

	EEPROM.write(0, brightness);
	EEPROM.commit();
}

void adjustBpm(bool up) {
	if (up)
		bpmIndex++;
	else
		bpmIndex--;

	// wrap around at the ends
	if (bpmIndex < 1)
		bpmIndex = bpmCount - 1;
	else if (bpmIndex >= bpmCount)
		bpmIndex = 0;

	bpm = bpmMap[bpmIndex];
	EEPROM.write(5, bpm);
	EEPROM.commit();
}

void setBpm(int value) {
	// don't wrap around at the ends
	if (value > 250)
		value = 250;
	else if (value < 1)
		value = 1;

	bpm = value;
	EEPROM.write(5, bpm);
	EEPROM.commit();
}

void adjustSpeed(bool up) {
	if (up)
		speedIndex++;
	else
		speedIndex--;

	// wrap around at the ends
	if (speedIndex < 0)
		speedIndex = speedCount - 1;
	else if (speedIndex >= speedCount)
		speedIndex = 0;

	speed = speedMap[speedIndex];
	EEPROM.write(6, speed);
	EEPROM.commit();
}

void setSpeed(int value) {
	// don't wrap around at the ends
	if (value > 255)
		value = 255;
	else if (value < 1)
		value = 1;

	speed = value;
	EEPROM.write(6, speed);
	EEPROM.commit();
}

void showSolidColor() {
	fill_solid(leds, NUM_LEDS, solidColor);
//  leds[0]= solidColor;
//  leds[2]= solidColor;
}

void rainbow() {
	// FastLED's built-in rainbow generator
	fill_rainbow(leds, NUM_LEDS, gHue, 10);
}

void rainbowWithGlitter() {
	// built-in FastLED rainbow, plus some random sparkly glitter
	rainbow();
	addGlitter(80);
}

void redWithGlitter() {
	// built-in FastLED red, plus some random sparkly glitter
	static uint8_t startindex = 0;
	CRGB red = CHSV(HUE_RED, 255, 255);
	currentPalette = CRGBPalette16(red, red, red, red, red, red, red, red, red,
			red, red, red, red, red, red, red);

	startindex = forward_startindex(forward, startindex);
	fill_palette(leds, NUM_LEDS, startindex, (256 / NUM_LEDS) + 1,
			currentPalette, 255, LINEARBLEND);
	addGlitter(80);
}

void blueWithGlitter() {
	// built-in FastLED blue, plus some random sparkly glitter
	static uint8_t startindex = 0;
	CRGB blue = CHSV(HUE_BLUE, 255, 255);
	currentPalette = CRGBPalette16(blue, blue, blue, blue, blue, blue, blue,
			blue, blue, blue, blue, blue, blue, blue, blue, blue);
	startindex = forward_startindex(forward, startindex);
	fill_palette(leds, NUM_LEDS, startindex, (256 / NUM_LEDS) + 1,
			currentPalette, 255, LINEARBLEND);
	addGlitter(80);
}

void uerdingenWithGlitter() {
	static uint8_t startindex = 0;
	// built-in FastLED blue, plus some random sparkly glitter
	CRGB red = CHSV(HUE_RED, 255, 255);
	CRGB blue = CHSV(HUE_BLUE, 255, 255);
	currentPalette = CRGBPalette16(red, red, red, red, red, red, red, red, blue,
			blue, blue, blue, blue, blue, blue, blue);
	startindex = forward_startindex(forward, startindex);
	fill_palette(leds, NUM_LEDS, startindex, (256 / NUM_LEDS) + 1,
			currentPalette, 255, LINEARBLEND);
	addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter) {
	if (random8() < chanceOfGlitter) {
		leds[random16(NUM_LEDS)] += CRGB::White;
	}
}

//--
int rand_forward(fract8 chanceOfForwardChange,int forward){
int arr[] = { 1, 2, 3, 4 };
int el=0;
	if ( random8() < chanceOfForwardChange) {
		arr[ random16(4) ] = 10;
	

	for (int i = 0; i < 4; i++) {
  el = arr[i];

		switch (el) {
			case 1: {
					break;
			}
			case 2: {
					break;
			}
			case 3: {
					break;
			}
			case 4: {
					break;
			}
			case 10: {
				if (forward==0) {
					forward =1;
					int arr[]= {1,2,3,4};
					break;
				} else {
					forward=0;
					int arr[]= {1,2,3,4};
         break;
				}
			}
		}
  
	}
	}
  return forward;
}

void confetti() {
	// random colored speckles that blink in and fade smoothly
	fadeToBlackBy(leds, NUM_LEDS, 10);
	int pos = random16(NUM_LEDS);
	leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon() {
	// a colored dot sweeping back and forth, with fading trails
	fadeToBlackBy(leds, NUM_LEDS, 20);
	int pos = beatsin16(13, 0, NUM_LEDS - 1);
	leds[pos] += CHSV(gHue, 255, 192);
}

void bpm_pal() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
	uint8_t BeatsPerMinute = 62;
	CRGBPalette16 palette = PartyColors_p;
	uint8_t beat = beatsin8(bpm, 128, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		leds[i] = ColorFromPalette(palette, gHue + (i * 2),
				beat - gHue + (i * 10));
	}
}

void juggle() {
	// eight colored dots, weaving in and out of sync with each other
	fadeToBlackBy(leds, NUM_LEDS, 20);
	byte dothue = 0;
	for (int i = 0; i < 8; i++) {
		leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
		dothue += 32;
	}
}

// - Beat generators which return sine or sawtooth
// waves in a specified number of Beats Per Minute.
// Sine wave beat generators can specify a low and
// high range for the output.  Sawtooth wave beat
// generators always range 0-255 or 0-65535.
// beatsin8( BPM, low8, high8)
// = (sine(beatphase) * (high8-low8)) + low8
// beatsin16( BPM, low16, high16)
// = (sine(beatphase) * (high16-low16)) + low16
// beatsin88( BPM88, low16, high16)
// = (sine(beatphase) * (high16-low16)) + low16
// beat8( BPM)  = 8-bit repeating sawtooth wave
// beat16( BPM) = 16-bit repeating sawtooth wave
// beat88( BPM88) = 16-bit repeating sawtooth wave
// BPM is beats per minute in either simple form
// e.g. 120, or Q8.8 fixed-point form.
// BPM88 is beats per minute in ONLY Q8.8 fixed-point
// form.
void bl_blue_blueplus_blue_bl() {
CRGBPalette16 palette = bl_blue_blueplus_blue_bl_gp;
  uint8_t beat = beatsin8(bpm, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) { //9948
    forward_case(forward, beat, gHue, palette);
  }
}
void bl_red_redplus_red_bl() {
CRGBPalette16 palette = bl_red_redplus_red_bl_gp;
    uint8_t beat = beatsin8(bpm, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) { //9948
    forward_case(forward, beat, gHue, palette);
  }
}
void frankreich() {
	CRGBPalette16 palette = bhw3_41_gp;
	  uint8_t beat = beatsin8(bpm, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) { //9948
    forward_case(forward, beat, gHue, palette);
  }
}
void hellblau() {
	CRGBPalette16 palette = bhw4_009_gp;
	  uint8_t beat = beatsin8(bpm, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) { //9948
    forward_case(forward, beat, gHue, palette);
  }
}
void hellrot() {
	CRGBPalette16 palette = bhw3_55_gp;
	  uint8_t beat = beatsin8(bpm, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) { //9948
    forward_case(forward, beat, gHue, palette);
  }
}
void backAndForth() {
   CRGB red = CHSV(HUE_RED, 255, 255);
   redPalette = CRGBPalette16(red, red, red, red, red, red, red, red, red,
      red, red, red, red, red, red, red);
  CRGB blue = CHSV(HUE_BLUE, 255, 255);
  bluePalette = CRGBPalette16(blue, blue, blue, blue, blue, blue, blue,
      blue, blue, blue, blue, blue, blue, blue, blue, blue); 
	
	static uint8_t startindex = 0;
	uint8_t beat = beatsin8(bpm, 128, 255);

  forward = rand_forward(80,forward);
  
	if (forward == 0) {
		//startindex=forward_startindex(forward,startindex);
    CRGBPalette16 palette = bluePalette;
    for (int i = 0; i < NUM_LEDS; i++){
      forward_case(forward, beat, gHue, palette);
		}
	}

	else {
		//startindex=forward_startindex(forward,startindex);
    CRGBPalette16 palette = redPalette;
    for (int i = 0; i < NUM_LEDS; i++){
      forward_case(forward, beat, gHue, palette);
    }
	}
}
void phase(){
    CRGBPalette16 palette = phase_gp;
    static uint8_t startindex =0;
    startindex = forward_startindex(forward, startindex);
    fill_palette(leds, NUM_LEDS, startindex, (256 / NUM_LEDS) + 1,
      palette, 255, LINEARBLEND);
    forward = pingpong(forward,startindex);  
}
void gay_flag_1978(){
    CRGBPalette16 palette = gay_flag_1978_gp;
   static uint8_t startindex =0;
    startindex = forward_startindex(forward, startindex);
    fill_palette(leds, NUM_LEDS, startindex, (256 / NUM_LEDS) + 1,
      palette, 255, LINEARBLEND);
    forward = pingpong(forward,startindex);  
}
void gay_flag_1978_gHue(){
    CRGBPalette16 palette = gay_flag_1978_gp;
   uint8_t beat = beatsin8(bpm, 0, 255);
 for (int i = 0; i < NUM_LEDS; i++) { //9948
    forward_case(forward, beat, gHue, palette);
  }
  forward = pingpong(forward,gHue);    
}
void schwarzrotblau(){
    CRGBPalette16 palette = schwarzrotblau_gp;
    static uint8_t startindex =0;
    startindex = forward_startindex(forward, startindex);
    fill_palette(leds, NUM_LEDS, startindex, (256 / NUM_LEDS) + 1,
      palette, 255, LINEARBLEND);
    forward = pingpong(forward,startindex);  
}

void schwarzrotblautight(){
    CRGBPalette16 palette = schwarzrotblautight_gp;
    static uint8_t startindex =0;
    startindex = forward_startindex(forward, startindex);
    fill_palette(leds, NUM_LEDS, startindex, (256 / NUM_LEDS) + 1,
      palette, 255, LINEARBLEND);
    forward = pingpong(forward,startindex);   
}
void schwarzrotblautight_gHue(){
    CRGBPalette16 palette = schwarzrotblautight_gp;
    uint8_t beat = beatsin8(bpm, 0, 255);
  for (int i = 0; i < NUM_LEDS; i++) { //9948
    forward_case(forward, beat, gHue, palette);
  }
    forward = pingpong(forward,gHue);   
}

int forward_startindex(uint8_t val, uint8_t ind) {
	if (val == 1) {
		ind--;
	} else {
		ind++;
	}
	return ind;
}

int pingpong(uint8_t val, uint8_t ind){
  switch(ind){
    case 0:{
      forward = 1;
    }
    case 255:{
      forward = 0; 
    }
  }
//  if (gHue < 1) forward = 1;
//  else if(gHue > 254) forward =0;
  return forward;
}

int forward_gHue(uint8_t val, uint8_t gHue) {
	if (val == 1) {
		gHue++;
	} else {
		gHue--;
	}
	return gHue;
}

void forward_case(uint8_t val, uint8_t beat, uint8_t gHue,
		CRGBPalette16 palette) {
	for (int i = 0; i < NUM_LEDS; i++) {
		if (val == 1) {
			//9948

			leds[i] = ColorFromPalette(palette, gHue + (i * 2),
					beat - gHue + (i * 10));
		} else {
			switch (i) {
			case 0: {
				leds[3] = ColorFromPalette(palette, gHue + (3 * 2),
						beat - gHue + (3 * 10));
				break;
			}
			case 1: {
				leds[2] = ColorFromPalette(palette, gHue + (2 * 2),
						beat - gHue + (2 * 10));
				break;
			}
			case 2: {
				leds[1] = ColorFromPalette(palette, gHue + (1 * 2),
						beat - gHue + (1 * 10));
				break;
			}
			case 3: {
				leds[0] = ColorFromPalette(palette, gHue + (0 * 2),
						beat - gHue + (0 * 10));
				break;
			}
			}
		}
	}
}

//void forward_case(uint8_t val,uint8_t beat, uint8_t gHue,CRGBPalette16 palette){
//  for (int i = 0; i < NUM_LEDS; i++) 
//  {
//      leds[i] = ColorFromPalette(palette, gHue + (i * 2),
//          beat - gHue + (i * 10));
//  }
//}

int change_bri8(uint8_t val, uint8_t bri8, uint8_t brightdepth) {
	if (val == 1) {
		bri8 += (255 - brightdepth);
	} else {
		bri8 -= (255 - brightdepth);
	}
	return bri8;
}
int change_hue16(int val, int hue16, int hueinc16) {
	if (val == 1) {
		hue16 += hueinc16;
	} else {
		hue16 -= hueinc16;
	}
	return hue16;
}

int change_brightnesstheta16(int val, int brightnesstheta16,
		int brightnessthetainc16) {
	if (val == 1) {
		brightnesstheta16 += brightnessthetainc16;
	} else {
		brightnesstheta16 -= brightnessthetainc16;
	}
	return brightnesstheta16;
}
void change_nblend(int val, int i, CRGB newcolor, CRGB leds[], int func_val) {

	if (val == 1) {
		nblend(leds[i], newcolor, func_val);
	} else {
		switch (i) {
		case 0: {
			nblend(leds[3], newcolor, func_val);
			break;
		}
		case 1: {
			nblend(leds[2], newcolor, func_val);
			break;
		}
		case 2: {
			nblend(leds[1], newcolor, func_val);
			break;
		}
		case 3: {
			nblend(leds[0], newcolor, func_val);
			break;
		}
		}
	}

}

// reverse array from start to end--------------------------------------------------
void reverse(int a[], int start, int end) {
	int i;
	int temp;
	while (start++ < end--) {
		temp = a[start];
		a[start] = a[end];
		a[end] = temp;
	}
}

// function that will rotate array by d elements--------------------------------------
void rotateArray(int a[], int d, int n) {
	reverse(a, 0, d - 1);
	reverse(a, d, n - 1);
	reverse(a, 0, n - 1);
}

void uerdingen() {

	CRGBPalette16 palette = myRedBluePalette_p;
	uint8_t beat = beatsin8(bpm, 0, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}

void lakers() {
	// a colored dot sweeping back and forth, with fading trails
	CRGB purple = CHSV(HUE_PURPLE, 255, 255);
	CRGB yellow = CHSV(HUE_YELLOW, 255, 255);
	CRGB black = CRGB::Black;
	CRGBPalette16 palette;
	palette = CRGBPalette16(yellow, yellow, black, black, purple, purple, black,
			black, yellow, yellow, black, black, purple, purple, yellow, yellow);

	uint8_t beat = beatsin8(bpm, 0, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}

void rotweissblau() {
	palette = myRedWhiteBluePalette_p;
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}

void schwarzweiss() {
	// 'black out' all 16 palette entries...
	fill_solid(currentPalette, 16, CRGB::Black);
	// and set every fourth one to white.
	currentPalette[0] = CRGB::White;
	currentPalette[12] = CRGB::White;
	palette = currentPalette;
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}

}

// Pride2015 by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.

void pride() {
	static uint16_t sPseudotime = 0;
	static uint16_t sLastMillis = 0;
	static uint16_t sHue16 = 0;

	uint8_t sat8 = beatsin88(87, 220, 250);
	uint8_t brightdepth = beatsin88(341, 96, 224);
	uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
	uint8_t msmultiplier = beatsin88(147, 23, 60);

	uint16_t hue16 = sHue16; //gHue * 256;
	uint16_t hueinc16 = beatsin88(113, 1, 3000);

	uint16_t ms = millis();
	uint16_t deltams = ms - sLastMillis;
	sLastMillis = ms;
	sPseudotime += deltams * msmultiplier;
	sHue16 += deltams * beatsin88(400, 5, 9);
	uint16_t brightnesstheta16 = sPseudotime;

	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		//hue16 += hueinc16;
		hue16 = change_hue16(forward, hue16, hueinc16);
		uint8_t hue8 = hue16 / 256;

		//brightnesstheta16  += brightnessthetainc16;
		brightnesstheta16 = change_brightnesstheta16(forward, brightnesstheta16,
				brightnessthetainc16);
		uint16_t b16 = sin16(brightnesstheta16) + 32768;

		uint16_t bri16 = (uint32_t)((uint32_t) b16 * (uint32_t) b16) / 65536;
		uint8_t bri8 = (uint32_t)(((uint32_t) bri16) * brightdepth) / 65536;
		//bri8 += (255 - brightdepth);
		bri8 = change_bri8(forward, bri8, brightdepth);

		CRGB newcolor = CHSV(hue8, sat8, bri8);

		//nblend(leds[i], newcolor, 64);
		change_nblend(forward, i, newcolor, leds, 64);
	}
}

// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void colorwaves() {
	static uint16_t sPseudotime = 0;
	static uint16_t sLastMillis = 0;
	static uint16_t sHue16 = 0;

	// uint8_t sat8 = beatsin88( 87, 220, 250);
	uint8_t brightdepth = beatsin88(341, 96, 224);
	uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
	uint8_t msmultiplier = beatsin88(147, 23, 60);

	uint16_t hue16 = sHue16; //gHue * 256;
	uint16_t hueinc16 = beatsin88(113, 300, 1500);

	uint16_t ms = millis();
	uint16_t deltams = ms - sLastMillis;
	sLastMillis = ms;
	sPseudotime += deltams * msmultiplier;
	sHue16 += deltams * beatsin88(400, 5, 9);
	uint16_t brightnesstheta16 = sPseudotime;

	for (uint16_t i = 0; i < NUM_LEDS; i++) {
		//hue16 += hueinc16;
		hue16 = change_hue16(forward, hue16, hueinc16); //edited AJ
		uint8_t hue8 = hue16 / 256;
		uint16_t h16_128 = hue16 >> 7;
		if (h16_128 & 0x100) {
			hue8 = 255 - (h16_128 >> 1);
		} else {
			hue8 = h16_128 >> 1;
		}

		//brightnesstheta16  += brightnessthetainc16;
		brightnesstheta16 = change_brightnesstheta16(forward, brightnesstheta16,    //edited AJ
				brightnessthetainc16);
		uint16_t b16 = sin16(brightnesstheta16) + 32768;

		uint16_t bri16 = (uint32_t)((uint32_t) b16 * (uint32_t) b16) / 65536;
		uint8_t bri8 = (uint32_t)(((uint32_t) bri16) * brightdepth) / 65536;
		//bri8 += (255 - brightdepth);
		bri8 = change_bri8(forward, bri8, brightdepth);

		uint8_t index = hue8;
		//index = triwave8( index);
		index = scale8(index, 240);

		CRGB newcolor = ColorFromPalette(gCurrentPalette, index, bri8);

		//nblend(leds[i], newcolor, 128);
		change_nblend(forward, i, newcolor, leds, 128);
	}
}

// Alternate rendering function just scrolls the current palette
// across the defined LED strip.
void palettetest() {
	static uint8_t startindex = 0;
	startindex = forward_startindex(forward, startindex);
	fill_palette(leds, NUM_LEDS, startindex, (256 / NUM_LEDS) + 1,
			gCurrentPalette, 255, LINEARBLEND);
}

void one_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = Sunset_Real_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void two_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = es_rivendell_15_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void three_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = es_ocean_breeze_036_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void four_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = rgi_15_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void five_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = retro2_16_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void six_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = Analogous_1_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void seven_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = es_pinksplash_08_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void eight_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = Coral_reef_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void nine_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = es_ocean_breeze_068_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void ten_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = es_pinksplash_07_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void eleven_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = es_vintage_01_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twelve_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = departure_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void thirteen_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = es_landscape_64_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void fourteen_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = es_landscape_33_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void fifteen_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = rainbowsherbet_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void sixteen_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = gr65_hult_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void seventeen_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = gr64_hult_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void eighteen_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = GMT_drywet_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twenty_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = ib_jul01_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twentyone_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = es_vintage_57_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twentytwo_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = ib15_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twentythree_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = Fuschia_7_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twentyfour_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = es_emerald_dragon_08_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twentyfive_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = lava_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twentysix_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = fire_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twentyseven_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = Colorfull_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twentyeight_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = Magenta_Evening_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void twentynine_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = Pink_Purple_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void thirty_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = Sunset_Real_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}

void thirtyone_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = BlacK_Blue_Magenta_White_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void thirtytwo_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = BlacK_Magenta_Red_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void thirtythree_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = BlacK_Red_Magenta_Yellow_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}
void thirtyfour_gp() {
	// colored stripes pulsing at a defined Beats-Per-Minute (BPM)

	CRGBPalette16 palette = Blue_Cyan_Yellow_gp;
	uint8_t beat = beatsin8(bpm, 64, 255);
	for (int i = 0; i < NUM_LEDS; i++) { //9948
		forward_case(forward, beat, gHue, palette);
	}
}

const TProgmemPalette16 myRedBluePalette_p = { CRGB::Red, CRGB::Blue, // 'white' is too bright compared to red and blue
		CRGB::Red, CRGB::Blue,

		CRGB::Red, CRGB::Blue, CRGB::Red, CRGB::Blue,

		CRGB::Red, CRGB::Blue, CRGB::Red, CRGB::Blue,

		CRGB::Red, CRGB::Blue, CRGB::Red, CRGB::Blue

};

const TProgmemPalette16 myBlueRedPalette_p = {

		CRGB::Blue, // 'white' is too bright compared to red and blue
		CRGB::Red, CRGB::Blue, CRGB::Red, CRGB::Blue, CRGB::Red, CRGB::Blue,
		CRGB::Red, CRGB::Blue, CRGB::Red, CRGB::Blue, CRGB::Red, CRGB::Blue,
		CRGB::Red, CRGB::Blue, CRGB::Red

};
const TProgmemPalette16 myRedWhiteBluePalette_p = { CRGB::Red, CRGB::Gray, // 'white' is too bright compared to red and blue
		CRGB::Blue, CRGB::Black,

		CRGB::Red, CRGB::Gray, CRGB::Blue, CRGB::Black,

		CRGB::Red, CRGB::Gray, CRGB::Blue, CRGB::Black,

		CRGB::Red, CRGB::Gray, CRGB::Blue, CRGB::Black, };
