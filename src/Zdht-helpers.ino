#ifdef DHT_ACTIVE
void loopZsensorDHT() {
        if (millis() - DHTlastSent >= DHT_INTERVAL * 1000UL || DHTlastSent == 0) {
                //Homie.getLogger() << " - DTH loop:" << endl;
                float humidity = dht.readHumidity();
                float temperature = dht.readTemperature();
                DHTlastSent = millis();
                if (temperature > 0 && humidity > 0) {
                        dhtTemp.setProperty("value").send(String(temperature));
                        dhtHum.setProperty("value").send(String(humidity));
                        Homie.getLogger() << " -- temp: " << temperature << " humidity: " << humidity << endl;
                } else {
                        Homie.getLogger() << " -- DTH sensor failed" << endl;
                }
        }
}
#endif
