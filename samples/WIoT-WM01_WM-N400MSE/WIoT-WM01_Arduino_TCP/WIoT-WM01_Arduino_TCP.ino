#include "Arduino.h"
#include "at_cmd_parser.h"
#include <SimpleTimer.h>

#define RESP_OK                            "OK\r\n"
#define RET_OK                             1
#define RET_NOK                            -1
#define DEBUG_ENABLE                       1
#define DEBUG_DISABLE                      0

#define WMN400MSE_DEFAULT_TIMEOUT               1000
#define WMN400MSE_CONNECT_TIMEOUT               15000
#define WMN400MSE_SEND_TIMEOUT                  500
#define WMN400MSE_RECV_TIMEOUT                  500

#define WM_N400MSE_DEFAULT_BAUD_RATE      115200
#define WMN400MSE_PARSER_DELIMITER             "\r\n"

#define CATM1_DEVICE_NAME_WMN400MSE        "WM-N400MSE"
#define DEVNAME                            CATM1_DEVICE_NAME_WMN400MSE

#define MBED_CONF_IOTSHIELD_CATM1_RESET     7
#define MBED_CONF_IOTSHIELD_CATM1_PWRKEY    9

#define LOGDEBUG(x)                        if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { Serial.print("["); Serial.print(F(DEVNAME)); Serial.print("] ");  Serial.println(x); }
#define MYPRINTF(x)                        { Serial.print("[MAIN] "); Serial.println(x); }

// Debug message settings 
#define WMN400MSE_PARSER_DEBUG             DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG                 DEBUG_ENABLE

ATCmdParser m_parser = ATCmdParser(&Serial3);

unsigned long lastConnectedTime = 0;         // last time you connected to the server, in milliseconds
char dest_ip[] = "222.xxx.xxx.xxx";
int  dest_port = 8000;
char rxbuf[40];
int  protocol = 1;                  // 1 : TCP, 2 : UPD
int  packet_type = 0;               // 0 : ASCII, 1 : HEX, 2 : Binary

void setup() {
  // put your setup code here, to run once:
  char buf[30];
  char buf2[40];
  
  serialPcInit();
  catm1DeviceInit();

  MYPRINTF("Waiting for Cat.M1 Module Ready...\r\n");
  
  catm1Device_reset();
  
  waitCatM1Ready();

  delay(5000);
  
  MYPRINTF("System Init Complete\r\n");

  MYPRINTF("WIZnet IoT Shield for Arduino");
  MYPRINTF("LTE Cat.M1 Version");
  MYPRINTF("=================================================");
  MYPRINTF(">> Target Board: WIoT-WM01 (Woori-Net WM-N400MSE)");
  MYPRINTF(">> Sample Code: TCP Test");
  MYPRINTF("=================================================\r\n");

  while(1)
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

    sprintf((char *)buf2, "TCP Client Start - Connect to %s:%d\r\n", dest_ip, dest_port);
  MYPRINTF(buf2);

  if (sockOpenConnect_WMN400MSE(protocol, dest_ip, dest_port, packet_type) == RET_OK) {
    MYPRINTF("sockOpenConnect: success\r\n");

    char sendbuf[] = "Hello Cat.M1\r\n";
    if (sendData_WMN400MSE(sendbuf, strlen(sendbuf))) {
      sprintf((char *)buf, "dataSend [%d] %s\r\n", sizeof(sendbuf), sendbuf);
      MYPRINTF(buf);
    }

  } else {
    MYPRINTF("sockOpen: failed\r\n");

    if (sockClose_WMN400MSE() == RET_OK) {
      MYPRINTF("sockClose: success\r\n");
    }
  }


}

void loop() {
  // put your main code here, to run repeatedly:
    while (1)
  {
  //  if (checkRecvData_WMN400MSE() == RET_OK) {
      // Data received
      char recvbuf[100] = {0, };
      int recvlen = 0;

      if (recvData_WMN400MSE(recvbuf, &recvlen) == RET_OK) {
        sprintf((char *)rxbuf, "dataRecv [%d] %s\r\n", recvlen, recvbuf);
        MYPRINTF(rxbuf);

        char * ptr = strstr(recvbuf, "exit");
        if (ptr != 0) break;
      }
   // }

  }

  if (sockClose_WMN400MSE() == RET_OK) {
    MYPRINTF("sockClose: success\r\n");
  }
  
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

void catm1Device_reset()
{
  pinMode(MBED_CONF_IOTSHIELD_CATM1_RESET, OUTPUT);
  pinMode(MBED_CONF_IOTSHIELD_CATM1_PWRKEY, OUTPUT);

  digitalWrite(MBED_CONF_IOTSHIELD_CATM1_RESET, HIGH);
  digitalWrite(MBED_CONF_IOTSHIELD_CATM1_PWRKEY, HIGH);
  delay(300);
  digitalWrite(MBED_CONF_IOTSHIELD_CATM1_RESET, LOW);
  digitalWrite(MBED_CONF_IOTSHIELD_CATM1_PWRKEY, LOW);
  delay(400);
  digitalWrite(MBED_CONF_IOTSHIELD_CATM1_RESET, HIGH);
  delay(10000);
  
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
  while(1)
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

  else if ( m_parser.send(F("AT$$STAT?")) &&
              m_parser.recv(F("$$STAT:%[^,],%[^\n]\n"), usim_stat, detail) &&
              m_parser.recv(F(RESP_OK)) ) {
    sprintf((char *)buf, "USIM Status: %s, %s", usim_stat, detail);
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
// Functions: TCP/UDP socket service
// ----------------------------------------------------------------

int8_t sockOpenConnect_WMN400MSE(int protocol, const char *addr, int port, int pckttype)
{
  int8_t ret = RET_NOK;
  int ok = 0; 
  int id = 0;

  bool done = false;
  //Timer t;

  m_parser.set_timeout(WMN400MSE_CONNECT_TIMEOUT);

    if((protocol != 0) && (protocol != 1))  // 0 : TCP, 1 : UDP
    {        
        return RET_NOK;
    }

  //t.start();
  lastConnectedTime = millis();
  m_parser.send("AT+WSOCR=%d,%s,%d,%d,%d", id, addr, port, protocol, pckttype);
  do {
    done = (m_parser.recv("+WSOCR:%d,%d", &ok,&id) && (ok == 1));
  } while (!done && (millis() - lastConnectedTime) < WMN400MSE_CONNECT_TIMEOUT);

  if (done) {
    if(m_parser.send("AT+WSOCO=%d", id) && m_parser.recv("+WSOCO:%d,%d,OPEN_WAIT", &ok, &id) && m_parser.recv("OK")){
      if (ok == 1) {
        if (m_parser.recv("+WSOCO:%d,OPEN_CMPL", &id)){
          ret = RET_OK;
        }
      }
    }
  }
  m_parser.set_timeout(WMN400MSE_DEFAULT_TIMEOUT);
  m_parser.flush();

  return ret;
}

int8_t sockClose_WMN400MSE(void)
{
  int8_t ret = RET_NOK;
  int id = 0;
  int ok = 0;
  m_parser.set_timeout(WMN400MSE_CONNECT_TIMEOUT);

  if (m_parser.send("AT+WSOCL=%d", id) && m_parser.recv("+WSOCL:%d,%d,CLOSE_WAIT", &ok, &id) && m_parser.recv("OK")) {
    if (ok == 1) ret = RET_OK;
  }
  m_parser.set_timeout(WMN400MSE_DEFAULT_TIMEOUT);

  return ret;
}

int8_t sendData_WMN400MSE(char * data, int len)
{
  int8_t ret = RET_NOK;
  int id = 0;
  int ok = 0;

  m_parser.set_timeout(WMN400MSE_SEND_TIMEOUT);
  m_parser.send("AT+WSOWR=%d,%d,%s", id, len, data);
  
  if ( m_parser.recv("+WSOWR:%d,%d", &ok, &id ) && m_parser.recv("OK")){
    if (ok == 1) { ret = RET_OK; }
  }

  m_parser.set_timeout(WMN400MSE_DEFAULT_TIMEOUT);

  return ret;
}

int8_t recvData_WMN400MSE(char * data, int * len)
{
  int8_t ret = RET_NOK;
  int id = 0;
  int i = 0;
  char _buf[100] = {0, };

  m_parser.set_timeout(WMN400MSE_RECV_TIMEOUT);
  lastConnectedTime = millis();
  
  while((millis() - lastConnectedTime) < WMN400MSE_RECV_TIMEOUT){
    _buf[i] = m_parser.p_getc();
    i++;
  }
  
  if(sscanf(_buf,"+WSORD:%d,%d,%[^\n]", &id, len, data))
  {
    ret = RET_OK;
  }
  
  m_parser.set_timeout(WMN400MSE_DEFAULT_TIMEOUT);
  m_parser.flush();

  return ret;
}
