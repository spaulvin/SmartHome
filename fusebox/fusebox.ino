#include <CoogleIOT.h>

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

#define CLIENT_ID "fusebox"

CoogleIOT *iot;
PubSubClient *mqtt;

class Action {
  public:
    int pin;
    char* topic;
};


Action act1 = {pin: D1, topic: "fusebox/1/ACTION"};
Action act2 = {pin: D2, topic: "fusebox/2/ACTION"};
Action act3 = {pin: D3, topic: "fusebox/3/ACTION"};
Action act4 = {pin: D4, topic: "fusebox/4/ACTION"};
const int actionsCount = 4;
Action actions[actionsCount] {act1, act2, act3, act4};

void setup()
{
  iot = new CoogleIOT(LED_BUILTIN);

  iot->enableSerial(SERIAL_BAUD)
  .setMQTTClientId(CLIENT_ID)
  .initialize();

  for (int n = 0; n < actionsCount; n++) {
    pinMode(actions[n].pin, OUTPUT);
    digitalWrite(actions[n].pin, HIGH);
  }

  if (iot->mqttActive()) {
    mqtt = iot->getMQTTClient();

    mqtt->setCallback(mqttCallbackHandler);

    for (int n = 0; n < actionsCount; n++) {
      mqtt->subscribe(actions[n].topic);
    }

    iot->info("Initialized");

  } else {
    iot->error("MQTT Not initialized, Inactive");
  }
}

void loop()
{
  iot->loop();
}

void mqttCallbackHandler(char *topic, byte *payload, unsigned int length)
{
  String action;
  char *payloadStr = (char *) payload;
  payloadStr[length] = '\0';

  for (int n = 0; n < actionsCount; n++) {
    if (strcmp(topic, actions[n].topic) == 0) {
      iot->info("Handling Action Request");
      iot->flashStatus(200, 1);
      if (strcmp(payloadStr, "on") == 0) {
        digitalWrite( actions[n].pin, LOW);
      } else if (strcmp(payloadStr, "off") == 0) {
        digitalWrite(actions[n].pin, HIGH);
      }
    }
  }
}
