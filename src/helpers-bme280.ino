#ifdef BME280_ACTIVE
void setupSensorBME280()
{
        mySensor.settings.commInterface = I2C_MODE;
        mySensor.settings.I2CAddress = 0x76; // Bosch BME280 I2C Address
// runMode Setting - Values: //  0, Sleep mode //  1 or 2, Forced mode //  3, Normal mode
        mySensor.settings.runMode = 3;
// tStandby Setting - Values: //  0, 0.5ms //  1, 62.5ms //  2, 125ms //  3, 250ms //  4, 500ms //  5, 1000ms //  6, 10ms //  7, 20ms
        mySensor.settings.tStandby = 0;
// Filter can be off or number of FIR coefficients - Values: //  0, filter off //  1, coefficients = 2 //  2, coefficients = 4 //  3, coefficients = 8 //  4, coefficients = 16
        mySensor.settings.filter = 1;
// tempOverSample - Values: //  0, skipped //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
        mySensor.settings.tempOverSample = 1;
// pressOverSample - Values: //  0, skipped //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
        mySensor.settings.pressOverSample = 1;
// humidOverSample - Values //  0, skipped //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
        mySensor.settings.humidOverSample = 1;
        delay(10); // Gives the Sensor enough time to turn on (The BME280 requires 2ms to start up)
        Homie.getLogger() << "Bosch BME280 Initialized - Result of .begin(): 0x" << mySensor.begin() << endl;
}
void loopSensorBME280() {
        if (millis() > (timebme280 + BME280_INTERVAL * 1000)) {
                //Homie.getLogger() << " - BME280 loop:" << endl;
                timebme280 = millis();
                float BmeTempC = mySensor.readTempC();
                float BmeHum = mySensor.readFloatHumidity();
                float BmePa = mySensor.readFloatPressure();
                float BmeAltiM = mySensor.readFloatAltitudeMeters();
                bme280Temp.setProperty("value").send(String(BmeTempC));
                bme280Hum.setProperty("value").send(String(BmeHum));
                bme280Press.setProperty("value").send(String(BmePa));
                bme280Alt.setProperty("value").send(String(BmeAltiM));
                Homie.getLogger() << " -- temp: " << String(BmeTempC) << " humidity: " << String(BmeHum) << " altitude: " << String(BmeAltiM) << " pressure: " << String(BmePa) << endl;
        }
}
#endif
