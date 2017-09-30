#ifdef RF_KAKU



void loopRf2ToMqtt(){
        if(rf2rd.hasNewData) {
                rf2rd.hasNewData=false;
                String MQTTAddress;
                String MQTTperiod;
                String MQTTunit;
                String MQTTgroupBit;
                String MQTTswitchType;

                MQTTAddress = String(rf2rd.address);
                MQTTperiod = String(rf2rd.period);
                MQTTunit = String(rf2rd.unit);
                MQTTgroupBit = String(rf2rd.groupBit);
                MQTTswitchType = String(rf2rd.switchType);
                String currentCode = MQTTAddress + MQTTperiod + MQTTunit + MQTTgroupBit + MQTTswitchType;
                Homie.getLogger() << " -- Receiving MQTT22222" << currentCode << endl;
                if (!isAduplicate(currentCode) && currentCode!=0) {
                        boolean result = receiverNode.setProperty("rf2").send(currentCode);
                        if (result) storeValue(currentCode);
                }
        }

}


void rf2Callback(unsigned int period, unsigned long address, unsigned long groupBit, unsigned long unit, unsigned long switchType) {

        rf2rd.period=period;
        rf2rd.address=address;
        rf2rd.groupBit=groupBit;
        rf2rd.unit=unit;
        rf2rd.switchType=switchType;
        rf2rd.hasNewData=true;

}

#endif
