#ifdef ALARM_ACTIVE
void loopAlarm() {
        if (millis() > (initialAlarmStateTime + ALARM_INTERVAL * 1000UL) || initialAlarmState == 0) {
                initialAlarmStateTime = millis();
                Homie.getLogger() << " - alarm status: " << alarmState << endl;
                alarmNode.setProperty("state").send(alarmState);
                initialAlarmState = 1;
        }
        disarmCheck();
        homeCheck();
        awayCheck();
        pendingCheck();
        triggeredCheck();
}

void readAlarmStateFromSpiffs() {
        File g = SPIFFS.open("/alarm.txt", "r");
        if (!g) {
                Homie.getLogger() << " -- file open failed" << endl;
        }
        String line = g.readStringUntil('\n');
        alarmState = line;
        Homie.getLogger() << " - readAlarmStateFromSpiffs()" << endl <<  " -- alarmState was: " << alarmStateOld << " is: " << alarmState << " will be: "  << alarmStateTarget << endl;
        g.close();
}

void writeAlarmStateToSpiffs(String value) {
        File f = SPIFFS.open("/alarm.txt", "w");
        if (!f) {
                Homie.getLogger() << " -- file write failed" << endl;
        }
        f.print(value);
        Homie.getLogger() <<  " - writeAlarmStateToSpiffs()" << endl << " -- alarmState write successfully"  << endl;
        f.close();
}


void setAlarmState(String value){
        if (value == "DISARM" || value == "disarmed") {
                alarmStateOld = alarmState;
                alarmState = alarmStates[0];
                alarmStateTarget = alarmStates[0];
                lastDisarmedTime = millis();
        }
        if (value == "ARM_HOME" || value == "armed_home") {
                alarmStateOld = alarmState;
                alarmState = alarmStates[3];
                alarmStateTarget = alarmStates[1];
                pendingCounter = millis();
                pendingStatusSent = false;
                lastPendingTime =  millis();
        }
        if (value == "ARM_AWAY" || value == "armed_away") {
                alarmStateOld = alarmState;
                alarmState = alarmStates[3];
                alarmStateTarget = alarmStates[2];
                pendingCounter = millis();
                pendingStatusSent = false;
                lastPendingTime =  millis();
        }
        if (value == "pending") {
                alarmStateOld = alarmState;
                alarmState = alarmStates[3];
                alarmStateTarget = alarmStates[0];
                pendingCounter = millis();
                pendingStatusSent = false;
                lastPendingTime =  millis();
        }
        if (value == "triggered") {
                alarmStateOld = alarmState;
                alarmState = alarmStates[4];
                alarmStateTarget = alarmStates[4];
                pendingCounter = millis();
                pendingStatusSent = false;
                lastPendingTime =  millis();
        }
        alarmNode.setProperty("state").send(alarmState);
        writeAlarmStateToSpiffs(alarmStateTarget);
        Homie.getLogger() << " - setAlarmState("<<  value << ")" << endl <<  " -- alarmState was: " << alarmStateOld << " is: " << alarmState << " will be: "  << alarmStateTarget << endl;
}
void setAlarmTimes(){
        lastArmedHomeTime = millis();
        lastPendingTime = millis();
        lastArmedAwayTime = millis();
        lastDisarmedTime = millis();
        lastTriggeredTime = millis();
}
void disarmCheck(){
        if (alarmState == alarmStates[0]) {  }
}
void homeCheck(){
        if (alarmState == alarmStates[1]) {

                for (size_t i = 0; i < 10; i++) {
                        for (size_t j = 0; j < 5; j++) {
                                if (ReceivedSignal[j][0].toInt() == arrayHome[i] &&
                                    ReceivedSignal[j][1].toInt()  > lastArmedHomeTime &&
                                    lastArmedHomeTime  > lastArmedAwayTime &&
                                    lastArmedHomeTime  > lastDisarmedTime &&
                                    lastArmedHomeTime  > lastPendingTime
                                    ) {
                                        alarmStateOld = alarmState;
                                        alarmState = alarmStates[3];
                                        alarmStateTarget = alarmStates[4];
                                        pendingCounter = millis();
                                        pendingStatusSent = false;
                                        Homie.getLogger() << " - homeCheck()" << endl <<  " -- alarmState was: " << alarmStateOld << " is: " << alarmState << " will be: "  << alarmStateTarget << endl;
                                }
                        }
                }
        }
}
void awayCheck(){
        if (alarmState == alarmStates[2]) {

                for (size_t i = 0; i < 10; i++) {
                        for (size_t j = 0; j < 5; j++) {
                                // Serial << ReceivedSignal[j][0].toInt() == arrayAway[i] << " if2 " << ReceivedSignal[j][1].toInt()  > lastArmedAwayTime << " arrayaway " << arrayAway[i] << " timestamp " << lastArmedAwayTime << endl;
                                if (arrayAway[i] > 0 &&
                                    ReceivedSignal[j][0].toInt() == arrayAway[i] &&
                                    ReceivedSignal[j][1].toInt()  > lastArmedAwayTime &&
                                    lastArmedAwayTime  > lastArmedHomeTime &&
                                    lastArmedAwayTime  > lastDisarmedTime &&
                                    lastArmedAwayTime  > lastPendingTime
                                    ) {
                                        //                Serial << ReceivedSignal[j][0].toInt() << " rec_timestamp " << ReceivedSignal[j][1] << " arrayaway " << arrayAway[i] << " timestamp " << lastArmedAwayTime << endl;
                                        alarmStateOld = alarmState;
                                        alarmState = alarmStates[3];
                                        alarmStateTarget = alarmStates[4];
                                        pendingCounter = millis();
                                        pendingStatusSent = false;
                                        Homie.getLogger() << " - awayCheck()" << endl;
                                        Homie.getLogger() << " -- alarmState was: " << alarmStateOld << " is: " << alarmState << " will be: "  << alarmStateTarget << endl;
                                }
                        }
                }
        }
}
void pendingCheck(){
        if (alarmState == alarmStates[3]) {
                if (millis() > (pendingCounter + TIME_TO_TRIGGER * 1000UL)) {
                        if (pendingCounter > 0) {

                                alarmState = alarmStateTarget;
                                alarmNode.setProperty("state").send(alarmState);
                                Homie.getLogger() << " - pendingCheck()" << endl <<  " -- alarmState was: " << alarmStateOld << " is: " << alarmState << endl;

                                if(alarmState == alarmStates[0]) {lastDisarmedTime = millis();}
                                if(alarmState == alarmStates[1]) {lastArmedHomeTime = millis();}
                                if(alarmState == alarmStates[2]) {lastArmedAwayTime = millis();}
                                if(alarmState == alarmStates[4]) {lastTriggeredTime = millis();}
                        }
                        pendingCounter = millis();
                } else {
                        if(!pendingStatusSent) {
                                alarmNode.setProperty("state").send(alarmState);
                                lastPendingTime = millis();
                                pendingStatusSent = true;
                        }
                }
        }
}
void triggeredCheck(){
        if (alarmState == alarmStates[4]) {
                if (millis() > (initialAlarmStateTime + (ALARM_INTERVAL * 1000UL)/4) || initialAlarmState == 0) {
                        initialAlarmStateTime = millis();
                        alarmNode.setProperty("state").send(alarmState);

                        for (size_t i = 0; i < 10; i++) {
                                if (arrayTrigger[i] > 0) {
                                        mySwitch.send(arrayTrigger[i], 24);
                                        delay(50);
                                }
                        }

                        Homie.getLogger() << " - triggeredCheck()" << endl <<  " -- alarmState was: " << alarmStateOld << " is: " << alarmState << endl;
                        initialAlarmState = 1;
                }
        }
}
void getAlarmArrays(){
        DynamicJsonBuffer jsonTriggered;
        String json = alarmArraySetting.get();
        JsonObject& root = jsonTriggered.parseObject(json);
        for (int i = 0; i < 10; i++)
        {
                arrayTrigger[i] = root["siren"][i];
                arrayHome[i] = root["home"][i];
                arrayAway[i] = root["away"][i];
                  Homie.getLogger() <<  " -- test array: " << arrayTrigger[i] << " " << arrayHome[i] << "  " << arrayAway[i] << endl;
        }
}
void getAlarmButtons(){
        DynamicJsonBuffer jsonTriggered;
        String json = alarmButtonSetting.get();
        JsonObject& root = jsonTriggered.parseObject(json);
        for (int i = 0; i < 3; i++)
        {
                armHomeButton[i] = root["home"][i];
                armAwayButton[i] = root["away"][i];
                disarmButton[i] = root["disarm"][i];
                triggerButton[i] = root["trigger"][i];
                  Homie.getLogger() <<  " -- test buttons: " << armHomeButton[i] << "  " << armAwayButton[i] << "  " << disarmButton[i] << "  " << triggerButton[i] << endl;

        }
}


bool alarmSwitchOnHandler(const HomieRange& range, const String& value) {
        Homie.getLogger() << " - alarmSwitchOnHandler(range," << value << ")" << endl;
        String data = value.c_str();
        Homie.getLogger() << " -- Receiving MQTT alarm status: " << data << endl;
        setAlarmState(data);
        return true;
}
#endif
