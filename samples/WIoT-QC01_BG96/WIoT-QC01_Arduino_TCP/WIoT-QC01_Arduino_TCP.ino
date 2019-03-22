#include "Arduino.h"
#include "at_cmd_parser.h"
#include <Timer.h>

#define RESP_OK                            "OK\r\n"
#define RET_OK                             1
#define RET_NOK                            -1
#define DEBUG_ENABLE                       1
#define DEBUG_DISABLE                      0

#define BG96_APN_PROTOCOL_IPv4             1
#define BG96_APN_PROTOCOL_IPv6             2
#define BG96_DEFAULT_TIMEOUT               1000
#define BG96_CONNECT_TIMEOUT        15000
#define BG96_SEND_TIMEOUT           500
#define BG96_RECV_TIMEOUT           500

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
#define CATM1_DEVICE_DEBUG                 DEBUG_ENABLE

ATCmdParser m_parser = ATCmdParser(&Serial3);

unsigned long lastConnectedTime = 0;         // last time you connected to the server, in milliseconds
char dest_ip[] = "222.98.173.203";
int  dest_port = 8000;
char rxbuf[40];

void setup() {
  m_parser.debug_on(1);

  char buf[30];

  char buf2[40];
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
  MYPRINTF(">> Sample Code: TCP Test");
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


  setContextActivate_BG96();
  sprintf((char *)buf2, "TCP Client Start - Connect to %s:%d\r\n", dest_ip, dest_port);
  MYPRINTF(buf2);

  if (sockOpenConnect_BG96("TCP", dest_ip, dest_port) == RET_OK) {
    MYPRINTF("sockOpenConnect: success\r\n");

    char sendbuf[] = "Hello Cat.M1\r\n";
    if (sendData_BG96(sendbuf, sizeof(sendbuf))) {
      sprintf((char *)buf, "dataSend [%d] %s\r\n", sizeof(sendbuf), sendbuf);
      MYPRINTF(buf);
    }

  } else {
    MYPRINTF("sockOpen: failed\r\n");

    if (sockClose_BG96() == RET_OK) {
      MYPRINTF("sockClose: success\r\n");
    }
  }
}



void loop() {
  // put your main code here, to run repeatedly:
  while (1)
  {
    if (checkRecvData_BG96() == RET_OK) {
      // Data received
      char recvbuf[100] = {0, };
      int recvlen = 0;

      if (recvData_BG96(recvbuf, &recvlen) == RET_OK) {
        sprintf((char *)rxbuf, "dataRecv [%d] %s\r\n", recvlen, recvbuf);
        MYPRINTF(rxbuf);

        char * ptr = strstr(recvbuf, "exit");
        if (ptr != 0) break;
      }
    }

  }

  if (sockClose_BG96() == RET_OK) {
    MYPRINTF("sockClose: success\r\n");
  }
  setContextDeactivate_BG96();
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
// Functions: Cat.M1 DNS
// ----------------------------------------------------------------

int8_t getIpAddressByName_BG96(const char * name, char * ipstr)
{
  char buf2[50];
  bool ok;
  int  err, ipcount, dnsttl;

  int8_t ret = RET_NOK;

  ok = ( m_parser.send("AT+QIDNSGIP=1,\"%s\"", name)
         && m_parser.recv("OK")
         && m_parser.recv("+QIURC: \"dnsgip\",%d,%d,%d", &err, &ipcount, &dnsttl)
         && err == 0
         && ipcount > 0
       );

  if ( ok ) {
    m_parser.recv("+QIURC: \"dnsgip\",\"%[^\"]\"", ipstr);       //use the first DNS value
    for ( int i = 0; i < ipcount - 1; i++ )
      m_parser.recv("+QIURC: \"dnsgip\",\"%[^\"]\"", buf2);   //and discrard the rest  if >1

    ret = RET_OK;
  }
  return ret;
}

// ----------------------------------------------------------------
// Functions: Cat.M1 PDP context activate / deactivate
// ----------------------------------------------------------------

void setContextActivate_BG96(void)
{
  if ( (m_parser.send(F("AT+QIACT=1"))
        && m_parser.recv(F(RESP_OK))) ) {
    LOGDEBUG("PDP Context Activation: Success\r\n");
  }
}

int8_t setContextDeactivate_BG96(void) // Deactivate a PDP Context
{
  if ( (m_parser.send(F("AT+QIDEACT=1"))
        && m_parser.recv(F(RESP_OK))) ) {
    LOGDEBUG("PDP Context Deactivation: Success\r\n");
  }
}

int8_t getIpAddress_BG96(char * ipstr) // IPv4 or IPv6
{
  int8_t ret = RET_NOK;
  int id, state, type; // not used

  m_parser.send("AT+QIACT?");
  if (m_parser.recv("+QIACT: %d,%d,%d,\"%[^\"]\"", &id, &state, &type, ipstr)
      && m_parser.recv("OK")) {
    ret = RET_OK;
  }
  return ret;
}

// ----------------------------------------------------------------
// Functions: TCP/UDP socket service
// ----------------------------------------------------------------

int8_t sockOpenConnect_BG96(const char * type, const char * addr, int port)
{
  int8_t ret = RET_NOK;
  int err = 1;
  int id = 0;

  bool done = false;
  //Timer t;

  m_parser.set_timeout(BG96_CONNECT_TIMEOUT);

  if ((strcmp(type, "TCP") != 0) && (strcmp(type, "UDP") != 0)) {
    return RET_NOK;
  }

  //t.start();
  lastConnectedTime = millis();
  m_parser.send("AT+QIOPEN=1,%d,\"%s\",\"%s\",%d", id, type, addr, port);
  do {
    done = (m_parser.recv("+QIOPEN: %d,%d", &id, &err) && (err == 0));
  } while (!done && (millis() - lastConnectedTime) < BG96_CONNECT_TIMEOUT);

  if (done) ret = RET_OK;

  m_parser.set_timeout(BG96_DEFAULT_TIMEOUT);
  m_parser.flush();

  return ret;
}

int8_t sockClose_BG96(void)
{
  int8_t ret = RET_NOK;
  int id = 0;

  m_parser.set_timeout(BG96_CONNECT_TIMEOUT);

  if (m_parser.send("AT+QICLOSE=%d", id) && m_parser.recv("OK")) {
    ret = RET_OK;
  }
  m_parser.set_timeout(BG96_DEFAULT_TIMEOUT);

  return ret;
}

int8_t sendData_BG96(char * data, int len)
{
  int8_t ret = RET_NOK;
  int id = 0;
  bool done = false;
  char buf[40];

  m_parser.set_timeout(BG96_SEND_TIMEOUT);
  sprintf((char *)buf, "AT+QISEND=%d,%d", id, len);
  m_parser.send(buf);
  if ( !done && m_parser.recv(">") )
    done = (m_parser.write(data, len) <= 0);

  if ( !done )
    done = m_parser.recv("SEND OK");

  m_parser.set_timeout(BG96_DEFAULT_TIMEOUT);

  return ret;
}



int8_t checkRecvData_BG96(void)
{
  int8_t ret = RET_NOK;
  int id = 0;
  char cmd[20];

  bool received = false;

  sprintf(cmd, "PSM POWER DOWN");
  m_parser.set_timeout(1);
  received = m_parser.recv(cmd);
  m_parser.set_timeout(BG96_DEFAULT_TIMEOUT);

  if (received) ret = RET_OK;
  return ret;
}

int8_t recvData_BG96(char * data, int * len)
{
  int8_t ret = RET_NOK;
  int id = 0;
  int recvCount = 0;

  m_parser.set_timeout(BG96_RECV_TIMEOUT);

  if ( m_parser.send("AT+QIRD=%d", id) && m_parser.recv("+QIRD:%d\r\n", &recvCount) ) {
    if (recvCount > 0) {
      //m_parser.getc();
      m_parser.read(data, recvCount + 1);
      if (m_parser.recv("OK")) {
        ret = RET_OK;
      } else {
        recvCount = 0;
      }
    }
  }
  m_parser.set_timeout(BG96_DEFAULT_TIMEOUT);
  m_parser.flush();

  *len = recvCount;

  return ret;
}
