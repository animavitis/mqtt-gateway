// // getChannel helper
// String getChannelByCode(const String & currentCode) {
//         String mappingConfig = channelMappingSetting.get();
//         String mapping = "";
//         String codes = "";
//         int lastIndex = 0;
//         int lastCodeIndex = 0;
//         for (int i = 0; i < mappingConfig.length(); i++) {
//                 if (mappingConfig.substring(i, i + 1) == ";") {
//                         mapping = mappingConfig.substring(lastIndex, i);
//                         //                Homie.getLogger() << " - getChannelByCode("<<  currentCode << ")" << endl <<  " -- mapping: " << mapping << endl;
//                         codes = mapping.substring(mapping.indexOf(':') + 2, mapping.length() - 1);
//                         for (int j = 0; j < codes.length(); j++) {
//                                 if (codes.substring(j, j + 1) == ",") {
//                                         if (currentCode.indexOf(codes.substring(lastCodeIndex, j)) > -1) {
//                                                 return mapping.substring(0, mapping.indexOf(':'));;
//                                         }
//                                         codes = codes.substring(j + 1, codes.length());
//                                 }
//                         }
//                         if (currentCode.indexOf(codes) > -1) {
//                                 return mapping.substring(0, mapping.indexOf(':'));;
//                         }
//                         lastIndex = i + 1;
//                 }
//         }
//         return "0";
// }
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
                if (ipos < 6) {
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
        arrayMQTT[2] = -1;     //protocol for RF, DIM for RF2
        arrayMQTT[3] = 0;     //group (1-> send to all group) (only for typeE)
        arrayMQTT[4] = 0;     //unit (15 first, 14 second, 13 third) (only for typeE)
        arrayMQTT[5] = 1;     //1:set ON,2:set OFF (only for typeE)

        getArrayMQTT(value);

        #ifdef RF_KAKU
        NewRemoteReceiver::disable();
        #endif

        mySwitch.setPulseLength(arrayMQTT[1]);
        mySwitch.setProtocol(arrayMQTT[2]);



        if (arrayMQTT[0] > 0 && arrayMQTT[4] == 0) {
                Homie.getLogger() << " -- Receiving MQTT > 433Mhz pulseLength: " << arrayMQTT[1] << " protocol: "<< arrayMQTT[2] <<" value: " << arrayMQTT[0] << endl;
                mySwitch.send(arrayMQTT[0], 24);
        }
        if (arrayMQTT[0] > 0 && arrayMQTT[3] == 0 && arrayMQTT[4] > 0) {
                Homie.getLogger() << " -- Receiving MQTT > 433Mhz Address: " << arrayMQTT[0] << " unit: "<< arrayMQTT[4] <<" group: false" << endl;
                if(arrayMQTT[5] == 1) {


                        if(arrayMQTT[2] != -1) {
                                transmitter.sendDim(arrayMQTT[4], arrayMQTT[2]);
                        } else {
                                transmitter.sendUnit(arrayMQTT[4], true);
                        }
                } else {


                        if(arrayMQTT[2] != -1) {
                                transmitter.sendDim(arrayMQTT[4], arrayMQTT[2]);
                        } else {
                                transmitter.sendUnit(arrayMQTT[4], false);
                        }


                }
        }
        if (arrayMQTT[0] > 0 && arrayMQTT[3] == 1 && arrayMQTT[4] > 0) {
                Homie.getLogger() << " -- Receiving MQTT > 433Mhz Address: " << arrayMQTT[0] << " unit: "<< arrayMQTT[4] <<" group: true" << endl;

                if(arrayMQTT[5] == 1) {
                  if(arrayMQTT[2] != -1) {
                          transmitter.sendGroupDim(arrayMQTT[2]);
                  } else {
                          transmitter.sendGroup(true);
                  }
                } else {
                  if(arrayMQTT[2] != -1) {
                          transmitter.sendGroupDim(arrayMQTT[2]);
                  } else {
                          transmitter.sendGroup(false);
                  }
                }
        }
        if(arrayMQTT[5] == 1) {
                boolean result = rfSwitchNode.setProperty("on").send(String(arrayMQTT[0]));
        } else {
                boolean result = rfSwitchNode.setProperty("off").send(String(arrayMQTT[0]));
        }
        if (result) Homie.getLogger() << " -- 433Mhz pulseLength: " << arrayMQTT[1] << "  value: " << arrayMQTT[0] << " sent"<< endl;
        delay(1500);
        return true;
}
void loopZrfToMqtt(){
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
//                  String channelId = getChannelByCode(currentCode);
//                  Homie.getLogger() << " -- Code: " << currentCode << " matched to channel " << channelId << endl;
                        Homie.getLogger() << " -- Code: " << currentCode << endl;

                        //                  boolean result = receiverNode.setProperty("rf" + channelId).send(currentCode);
                        boolean result = receiverNode.setProperty("rf").send(currentCode);
                        if (result) storeValue(currentCode);
                }
        }
}
