// ReceivedSignal store
void storeValue(String currentCode){
        long now = millis();
        int o = getMin();
        Homie.getLogger() << "✔ storeValue("<<  currentCode << "):" << endl;
        ReceivedSignal[o][0] = currentCode;
        ReceivedSignal[o][1] = now;
        ReceivedSignal[o][2] = alarmState;
        Homie.getLogger() << "  No.: value / timestamp / alarmstate" << endl;
        for (int i = 0; i < 8; i++)
        {
                Homie.getLogger() << "  " << String(i+1) << ": " << ReceivedSignal[i][0] << " / " << String(ReceivedSignal[i][1]) << " / " << String(ReceivedSignal[i][2]) << endl;
        }
        #ifdef ALARM_ACTIVE
        long currentCodeLong = currentCode.toInt();
        homeCheck(currentCodeLong);
        awayCheck(currentCodeLong);
        buttonsCheck(currentCodeLong);
        #endif
}
// ReceivedSignal helpers
int getMin(){
        int minimum = 0;
        minimum = ReceivedSignal[0][1].toInt();
        int minindex = 0;
        for (int i = 0; i < 8; i++)
        {
                if (ReceivedSignal[i][1].toInt() < minimum) {
                        minimum = ReceivedSignal[i][1].toInt();
                        minindex = i;
                }
        }
        return minindex;
}
//433 & IR duplicate check
boolean isAduplicate(String value){
        Homie.getLogger();
        for (int i = 0; i < 8; i++) {
                if (ReceivedSignal[i][0] == value) {
                        long now = millis();
                        if (now - ReceivedSignal[i][1].toInt() < TIME_AVOID_DUPLICATE * 1000UL) { // change
                                Homie.getLogger() << "✖ isAduplicate("<<  value << "): Duplicate found, don't send" << endl;
                                return true;
                        }
                }
        }
        Homie.getLogger() << "✔ isAduplicate("<<  value << "): not dulicated" << endl;
        return false;
}
//Get data from MQTT
void getArrayMQTT(String value) {
        int value_len = value.length() + 1;
        char char_array[value_len];
        value.toCharArray(char_array, value_len);
        int ipos = 0;
        char *tok = strtok(char_array, ",");
        while (tok) {
                if (ipos < 3) {
                        arrayMQTT[ipos++] = atoi(tok);
                }
                tok = strtok(NULL, ",");
        }
}
//loop MQTTtoRF
bool rfSwitchOnHandler(const HomieRange& range, const String& value) {
        arrayMQTT[0] = 0;     //data or address
        arrayMQTT[1] = 350;   //pulseLength
        arrayMQTT[2] = 1;     //protocol
        getArrayMQTT(value);
        mySwitch.setPulseLength(arrayMQTT[1]);
        mySwitch.setProtocol(arrayMQTT[2]);
        if (arrayMQTT[0] > 0)  {
                if(arrayMQTT[2] != 7) {
                        mySwitch.send(arrayMQTT[0], 24);
                }else{
                        mySwitch.send(arrayMQTT[0], 32);
                }
                delay((TIME_AVOID_DUPLICATE - 1) * 1000UL);
        }
        boolean result = rfSwitchNode.setProperty("code").send(String(arrayMQTT[0]));
        if (result) Homie.getLogger() << "✔ rfSwitchOnHandler(range," << value << "): 433Mhz pulseLength: " << arrayMQTT[1] << " protocol: "<< arrayMQTT[2] <<" value: " << arrayMQTT[0] <<  " sent"<< endl;
        return true;
}
//loop RFtoMQTT
void loopRfToMqtt(){
        if (mySwitch.available()) {
                long data = mySwitch.getReceivedValue();
                mySwitch.resetAvailable();
                String currentCode = String(data);
                if (!isAduplicate(currentCode) && currentCode!=0) {
                        Homie.getLogger() << "✔ Receiving 433Mhz value: " << mySwitch.getReceivedValue() << " bitLenght: " << mySwitch.getReceivedBitlength() << " protocol: " << mySwitch.getReceivedProtocol() << " delay: " << mySwitch.getReceivedDelay() << endl;
                        rfSwitchNode.setProperty("toMQTT").send(currentCode);
                        storeValue(currentCode);
                }
        }
}
