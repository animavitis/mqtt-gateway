#include <Homie.h>
#include "config-alarm.h" // alarm Configuration
#include "config-bme280.h" // bme280 Configuration
#include "config-dht.h" // DHT Configuration
#include "config-ir.h" // bme280 Configuration
#include "config-rf.h" //rf Configuration
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
// #ifdef ALARM_ACTIVE
//
// #endif

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
                uint16_t packetIdSub = mqttClient.subscribe("anima/+/rf/toMQTT", 0);
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
HomieNode irSwitchNode("ir", "switch");
#endif
#ifdef RF_ACTIVE
HomieNode rfSwitchNode("rf", "switch");
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

HomieSetting<const char*> alarmButtonSetting("alarm buttons", "arm home, arm away, disarm, trigger buttons as JSON (example in config.json)");
HomieSetting<const char*> alarmArraySetting("alarm Arrays", "home, away & siren sensor as JSON (example in config.json)");

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
}
void loopHandler() {
        #ifdef ALARM_ACTIVE
        loopAlarm();
        #endif
        #ifdef DHT_ACTIVE
        loopSensorDHT();
        #endif
        #ifdef BME280_ACTIVE
        loopSensorBME280();
        #endif
        #ifdef RF_ACTIVE
        loopRfToMqtt();
        #endif
        #ifdef IR_ACTIVE
        loopIrToMqtt();
        #endif
        delay(50);
}
void setup() {
        Serial.begin(115200);
        #ifdef BME280_ACTIVE
        setupSensorBME280();
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
        irSwitchNode.advertise("toMQTT");
        irSwitchNode.advertise("code").settable(irSwitchOnHandler);
        #endif
        #ifdef RF_ACTIVE
        rfSwitchNode.advertise("toMQTT");
        rfSwitchNode.advertise("code").settable(rfSwitchOnHandler);
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
        #ifdef ALARM_ACTIVE
        readAlarmStateFromSpiffs();
        getAlarmArrays();
        getAlarmButtons();
        setAlarmTimes();
        #endif
}
void loop() {
        Homie.loop();
        if (Homie.isConnected()) {
                // The device is connected
        } else {
                // The device is not connected
                #ifdef ALARM_ACTIVE
                loopAlarm();
                #endif
                #ifdef RF_ACTIVE
                loopRfToMqtt();
                #endif
                #ifdef IR_ACTIVE
                loopIrToMqtt();
                #endif
                delay(50);
        }
}
