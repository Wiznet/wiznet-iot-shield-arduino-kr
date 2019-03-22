#include "Arduino.h"
#include "at_cmd_parser.h"

#define RESP_OK                            "OK\r\n"
#define RET_OK                             1
#define RET_NOK                            -1
#define DEBUG_ENABLE                       1
#define DEBUG_DISABLE                      0

#define BG96_APN_PROTOCOL_IPv4             1
#define BG96_APN_PROTOCOL_IPv6             2
#define BG96_DEFAULT_TIMEOUT               1000
#define BG96_CONNECT_TIMEOUT               15000
#define BG96_SEND_TIMEOUT                  500
#define BG96_RECV_TIMEOUT                  500

#define BG96_APN_PROTOCOL                  BG96_APN_PROTOCOL_IPv6
#define WM_N400MSE_DEFAULT_BAUD_RATE       115200
#define BG96_PARSER_DELIMITER              "\r\n"

#define CATM1_APN_SKT                      "lte-internet.sktelecom.com"

#define CATM1_DEVICE_NAME_BG96             "BG96"
#define DEVNAME                             CATM1_DEVICE_NAME_BG96

#define LOGDEBUG(x)                        if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { Serial.print("["); Serial.print(F(DEVNAME)); Serial.print("] ");  Serial.println(x); }
#define MYPRINTF(x)                        if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { Serial.print("[MAIN] "); Serial.println(x); }

// Sensors
#define MBED_CONF_IOTSHIELD_SENSOR_CDS     A0
#define MBED_CONF_IOTSHIELD_SENSOR_TEMP    A1

// Debug message settings
#define BG96_PARSER_DEBUG                  DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG                 DEBUG_ENABLE

#define SMS_EOF                            0x1A
#define MAX_SMS_SIZE                       100
 
char phone_number[] = "01090373914";
char send_message[] = "WIZnet Cat.M1 IoT shield is powered on";

ATCmdParser m_parser = ATCmdParser(&Serial3);
unsigned long recvTime = 0;

void setup() {
  char buf[40];
  char buf1[40];
  
  // put your setup code here, to run once:
  serialPcInit();
  catm1DeviceInit();

  MYPRINTF("Waiting for Cat.M1 Module Ready...\r\n");

  waitCatM1Ready();

  MYPRINTF("System Init Complete\r\n");

  MYPRINTF("WIZnet IoT Shield for Arduino");
  MYPRINTF("LTE Cat.M1 Version");
  MYPRINTF("=================================================");
  MYPRINTF(">> Target Board: WIoT-QC01 (Quectel BG96)");
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

  checknSetApn_BG96(CATM1_APN_SKT);

  // SMS configuration
  if (initSMS_BG96() != RET_OK) {
    MYPRINTF("[SMS Init] failed\r\n");
    while (1) {
      ;
    }
  }

  // Send a message
  if (sendSMS_BG96(phone_number, send_message, strlen(send_message)) == RET_OK) {
    sprintf((char *)buf, "[SMS Send] to %s, \"%s\"\r\n", phone_number, send_message);
    MYPRINTF(buf);
  }

#if 0
  // Delete messages
  deleteAllSMS_BG96(3);
#endif

  int msg_idx;
  char recv_message[MAX_SMS_SIZE] = {0, };
  char dest_addr[20] = {0, };
  char date_time[25] = {0, };

  m_parser.debug_on(DEBUG_DISABLE);

  while (1)
  {
    // SMS receive check
    msg_idx = checkRecvSMS_BG96();

    if (msg_idx > RET_NOK) { // SMS received
      m_parser.debug_on(BG96_PARSER_DEBUG);

      // Receive a message
      if (recvSMS_BG96(msg_idx, recv_message, dest_addr, date_time) == RET_OK) {
        sprintf((char *)buf1, "[SMS Recv] from %s, %s, \"%s\"\r\n", dest_addr, date_time, recv_message);
        MYPRINTF(buf1);
      }

      m_parser.debug_on(DEBUG_DISABLE);
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:


  
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
  while (1)
  {
    if (m_parser.recv(F("RDY"))) {
      MYPRINTF("BG96 ready\r\n");
      return RET_OK;
    }
    else if (m_parser.send(F("AT")) && m_parser.recv(F(RESP_OK)))
    {
      MYPRINTF("BG96 already available\r\n");
      return RET_OK;
    }
  }
  return RET_NOK;
}


bool initStatus()
{

  if ( setEchoStatus_BG96(false) != RET_OK )
  {
    return false;
  }

  if ( getUsimStatus_BG96() != RET_OK )
  {
    return false;
  }

  if ( getNetworkStatus_BG96() != RET_OK )
  {
    return false;
  }

  return true;
}

int8_t setEchoStatus_BG96(bool onoff)
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



int8_t getUsimStatus_BG96(void)
{
  char usim_stat[10], detail[10];
  char buf[40];

  if ( m_parser.send(F("AT+CPIN?")) &&
       m_parser.recv(F("+CPIN: READY\n")) &&
       m_parser.recv(F(RESP_OK)) ) {
    LOGDEBUG("USIM Status: READY\r\n");
    return RET_OK;
  }

  else if ( !(m_parser.send(F("AT+CPIN?")) && m_parser.recv(F("+CPIN: %[^,],%[^\n]\n"), usim_stat, detail)) ) {
    sprintf((char *)buf, "USIM Satatus: %s, %s", usim_stat, detail);
    LOGDEBUG(buf);
    return RET_NOK;
  }
}

int8_t checknSetApn_BG96(const char * apn) // Configure Parameters of a TCP/IP Context
{
  char resp_str[100];
  char buf[25];
  char buf1[25];

  uint16_t i = 0;
  char * search_pt;

  memset(resp_str, 0, sizeof(resp_str));

  LOGDEBUG("Checking APN...\r\n");


  m_parser.send(F("AT+QICSGP=1"));
  while (1)
  {
    m_parser.read(&resp_str[i++], 1);
    search_pt = strstr(resp_str, "OK\r\n");
    if (search_pt != 0)
    {
      break;
    }
  }

  search_pt = strstr(resp_str, apn);
  if (search_pt == 0)
  {
    sprintf((char *)buf, "Mismatched APN: %s\r\n", resp_str);
    sprintf((char *)buf1, "Storing APN %s...\r\n", apn);
    LOGDEBUG(buf);
    LOGDEBUG(buf1);

    if (!(m_parser.send("AT+QICSGP=1,%d,\"%s\",\"\",\"\",0", BG96_APN_PROTOCOL, apn) && m_parser.recv("OK")))
    {
      return RET_NOK; // failed
    }
  }
  LOGDEBUG("APN Check Done\r\n");

  return RET_OK;
}

int8_t getNetworkStatus_BG96(void)
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

void getIMEIInfo_BG96(void)
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

int8_t getFirmwareVersion_BG96(char * version)
{
  int8_t ret = RET_NOK;

  if (m_parser.send("AT+QGMR") && m_parser.recv("%s\n", version) && m_parser.recv("OK"))
  {
    ret = RET_OK;
  }
  return ret;
}

int8_t getImeiNumber_BG96(char * imei)
{
  int8_t ret = RET_NOK;

  if (m_parser.send("AT+CGSN") && m_parser.recv("%s\n", imei) && m_parser.recv("OK"))
  {
    ret = RET_OK;
  }
  return ret;
}



// ----------------------------------------------------------------
// Functions: Cat.M1 SMS
// ----------------------------------------------------------------

int8_t initSMS_BG96(void)
{
  int8_t ret = RET_NOK;
  bool msgformat = false, charset = false;

  // 0 = PDU mode / 1 = Text mode
  if (m_parser.send("AT+CMGF=1") && m_parser.recv("OK")) {        // Set SMS message format as text mode
    LOGDEBUG("SMS message format: Text mode\r\n");
    msgformat = true;
  }

  // "GSM" / "IRA" / "USC2"
  if (m_parser.send("AT+CSCS=\"GSM\"") && m_parser.recv("OK")) {  // Set character set as GSM
    LOGDEBUG("SMS character set: GSM\r\n");
    charset = true;
  }

  if (msgformat && charset) {
    ret = RET_OK;
  }

  return ret;
}

int8_t sendSMS_BG96(char * da, char * msg, int len)
{
  char buf[40];
  int8_t ret = RET_NOK;
  int msg_idx = 0;
  bool done = false;

  m_parser.set_timeout(BG96_CONNECT_TIMEOUT);

  m_parser.send("AT+CMGS=\"%s\"", da); // DA(Destination address, Phone number)
  if ( !done && m_parser.recv(">") )
    done = (m_parser.write(msg, len) <= 0) & m_parser.send("%c", SMS_EOF);

  if ( !done ) {
    done = (m_parser.recv("+CMGS: %d", &msg_idx) && m_parser.recv("OK"));
    if (done) {
      sprintf((char *)buf, ">> SMS send success: index %d\r\n", msg_idx);
      LOGDEBUG(buf);
      ret = RET_OK;
    }
  }
  m_parser.set_timeout(BG96_DEFAULT_TIMEOUT);

  return ret;
}

int checkRecvSMS_BG96(void)
{
  char buf[40];
  int ret = RET_NOK;
  int msg_idx = 0;

  bool received = false;

  m_parser.set_timeout(1);
  received = m_parser.recv("+CMTI: \"ME\",%d", &msg_idx);
  m_parser.set_timeout(BG96_DEFAULT_TIMEOUT);

  if (received) {
    ret = msg_idx;
    sprintf((char *)buf, "<< SMS received: index %d\r\n", msg_idx);
    LOGDEBUG(buf);
  }
  return ret;
}

int8_t recvSMS_BG96(int msg_idx, char * msg, char * da, char * datetime)
{
  int8_t ret = RET_NOK;
  char type[15] = {0, };
  char buf[40];
  int i = 0;
  bool done = false;
  char recv_msg[MAX_SMS_SIZE] = {0, };
  char * search_pt;
  //Timer t;

  memset(recv_msg, 0x00, MAX_SMS_SIZE);

  m_parser.set_timeout(BG96_RECV_TIMEOUT);

  if ( m_parser.send("AT+CMGR=%d", msg_idx)
       && m_parser.recv("+CMGR: \"%[^\"]\",\"%[^\"]\",,\"%[^\"]\"", type, da, datetime))
  {
    // timer start
    //t.start();
    recvTime = millis();

    while ( !done && ( (millis() - recvTime) < BG96_DEFAULT_TIMEOUT)) {
      m_parser.read(&recv_msg[i++], 1);
      search_pt = strstr(recv_msg, "OK");
      if (search_pt != 0) {
        done = true; // break;
      }
    }
    if (i > 8) {
      memcpy(msg, recv_msg + 2, i - 8);
      sprintf((char *)buf, "<< SMS receive success: index %d\r\n", msg_idx);
      LOGDEBUG(buf);
      ret = RET_OK;
    }
  }
  m_parser.set_timeout(BG96_DEFAULT_TIMEOUT);
  m_parser.flush();

  return ret;
}

int8_t deleteSMS_BG96(int msg_idx)
{
  int8_t ret = RET_NOK;
  char buf[40];

  if (m_parser.send("AT+CMGD=%d", msg_idx) && m_parser.recv("OK"))
  {
    sprintf((char *)buf, "Message index[%d] has been deleted\r\n", msg_idx);
    LOGDEBUG(buf);
    ret = RET_OK;
  }
  return ret;
}

int8_t deleteAllSMS_BG96(int delflag)
{
  int8_t ret = RET_NOK;
  char buf[40];

  //delflag == 1; // Delete all read messages from storage
  //delflag == 2; // Delete all read messages from storage and sent mobile originated messages
  //delflag == 3; // Delete all read messages from storage, sent and unsent mobile originated messages
  //delflag == 4; // Delete all messages from storage

  if (m_parser.send("AT+CMGD=0,%d", delflag) && m_parser.recv("OK"))
  {
    sprintf((char *)buf, "All messages has been deleted (delflag: %d)\r\n", delflag);
    LOGDEBUG(buf);
    ret = RET_OK;
  }
  return ret;
}
