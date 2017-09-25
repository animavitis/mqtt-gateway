#define BME280_ACTIVE                   // comment to disable  (dht & bme280 cant be enabled at same time)
#define BME280_ALWAYS true // if false when the current value is the same as previous one don't send it by MQTT
#define BME280_INTERVAL 60
unsigned long timebme280 = 0;
