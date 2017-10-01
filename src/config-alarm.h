#define ALARM_ACTIVE                    // comment to disable
#define ALARM_INTERVAL 60              // in seconds
#define TIME_TO_TRIGGER 30              // in seconds
const String alarmStates[5] = {"disarmed","armed_home","armed_away","pending","triggered"};
String alarmStateOld = alarmStates[0];
String alarmStateTarget = alarmStates[0];
long lastArmedHomeTime = 0;
long lastPendingTime = 0;
long lastArmedAwayTime = 0;
long lastDisarmedTime = 0;
long lastTriggeredTime = 0;
long pendingCounter = 0;
bool pendingStatusSent = true;
long initialAlarmState = 0;
long initialAlarmStateTime = 0;
long arrayHome[10] = {0,0,0,0,0,0,0,0,0,0};
long arrayAway[10] = {0,0,0,0,0,0,0,0,0,0};
