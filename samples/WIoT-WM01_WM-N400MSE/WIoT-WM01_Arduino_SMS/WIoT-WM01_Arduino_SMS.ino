#include "Arduino.h"
#include "at_cmd_parser.h"

#define RESP_OK                            "OK\r\n"
#define RET_OK                             1
#define RET_NOK                            -1
#define DEBUG_ENABLE                       1
#define DEBUG_DISABLE                      0

#define WM_N400MSE_DEFAULT_BAUD_RATE      115200
#define BG96_PARSER_DELIMITER             "\r\n"

#define CATM1_DEVICE_NAME_WMN400MSE        "WM-N400MSE"
#define DEVNAME                            CATM1_DEVICE_NAME_WMN400MSE

#define LOGDEBUG(x)                        if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { Serial.print("["); Serial.print(F(DEVNAME)); Serial.print("] ");  Serial.println(x); }
#define MYPRINTF(x)                        { Serial.print("[MAIN] "); Serial.println(x); }

// Debug message settings
#define BG96_PARSER_DEBUG                  DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG                 DEBUG_ENABLE

char *phone_number = "Insert Your Phone Numer";
char *send_message = "Insert Your Message";
String rxdata;

ATCmdParser m_parser = ATCmdParser(&Serial3);

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
  MYPRINTF(">> Sample Code: SMS Test");
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

  setRecvSMSUart();

  sendSMS(phone_number, send_message);

}

void loop() {
  // put your main code here, to run repeatedly:
  rxdataHandling();
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
    MYPRINTF("WM-N400MSE AT Command Not Available\r\n");
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

  else if ( m_parser.send(F("AT$$STAT?")) &&
              m_parser.recv(F("$$STAT:%[^,],%[^\n]\n"), usim_stat, detail) &&
              m_parser.recv(F(RESP_OK)) ) {
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
      sprintf((char *)buf, "Network Status: %d, %d\r\n", atoi(mode), atoi(stat));
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
// Functions: Cat.M1 SMS
// ----------------------------------------------------------------

void sendSMS(char *_phone_number, char *_send_message)
{
  char buf[100];
  if ( (m_parser.send(F("AT*SMSMO=%s,,1,1,%s"), _phone_number, _send_message)
        //&& m_parser.recv(F(RESP_OK))
        && m_parser.recv(F("*SMSACK"))) ) {

    sprintf((char *)buf, "[SMS Send] to %s, \"%s\"\r\n", _phone_number, _send_message);

    LOGDEBUG(">> SMS send success\r\n");
    MYPRINTF(buf);
  }
}

void setRecvSMSUart(void)
{
  if ( (m_parser.send(F("AT*SKT*NEWMSG=4098"))
        && m_parser.recv(F(RESP_OK))) ) {
    LOGDEBUG("Set to receive SMS from UART: Success\r\n");
  }
}

void rxdataHandling(void)
{
  char buf[100];
  if (Serial3.available())
  {
    rxdata = Serial3.readStringUntil('\n');
    const char* chr = rxdata.c_str();

    if ( strstr(chr, "*SKT*NEWMSG:") != NULL )
    {
      sprintf((char *)buf, "[SMS Recv] %s\r\n", chr);

      LOGDEBUG("<< SMS recv success\r\n");
      MYPRINTF(buf);

      m_parser.send(F("AT*SKT*MTACK=0"));
      m_parser.recv(F(RESP_OK));
      m_parser.flush();
    }
  }
}



