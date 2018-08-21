#include <CoogleIOT.h>

#include <Arduino.h>
#include <SPI.h>

#include "Adafruit_ILI9341esp.h"
#include <Adafruit_GFX.h>
#include <XPT2046.h>

// Modify the following two lines to match your hardware
// Also, update calibration parameters below, as necessary

// For the esp shield, these are the default.
#define TFT_DC 2
#define TFT_CS 15

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046 touch(/*cs=*/ 4, /*irq=*/ 5);

#define SERIAL_BAUD 115200

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3
#define D10 1

#define CLIENT_ID "panel"

#undef COOGLEIOT_TIMEZONE_OFFSET
#define COOGLEIOT_TIMEZONE_OFFSET ((3600 * 2) * 1) // Default Timezone is -5 UTC (America/New York)

CoogleIOT *iot;
PubSubClient *mqtt;


#define BUTTON_X 40
#define BUTTON_Y 100
#define BUTTON_W 65
#define BUTTON_H 35
#define BUTTON_SPACING_X 15
#define BUTTON_SPACING_Y 15
#define BUTTON_TEXTSIZE 2

class Button {
  public:
    uint16_t color;
    char* label;
    char* m[6];
    char* t;
    int pressCount;
    Adafruit_GFX_Button button;
};


Button b1 = {color: ILI9341_DARKGREEN, label: "Font", m: {"on","off"}, t: "fountain/ACTION"};
Button b2 = {color: ILI9341_DARKGREEN, label: "Gate", m: {"open", "close"}, t: "gate/ACTION"};
Button b3 = {color: ILI9341_DARKGREEN, label: "Gara", m: {"on","off"},  t: "garage/ACTION"};
Button b4 = {color: ILI9341_DARKGREEN, label: "Z1", m: {"on","off"}, t: "yard/1/ACTION"};
Button b5 = {color: ILI9341_DARKGREEN, label: "Z2", m: {"on","off"}, t: "yard/2/ACTION"};
Button b6 = {color: ILI9341_DARKGREEN, label: "Z3", m: {"on","off"}, t: "yard/3/ACTION"};
const int buttonsCount = 6;
Button buttons[buttonsCount] {b1, b2, b3, b4, b5, b6};

void setup()
{
  SPI.setFrequency(ESP_SPI_FREQ);

  tft.begin();
  touch.begin(tft.width(), tft.height());  // Must be done before setting rotation
  tft.fillScreen(ILI9341_BLACK);
  // Replace these for your screen module
  touch.setCalibration(209, 1759, 1775, 273);


  for (int n = 0; n < buttonsCount; n++) {
    int row = n % 3;
    int col = n / 3;
    int x = BUTTON_X + row * (BUTTON_W + BUTTON_SPACING_X);
    int y = BUTTON_Y + col * (BUTTON_H + BUTTON_SPACING_Y);
    buttons[n].button.initButton(&tft, x, y, BUTTON_W, BUTTON_H, ILI9341_WHITE,  buttons[n].color, ILI9341_WHITE, buttons[n].label, BUTTON_TEXTSIZE);
    buttons[n].button.drawButton();
  }

  iot = new CoogleIOT(LED_BUILTIN);

  iot->enableSerial(SERIAL_BAUD)
  .setMQTTClientId(CLIENT_ID)
  .initialize();

  if (iot->mqttActive()) {
    mqtt = iot->getMQTTClient();

    mqtt->setCallback(mqttCallbackHandler);

    mqtt->subscribe("#");

    iot->info("Initialized");

  } else {
    iot->error("MQTT Not initialized, Inactive");
  }
  ///

}

void loop()
{
  iot->loop();
  screen();
  tft.setCursor(45, 30);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  tft.setTextSize(3);
  tft.println(iot->getTimeAsString());
}

void screen() {
  uint16_t x = NULL;
  uint16_t y = NULL;
  if (touch.isTouching()) {
    touch.getPosition(x, y);
  }

  Serial.print(x); Serial.print(":");Serial.println(y);

  // go thru all the buttons, checking if they were pressed
  for (uint8_t b = 0; b < buttonsCount; b++) {
    buttons[b].button.press(buttons[b].button.contains(x, y));  // tell the button it is pressed
  }

  // now we can ask the buttons if their state has changed
  for (uint8_t b = 0; b < buttonsCount; b++) {
    if (buttons[b].button.justReleased()) {
      buttons[b].pressCount++;
      buttons[b].button.drawButton(false);  // draw normal
      mqtt->publish(buttons[b].t, buttons[b].m[buttons[b].pressCount%2]);
    }

    if (buttons[b].button.justPressed()) {
      buttons[b].button.drawButton(true);  // draw invert!
    }
  }


  delay(20);
}

void mqttCallbackHandler(char *topic, byte * payload, unsigned int length)
{
  String action;
  char *payloadStr = (char *) payload;
  payloadStr[length] = '\0';

  iot->info("Handling Action Request");
  iot->flashStatus(200, 1);

//  tft.setCursor(0, 0);
//  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  tft.setTextSize(1);
//  tft.println(topic);
//  tft.setCursor(100, 0);
//  tft.println(payloadStr);
}
