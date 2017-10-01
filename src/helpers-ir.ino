#ifdef IR_ACTIVE
void loopIrToMqtt() {
        if (irrecv.decode(&results)) {
                //Homie.getLogger() << " - IR loop:" << endl;
                long data = results.value;
                irrecv.resume();
                Homie.getLogger() << " -- Receiving IR signal: " << data << endl;
                String currentCode = String(data);
                if (!isAduplicate(currentCode) && currentCode!=0) {
                        Homie.getLogger() << " -- Code: " << currentCode << endl;
                        boolean result = receiverNode.setProperty("ir").send(currentCode);
                        if (result) storeValue(currentCode);
                }
        }
}
bool irSwitchOnHandler(const HomieRange& range, const String& value) {
        Homie.getLogger() << " - irSwitchOnHandler(range," << value << ")" << endl;
        arrayMQTT[0] = 0;         //data or address
        arrayMQTT[1] = 1;         // IR protocol: 0:IR_COOLIX,1:IR_NEC,2:IR_Whynter,3:IR_LG,4:IR_Sony,5:IR_DISH,6:IR_RC5,7:IR_Sharp,8:IR_SAMSUNG
        arrayMQTT[2] = 0;         //not used
        arrayMQTT[3] = 0;         //not used
        arrayMQTT[4] = 0;         //not used
        arrayMQTT[5] = 0;         //not used
        getArrayMQTT(value);
        Homie.getLogger() << " -- Receiving MQTT > IR Protocol: " << arrayMQTT[1] << " value: " << arrayMQTT[0] << endl;
        boolean signalSent = false;
        if (arrayMQTT[1] == 0) {irsend.sendCOOLIX(arrayMQTT[0], 24);
                                signalSent = true;}
        if (arrayMQTT[1] == 1) {irsend.sendNEC(arrayMQTT[0], 32);
                                signalSent = true;}
        if (arrayMQTT[1] == 2) {irsend.sendWhynter(arrayMQTT[0], 32);
                                signalSent = true;}
        if (arrayMQTT[1] == 3) {irsend.sendLG(arrayMQTT[0], 28);
                                signalSent = true;}
        if (arrayMQTT[1] == 4) {irsend.sendSony(arrayMQTT[0], 12);
                                signalSent = true;}
        if (arrayMQTT[1] == 5) {irsend.sendDISH(arrayMQTT[0], 16);
                                signalSent = true;}
        if (arrayMQTT[1] == 6) {irsend.sendRC5(arrayMQTT[0], 12);
                                signalSent = true;}
        if (arrayMQTT[1] == 7) {irsend.sendSharpRaw(arrayMQTT[0], 15);
                                signalSent = true;}
        if (arrayMQTT[1] == 8) {irsend.sendSAMSUNG(arrayMQTT[0], 32);
                                signalSent = true;}
        if (signalSent) {
                boolean result = irSwitchNode.setProperty("code").send(String(arrayMQTT[0]));
                if (result) Homie.getLogger() << " -- IR protocol: " << arrayMQTT[1] << "  value: " << arrayMQTT[0] << " sent"<< endl;
        }
        irrecv.enableIRIn();
        return true;
}
#endif
