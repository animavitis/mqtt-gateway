// ReceivedSignal store
void storeValue(String currentCode){
        long now = millis();
        // find oldest value of the buffer
        int o = getMin();
        Homie.getLogger() << " - storeValue("<<  currentCode << ")" << endl;
        Homie.getLogger() <<  " -- minimum index: " << o << endl;
        // replace it by the new one
        ReceivedSignal[o][0] = currentCode;
        ReceivedSignal[o][1] = now;
        ReceivedSignal[o][2] = alarmState;
        Homie.getLogger() << " -- No.: value / timestamp / alarmstate" << endl;

#ifdef ALARM_ACTIVE
  long currentCodeLong = currentCode.toInt();

 if (currentCodeLong == (armHomeButton[0] || armHomeButton[1] || armHomeButton[2])){
   setAlarmState("armed_home");
 }
 if (currentCodeLong == (armAwayButton[0] || armAwayButton[1] || armAwayButton[2])){
   setAlarmState("armed_away");
 }
 if (currentCodeLong == (disarmButton[0] || disarmButton[1] || disarmButton[2])){
   setAlarmState("disarmed");
 }
 if (currentCodeLong == (triggerButton[0] || triggerButton[1] || triggerButton[2])){
   setAlarmState("triggered");
 }
#endif

        for (int i = 0; i < 5; i++)
        {
                Homie.getLogger() << " -- " << String(i+1) << ": " << ReceivedSignal[i][0] << " / " << String(ReceivedSignal[i][1]) << " / " << String(ReceivedSignal[i][2]) << endl;
        }
}
// ReceivedSignal helpers
int getMin(){
        int minimum = 0;
        minimum = ReceivedSignal[0][1].toInt();
        int minindex=0;
        for (int i = 0; i < 5; i++)
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
        //    Homie.getLogger() << " - isAduplicate("<<  value << ")" << endl;
        for (int i=0; i<5; i++) {
                if (ReceivedSignal[i][0] == value) {
                        long now = millis();
                        if (now - ReceivedSignal[i][1].toInt() < TIME_AVOID_DUPLICATE * 1000UL) { // change
                                Homie.getLogger() << " -- Duplicate found, don't send" << endl;
                                return true;
                        }
                }
        }
        Homie.getLogger() << " -- not dulicated, will be sent" << endl;
        return false;
}
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

bool rfSwitchOnHandler(const HomieRange& range, const String& value) {
        Homie.getLogger() << " - rfSwitchOnHandler(range," << value << ")" << endl;
        bool result = false;
        arrayMQTT[0] = 0;     //data or address
        arrayMQTT[1] = 350;   //pulseLength
        arrayMQTT[2] = 1;     //protocol

        getArrayMQTT(value);

        mySwitch.setPulseLength(arrayMQTT[1]);
        mySwitch.setProtocol(arrayMQTT[2]);

        if (arrayMQTT[0] > 0)  {
                Homie.getLogger() << " -- Receiving MQTT > 433Mhz pulseLength: " << arrayMQTT[1] << " protocol: "<< arrayMQTT[2] <<" value: " << arrayMQTT[0] << endl;
                if(arrayMQTT[2] != 7) {
                        mySwitch.send(arrayMQTT[0], 24);
                }else{
                        mySwitch.send(arrayMQTT[0], 32);
                }
        }



        result = rfSwitchNode.setProperty("code").send(String(arrayMQTT[0]));
        if (result) Homie.getLogger() << " -- 433Mhz pulseLength: " << arrayMQTT[1] << "  value: " << arrayMQTT[0] << " sent"<< endl;
        return true;
}
void loopRfToMqtt(){
        if (mySwitch.available()) {
                long data = mySwitch.getReceivedValue();
                //              Homie.getLogger() << " - 433Mhz loop:" << endl;
                Homie.getLogger() << " -- Receiving 433Mhz value: " << mySwitch.getReceivedValue();
                Homie.getLogger() << " bitLenght: " << mySwitch.getReceivedBitlength();
                Homie.getLogger() << " protocol: " << mySwitch.getReceivedProtocol();
                Homie.getLogger() << " delay: " << mySwitch.getReceivedDelay() << endl;
                mySwitch.resetAvailable();
                String currentCode = String(data);
                if (!isAduplicate(currentCode) && currentCode!=0) {
                        Homie.getLogger() << " -- Code: " << currentCode << endl;
                        boolean result = rfSwitchNode.setProperty("toMQTT").send(currentCode);
                        if (result) storeValue(currentCode);
                }
        }
}
