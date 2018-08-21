#include <CoogleIOT.h>
#include <DHT.h>

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

#define CLIENT_ID "yard"

CoogleIOT *iot;
PubSubClient *mqtt;
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// DHT Sensor
const int DHTPin = D5;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

const int RainPin = D6;

os_timer_t myTimer;

bool timerTick = true;

class Action {
  public:
    int pin;
    char* topic;
};


Action act1 = {pin: D1, topic: "yard/1/ACTION"};
Action act2 = {pin: D2, topic: "yard/2/ACTION"};
Action act3 = {pin: D3, topic: "yard/3/ACTION"};
Action act4 = {pin: D4, topic: "yard/light/ACTION"};
const int actionsCount = 4;
Action actions[actionsCount] {act1, act2, act3, act4};

void setup()
{
  iot = new CoogleIOT(LED_BUILTIN);

  iot->enableSerial(SERIAL_BAUD)
  .setMQTTClientId(CLIENT_ID)
  .initialize();

  os_timer_setfn(&myTimer, timerCallback, NULL);

  os_timer_arm(&myTimer, 30 * 1000, true);

  for (int n = 0; n < actionsCount; n++) {
    pinMode(actions[n].pin, OUTPUT);
    digitalWrite(actions[n].pin, HIGH);
  }

  pinMode(RainPin, INPUT);

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

void timerCallback(void *pArg) {
  timerTick = true;
}

void loop()
{
  iot->loop();

  if (timerTick) {
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
    }

    mqtt->publish("yard/temp", String(t).c_str());
    mqtt->publish("yard/humidity", String(h).c_str());
    mqtt->publish("yard/rain", String(!digitalRead(RainPin)).c_str());
    timerTick = false;
  }
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
