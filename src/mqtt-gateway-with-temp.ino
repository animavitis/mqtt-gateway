#include <Homie.h>
#include "Zbme280-config.h" // bme280 Configuration
#include "Zir-config.h" // bme280 Configuration
#include "Zdht-config.h" // DHT Configuration
#include "Zrf-config.h" //rf Configuration
#include "Zalarm-config.h" // alarm Configuration
#ifdef DHT_ACTIVE
#include <DHT.h>
#endif
#ifdef IR_ACTIVE
#include <IRremoteESP8266.h>
#include <IRsend.h>  // Needed if you want to send IR commands.
#include <IRrecv.h>  // Needed if you want to receive IR commands.
#endif
#ifdef BME280_ACTIVE
#include "Wire.h"
#include "SPI.h"
#include "SparkFunBME280.h"
#endif
#ifdef RF_ACTIVE
#include <RCSwitch.h>
#endif
int arrayMQTT[6] = {0,0,0,0,0,0};
String ReceivedSignal[5][3] ={{"N/A", "N/A", "N/A"},{"N/A", "N/A", "N/A"},{"N/A", "N/A", "N/A"},{"N/A", "N/A", "N/A"},{"N/A", "N/A", "N/A"}};
String alarmState = "N/A";
#ifdef ALARM_ACTIVE
const String alarmStates[5] = {"disarmed","armed_home","armed_away","pending","triggered"};
String alarmStateOld = alarmStates[0];
String alarmStateTarget = alarmStates[0];
long lastArmedHomeTime = 0;
long lastPendingTime = 0;
long lastArmedAwayTime = 0;
long lastDisarmedTime = 0;
long lastTriggeredTime = 0;
long pendingCounter = 0;
bool pendingStatusSent = true;
long initialAlarmState = 0;
long initialAlarmStateTime = 0;
long arrayHome[10] = {0,0,0,0,0,0,0,0,0,0};
long arrayAway[10] = {0,0,0,0,0,0,0,0,0,0};
#endif

AsyncMqttClient& mqttClient = Homie.getMqttClient();
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
        Homie.getLogger() << " - Recieving toMQTT topic:" << endl;
        Homie.getLogger() << " -- topic: " << topic << " payload: " << payload << endl;
        long duplicteMQTT = 0;
        storeValue(payload);
}
void onHomieEvent(const HomieEvent& event) {
        switch(event.type) {
        case HomieEventType::MQTT_READY:
                uint16_t packetIdSub = mqttClient.subscribe("anima/+/toMQTT/rf", 0);
                Homie.getLogger() << " - Subscribing, packetId: " << packetIdSub;
                break;
        }
}
#ifdef RF_ACTIVE
RCSwitch mySwitch = RCSwitch();
#endif
#ifdef IR_ACTIVE
IRrecv irrecv(IR_RECEIVER_PIN);
IRsend irsend(IR_EMITTER_PIN);
decode_results results;
#endif
#ifdef DHT_ACTIVE
DHT dht(DHT_PIN, DHT_TYPE, 11);
#endif
#ifdef BME280_ACTIVE
BME280 mySensor;
#endif
// homie nodes & settings
#ifdef ALARM_ACTIVE
HomieNode alarmNode("alarm", "switch");
#endif
#ifdef IR_ACTIVE
HomieNode irSwitchNode("toIR", "switch");
#endif
#ifdef RF_ACTIVE
HomieNode rfSwitchNode("toRF", "switch");
HomieNode receiverNode("toMQTT", "switch");
#endif
#ifdef BME280_ACTIVE
HomieNode bme280Temp("temperature", "temperature");
HomieNode bme280Hum("humidity", "humidity");
HomieNode bme280Press("pressure", "pressure");
HomieNode bme280Alt("altitude", "altitude");
#endif
#ifdef DHT_ACTIVE
HomieNode dhtTemp("temperature", "temperature");
HomieNode dhtHum("humidity", "humidity");
#endif
#ifdef ALARM_ACTIVE
HomieSetting<const char*> sensorArrayAwaySetting("arrayAway", "list of sensor for arm away");
HomieSetting<const char*> sensorArrayHomeSetting("arrayHome", "list of sensor for arm home");
#endif
void setupHandler() {
        #ifdef DHT_ACTIVE
        dhtTemp.setProperty("unit").send("C");
        dhtHum.setProperty("unit").send("%");
        #endif
        #ifdef BME280_ACTIVE
        bme280Temp.setProperty("unit").send("C");
        bme280Hum.setProperty("unit").send("%");
        bme280Press.setProperty("unit").send("Pa");
        bme280Alt.setProperty("unit").send("M");
        #endif
        #ifdef ALARM_ACTIVE
        readAlarmStateFromSpiffs();
        getSensorArrayAway();
        getSensorArrayHome();
        #endif
}
void loopHandler() {
        #ifdef ALARM_ACTIVE
        loopZalarm();
        #endif
        #ifdef DHT_ACTIVE
        loopZsensorDHT();
        #endif
        #ifdef BME280_ACTIVE
        loopZsensorBME280();
        #endif
        #ifdef RF_ACTIVE
        loopZrfToMqtt();
        #endif
        #ifdef IR_ACTIVE
        loopZirToMqtt();
        #endif
        delay(50);
}
void setup() {
        Serial.begin(115200);
        #ifdef ALARM_ACTIVE
        setAlarmTimes();
        #endif
        #ifdef BME280_ACTIVE
        setupZsensorBME280();
        #endif
        #ifdef IR_ACTIVE
        irsend.begin();
        irrecv.enableIRIn();
        #endif
        #ifdef RF_ACTIVE
        mySwitch.enableTransmit(RF_EMITTER_PIN); // RF Transmitter
        mySwitch.setRepeatTransmit(RF_EMITTER_REPEAT); //increase transmit repeat to avoid lost of rf sendings
        mySwitch.enableReceive(RF_RECEIVER_PIN); // Receiver on pin D3
        #endif
        // init Homie
        Homie_setFirmware("mqtt-gateway", "1.0.0");
        Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
        Homie.disableResetTrigger();
        //Homie.disableLogging();
        //Homie.disableLedFeedback();
        #ifdef ALARM_ACTIVE
        alarmNode.advertise("state").settable(alarmSwitchOnHandler);

        #endif
        #ifdef IR_ACTIVE
        receiverNode.advertise("ir");
        irSwitchNode.advertise("code").settable(irSwitchOnHandler);
        #endif
        #ifdef RF_ACTIVE
        receiverNode.advertise("rf");
        rfSwitchNode.advertise("on").settable(rfSwitchOnHandler);
        rfSwitchNode.advertise("off").settable(rfSwitchOnHandler);
        #endif
        #ifdef BME280_ACTIVE
        bme280Temp.advertise("unit");
        bme280Temp.advertise("value");
        bme280Press.advertise("unit");
        bme280Press.advertise("value");
        bme280Hum.advertise("unit");
        bme280Hum.advertise("value");
        bme280Alt.advertise("unit");
        bme280Alt.advertise("value");
        #endif
        #ifdef DHT_ACTIVE
        dhtTemp.advertise("unit");
        dhtTemp.advertise("value");
        dhtHum.advertise("unit");
        dhtHum.advertise("value");
        #endif
        Homie.onEvent(onHomieEvent);
        mqttClient.onMessage(onMqttMessage);
        Homie_setBrand("anima");
        Homie.setup();
}
void loop() {
        Homie.loop();
        if (Homie.isConnected()) {
                // The device is connected
        } else {
                // The device is not connected
                #ifdef ALARM_ACTIVE
                loopZalarm();
                #endif
                #ifdef RF_ACTIVE
                loopZrfToMqtt();
                #endif
                #ifdef IR_ACTIVE
                loopZirToMqtt();
                #endif
                delay(50);
        }

}
