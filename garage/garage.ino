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

#define CLIENT_ID "garage"

CoogleIOT *iot;
PubSubClient *mqtt;

class Action {
  public:
    int pin;
    char* topic;
};


Action act1 = {pin: D1, topic: "garage/ACTION"};
const int actionsCount = 1;
Action actions[actionsCount] {act1};

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

    mqtt->subscribe(actions[0].topic);

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

  iot->info("Handling Action Request");
  iot->flashStatus(200, 1);
  if (strcmp(payloadStr, "open") == 0) {
    digitalWrite( actions[0].pin, LOW);
    delay(500);
    digitalWrite( actions[0].pin, HIGH);
  }
}
