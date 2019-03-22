#include "Arduino.h"
#include "at_cmd_parser.h"

#define RESP_OK                            "OK\r\n"
#define RET_OK                             1
#define RET_NOK                            -1
#define DEBUG_ENABLE                       1
#define DEBUG_DISABLE                      0

#define WM_N400MSE_DEFAULT_BAUD_RATE      115200
#define BG96_PARSER_DELIMITER             "\r\n"

#define CATM1_DEVICE_NAME_WMN400MSE       "WM-N400MSE"
#define DEVNAME                           CATM1_DEVICE_NAME_WMN400MSE

#define LOGDEBUG(x)                       if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { Serial.print("["); Serial.print(F(DEVNAME)); Serial.print("] ");  Serial.println(x); }
#define MYPRINTF(x)                       if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { Serial.print("[MAIN] "); Serial.println(x); }

// Sensors
#define MBED_CONF_IOTSHIELD_SENSOR_CDS     A0
#define MBED_CONF_IOTSHIELD_SENSOR_TEMP    A1

// Debug message settings 
#define CATM1_DEVICE_DEBUG                DEBUG_ENABLE

#define REQUESTED_PERIODIC_TAU            "10000110"
#define REQUESTED_ACTIVE_TIME             "00100001"

ATCmdParser m_parser = ATCmdParser(&Serial3);

char hour[5], minute[5], second[5];
char timeBuf[35];

void setup() {
  // put your setup code here, to run once:
  serialPcInit();
  catm1DeviceInit();

  MYPRINTF("Waiting for Cat.M1 Module Ready...\r\n");

  waitCatM1Ready();

  MYPRINTF("System Init Complete\r\n");

  MYPRINTF("WIZnet IoT Shield for Arduino");
  MYPRINTF("LTE Cat.M1 Version");
  MYPRINTF("=================================================");
  MYPRINTF(">> Target Board: WIoT-WM01 (Woori-Net WM-N400MSE)");
  MYPRINTF(">> Sample Code: Ping Test");
  MYPRINTF("=================================================\r\n");

  for (int i = 0; i < 5; i++)
  {
    Serial.println("");
    if (initStatus() == true) {
      break;
    }
    else {
      LOGDEBUG("Please Check your H/W Status\r\n");
    }
    delay(1000);
  }

  setContextActivate_wm01();

  setPSMActivate(REQUESTED_PERIODIC_TAU, REQUESTED_ACTIVE_TIME);
  
  moduleReset();
}

void loop() {
  // put your main code here, to run repeatedly:
  getcurrentTime();
  delay(1000);

}

void serialPcInit(void)
{
  Serial.begin(115200);
}

void serialDeviceInit()
{
  Serial3.begin(WM_N400MSE_DEFAULT_BAUD_RATE);
}

void serialAtParserInit()
{
  m_parser.set_timeout(1000);
  m_parser.set_delimiter("\r");
}
void catm1DeviceInit()
{
  serialDeviceInit();
  serialAtParserInit();
}

// ----------------------------------------------------------------
// Functions: Cat.M1 Status
// ----------------------------------------------------------------

int8_t waitCatM1Ready()
{
  bool initOK = false;
  for (int i = 0; i < 20; i++)
  {
    if (m_parser.send(F("AT")) && m_parser.recv(F(RESP_OK))) {
      MYPRINTF("WM-N400MSE AT Command Available\r\n");
      initOK = true;
      return RET_OK;
    }
    delay(1000);
  }
  if (!initOK) {
    Serial.println("[WM-N400MSE] AT Command: Not Available");
    return RET_NOK;
  }
}

bool initStatus()
{

  if ( setEchoStatus_wm01(false) != RET_OK )
  {
    return false;
  }

  if ( getUsimStatus_wm01() != RET_OK )
  {
    return false;
  }

  if ( getNetworkStatus_wm01() != RET_OK )
  {
    return false;
  }

  return true;
}

int8_t setEchoStatus_wm01(bool onoff)
{
  if ( onoff == true )
  {
    if ( !(m_parser.send(F("ATE1")) && m_parser.recv(F(RESP_OK))) ) {
      LOGDEBUG("Echo On: Failed\r\n");
      return RET_NOK;
    }
    else
    {
      LOGDEBUG("Echo On: Success\r\n");
      return RET_OK;
    }

  }
  else if ( onoff == false )
  {
    if ( !(m_parser.send(F("ATE0")) && m_parser.recv(F(RESP_OK))) ) {
      LOGDEBUG("Echo Off: Failed\r\n");
      return RET_NOK;
    }
    else
    {
      LOGDEBUG("Echo Off: Success\r\n");
      return RET_OK;
    }
  }
}

int8_t getUsimStatus_wm01(void)
{
  char usim_stat[10], detail[10];
  char buf[40];

  if ( m_parser.send(F("AT$$STAT?")) &&
       m_parser.recv(F("$$STAT:READY\n")) &&
       m_parser.recv(F(RESP_OK)) ) {
    LOGDEBUG("USIM Status: READY\r\n");
    return RET_OK;
  }

  else if ( !(m_parser.send(F("AT$$STAT?")) && m_parser.recv(F("$$STAT:%[^,],%[^\n]\n"), usim_stat, detail)) ) {
    sprintf((char *)buf, "USIM Satatus: %s, %s", usim_stat, detail);
    LOGDEBUG(buf);
    return RET_NOK;
  }
}

int8_t getNetworkStatus_wm01(void)
{
  char mode[10], stat[10];
  char buf[10];

  if ( m_parser.send(F("AT+CEREG?")) &&
       m_parser.recv(F("+CEREG: %[^,],%[^\n]\n"), mode, stat) &&
       m_parser.recv(F(RESP_OK)) ) {

    if ( (atoi(mode) == 0) && (atoi(stat) == 1) ) {
      LOGDEBUG("Network Status: Attach\r\n");
      return RET_OK;
    }

    else if (( atoi(stat) != 1 )) {
      sprintf((char *)buf, "Network Status: %d, %d", atoi(mode), atoi(stat));
      LOGDEBUG(buf);
      return RET_NOK;
    }
  }
}

void getIMEIInfo_wm01(void)
{
  char m_imei[30];
  char buf[25];

  if ( (m_parser.send(F("AT*MINFO"))
        && m_parser.recv(F("*MINFO:%*[^,],%*[^,],%[^,],%*[^\n]\n"), m_imei)
        && m_parser.recv(F(RESP_OK))) ) {
    sprintf((char *)buf, "Module IME: %s\r\n", m_imei);
    LOGDEBUG(buf);
  }
}

void getSwVersion_wm01(void)
{
  char m_sw_version[30];
  char buf[35];

  if ( (m_parser.send(F("AT$$SWVER"))
        && m_parser.recv(F("$$SWVER: %[^\n]\n"), m_sw_version)
        && m_parser.recv(F(RESP_OK))) ) {
    sprintf((char *)buf, "Module SW Ver: %s", m_sw_version);
    LOGDEBUG(buf);
  }
}

// ----------------------------------------------------------------
// Functions: Cat.M1 PDP context activate
// ----------------------------------------------------------------

void setContextActivate_wm01(void)
{
  if ( (m_parser.send(F("AT*RNDISDATA=1"))
        && m_parser.recv(F("\r\n"))
        && m_parser.recv(F(RESP_OK))) ) {
    LOGDEBUG("PDP Context Activation: Success\r\n");
  }
}

// ----------------------------------------------------------------
// Functions: Cat.M1 PSM activate / deactivate
// ----------------------------------------------------------------

void setPSMActivate(char *Requested_Periodic_TAU, char *Requested_Active_Time)
{
  if ( (m_parser.send(F("AT+CPSMS=1,,,\"%s\",\"%s\""), Requested_Periodic_TAU, Requested_Active_Time)
        && m_parser.recv(F(RESP_OK))) ) {
    LOGDEBUG("PSM Activate: Success\r\n");
  }
}

void setPSMDeactivate()
{
  if ( (m_parser.send(F("AT+CPSMS=0"))
        && m_parser.recv(F(RESP_OK))) ) {
    LOGDEBUG("PSM Deactivate: Success\r\n");
  }
}

// ----------------------------------------------------------------
// Functions: Cat.M1 module reset
// ----------------------------------------------------------------

void moduleReset(void)
{
  if ( (m_parser.send(F("AT*SKT*RESET"))
        && m_parser.recv(F("*SKT*RESET:1\r\n"))
        && m_parser.recv(F(RESP_OK))) ) {
    LOGDEBUG("Reset: Success\r\n");
  }
}

// ----------------------------------------------------------------
// Functions: Cat.M1 module get current time
// ----------------------------------------------------------------

void getcurrentTime(void)
{
  if ( m_parser.send(F("AT$$MSTIME?")) &&
       m_parser.recv(F("$$MSTIME: %*[^,],%*[^,],%*[^,],%*[^,],%[^,],%[^,],%[^\n]\n"), hour, minute, second) &&
       m_parser.recv(F(RESP_OK)) ) {

    sprintf((char *)timeBuf, "Current Time: %d h %d m %d s", atoi(hour), atoi(minute), atoi(second) );
    LOGDEBUG(timeBuf);
  }
}
