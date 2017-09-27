#ifdef BME280_ACTIVE
void setupZsensorBME280()
{
        mySensor.settings.commInterface = I2C_MODE;
        mySensor.settings.I2CAddress = 0x76; // Bosch BME280 I2C Address
//***Operation settings*****************************//
// runMode Setting - Values:
// -------------------------
//  0, Sleep mode
//  1 or 2, Forced mode
//  3, Normal mode
        mySensor.settings.runMode = 3; //Normal mode
// tStandby Setting - Values:
// --------------------------
//  0, 0.5ms
//  1, 62.5ms
//  2, 125ms
//  3, 250ms
//  4, 500ms
//  5, 1000ms
//  6, 10ms
//  7, 20ms
        mySensor.settings.tStandby = 0;
// Filter can be off or number of FIR coefficients - Values:
// ---------------------------------------------------------
//  0, filter off
//  1, coefficients = 2
//  2, coefficients = 4
//  3, coefficients = 8
//  4, coefficients = 16
        mySensor.settings.filter = 1;
// tempOverSample - Values:
// ------------------------
//  0, skipped
//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
        mySensor.settings.tempOverSample = 1;
// pressOverSample - Values:
// -------------------------
//  0, skipped
//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
        mySensor.settings.pressOverSample = 1;
// humidOverSample - Values:
// -------------------------
//  0, skipped
//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
        mySensor.settings.humidOverSample = 1;
        delay(10); // Gives the Sensor enough time to turn on (The BME280 requires 2ms to start up)
        Homie.getLogger() << "Bosch BME280 Initialized - Result of .begin(): 0x" << mySensor.begin() << endl;
}

void loopZsensorBME280() {
        if (millis() > (timebme280 + BME280_INTERVAL * 1000)) {
                timebme280 = millis();
                static float persisted_bme_tempc;
                static float persisted_bme_hum;
                static float persisted_bme_pa;
                static float persisted_bme_altim;

                float BmeTempC = mySensor.readTempC();
                float BmeHum = mySensor.readFloatHumidity();
                float BmePa = mySensor.readFloatPressure();
                float BmeAltiM = mySensor.readFloatAltitudeMeters();

                // Check if reads failed and exit early (to try again).
                if (isnan(BmeTempC) || isnan(BmeHum) || isnan(BmePa) || isnan(BmeAltiM)) {
                        Homie.getLogger() << "Failed to read from Weather Sensor BME280!" << endl;
                }else{
                        // Generate Temperature in degrees C
                        if(BmeTempC != persisted_bme_tempc || BME280_ALWAYS) {
                                char bmetempc[7];
                                dtostrf(BmeTempC,4,2,bmetempc);
                                bme280Temp.setProperty("value").send(String(bmetempc));
                        }else{
                                Homie.getLogger() << "Same Degrees C don't send it" << endl;
                        }
                        // Generate Humidity in percent
                        if(BmeHum != persisted_bme_hum || BME280_ALWAYS) {
                                char bmehum[7];
                                dtostrf(BmeHum,4,2,bmehum);
                                bme280Hum.setProperty("value").send(String(bmehum));
                        }else{
                                Homie.getLogger() << "Same Humidity don't send it" << endl;
                        }

                        // Generate Pressure in Pa
                        if(BmePa != persisted_bme_pa || BME280_ALWAYS) {
                                char bmepa[7];
                                dtostrf(BmePa,4,2,bmepa);
                                bme280Press.setProperty("value").send(String(bmepa));
                        }else{
                                Homie.getLogger() << "Same Pressure don't send it" << endl;
                        }

                        // Generate Altitude in Meter
                        if(BmeAltiM != persisted_bme_altim || BME280_ALWAYS) {
                                char bmealtim[7];
                                dtostrf(BmeAltiM,4,2,bmealtim);
                                bme280Alt.setProperty("value").send(String(bmealtim));
                        }else{
                                Homie.getLogger() << "Same Altitude Meter don't send it" << endl;
                        }
                }
                persisted_bme_tempc = BmeTempC;
                persisted_bme_hum = BmeHum;
                persisted_bme_pa = BmePa;
                persisted_bme_altim = BmeAltiM;
                Homie.getLogger() << " -- temp: " << persisted_bme_tempc << " humidity: " << persisted_bme_hum << " altitude: " << persisted_bme_altim << " pressure: " << persisted_bme_pa << endl;
        }
}
#endif
