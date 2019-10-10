#include "Arduino.h"
#include "at_cmd_parser.h"
#include <SimpleTimer.h>

#define RESP_OK                            "OK\r\n"
#define RET_OK                             1
#define RET_NOK                            -1
#define DEBUG_ENABLE                       1
#define DEBUG_DISABLE                      0

#define AMM592_APN_PROTOCOL_IPv4             1
#define AMM592_APN_PROTOCOL_IPv6             2
#define AMM592_DEFAULT_TIMEOUT               1000

#define AMM592_APN_PROTOCOL                  AMM592_APN_PROTOCOL_IPv6
#define AMM592_DEFAULT_BAUD_RATE       115200
#define AMM592_PARSER_DELIMITER              "\r\n"

#define CATM1_APN_SKT                      "lte-internet.sktelecom.com"

#define CATM1_DEVICE_NAME_AMM592             "AMM592"
#define DEVNAME                             CATM1_DEVICE_NAME_AMM592

#define MBED_CONF_IOTSHIELD_CATM1_RESET     7
#define MBED_CONF_IOTSHIELD_CATM1_PWRKEY    9

#define LOGDEBUG(x)                        if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { Serial.print("["); Serial.print(F(DEVNAME)); Serial.print("] ");  Serial.println(x); }
#define MYPRINTF(x)                        { Serial.print("[MAIN] "); Serial.println(x); }

// Sensors
#define MBED_CONF_IOTSHIELD_SENSOR_CDS     A0
#define MBED_CONF_IOTSHIELD_SENSOR_TEMP    A1

// Debug message settings
#define AMM592_PARSER_DEBUG                  DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG                 DEBUG_ENABLE

ATCmdParser m_parser = ATCmdParser(&Serial3);

String rxdata;
int timeCount = 0;
const char* chr;
SimpleTimer timer;

void setup() {
  char buf[30];
  char buf1[40];

  serialPcInit();
  catm1DeviceInit();


  MYPRINTF("Waiting for Cat.M1 Module Ready...\r\n");
  catm1Device_reset();
  waitCatM1Ready();

  MYPRINTF("System Init Complete\r\n");

  MYPRINTF("WIZnet IoT Shield for Arduino");
  MYPRINTF("LTE Cat.M1 Version");
  MYPRINTF("=================================================");
  MYPRINTF(">> Target Board: WIoT-AM01 (AM Telecom AMM592)");
  MYPRINTF(">> Sample Code: PSM Test");
  MYPRINTF("=================================================\r\n");

  while (1)
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

  checknSetApn_AMM592(CATM1_APN_SKT);

  //setContextActivate_AMM592();

 // setPSMDeactivate_AMM592();
 // catm1Device_reset();
  
  setPSMActivate_AMM592(60,300);
  catm1DeviceReset_AMM592();
  waitCatM1Ready();

  timer.setInterval(1000, printTimerInfo_AMM592);
}

void loop() {
char nettime[30];
 if (Serial3.available())
  {
    rxdata = Serial3.readString();
    //Serial.println(rxdata);
    chr = rxdata.c_str();
    if ( strstr(chr, "@NETSTI:3") != NULL )
    {
      timeCount = 0;
      MYPRINTF("Active Start");
    }
    
    if (getNetworkTimeLocal_AMM592(nettime) != RET_OK)
    {
      timeCount = 0;
      MYPRINTF("Sleep Start");
    } 
    


  }
  timer.run();
}




void serialPcInit(void)
{
  Serial.begin(115200);
}

void serialDeviceInit()
{
  Serial3.begin(AMM592_DEFAULT_BAUD_RATE);
}

void serialAtParserInit()
{
  m_parser.set_timeout(1000);
  m_parser.set_delimiter("\r\n");
}
void catm1DeviceInit()
{
  serialDeviceInit();
  serialAtParserInit();
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
  digitalWrite(MBED_CONF_IOTSHIELD_CATM1_PWRKEY, HIGH);
  delay(1000);
  
}

// ----------------------------------------------------------------
// Functions: Cat.M1 Status
// ----------------------------------------------------------------

int8_t waitCatM1Ready()
{
  bool initOK = false;
  while (1)
  {
   if (m_parser.recv("@NOTI:34,AMT_BOOT_ALERT")/* && m_parser.recv("@NETSTI:3")*/)
   {
      MYPRINTF("AMM592 ready\r\n");
      initOK = true;
      return RET_OK;
   }
/*  else if (m_parser.send(F("AT")) && m_parser.recv(F(RESP_OK)))
    {
      MYPRINTF("AMM592 already available\r\n");
      initOK = true;
      return RET_OK;
    }*/
  }
  if (!initOK) {
    Serial.println("[WM-AM01] AT Command: Not Available");
    return RET_NOK;
  }
}


bool initStatus()
{

  if ( setEchoStatus_AMM592(false) != RET_OK )
  {
    return false;
  }

  if ( getUsimStatus_AMM592() != RET_OK )
  {
    return false;
  }

  if ( getNetworkStatus_AMM592() != RET_OK )
  {
    return false;
  }

  return true;
}

int8_t setEchoStatus_AMM592(bool onoff)
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



int8_t getUsimStatus_AMM592(void)
{
  char usim_stat[10], detail[10];
  char buf[40];

  if ( m_parser.send(F("AT+CPIN?")) &&
       m_parser.recv(F("+CPIN: READY\n")) &&
       m_parser.recv(F(RESP_OK)) ) {
    LOGDEBUG("USIM Status: READY\r\n");
    return RET_OK;
  }

  else if ( m_parser.send(F("AT+CPIN?")) &&
            m_parser.recv(F("+CPIN: %[^,],%[^\n]\n"), usim_stat, detail) &&
            m_parser.recv(F(RESP_OK)) ) {
    sprintf((char *)buf, "USIM Satatus: %s, %s", usim_stat, detail);
    LOGDEBUG(buf);
    return RET_NOK;
  }
}

int8_t checknSetApn_AMM592(const char * apn) // Configure Parameters of a TCP/IP Context
{
    bool cgdccont = false, cgatt = false;
    int8_t ret = RET_NOK;
    
    LOGDEBUG("Checking APN...\r\n");
    //APN setting
    if (m_parser.send("AT+CGDCONT=1,\"IPV4V6\",\"%s\"", apn) && m_parser.recv("OK")){
        cgdccont = true;
    }
    //Attach setting
    if (m_parser.send("AT+CGATT?") && m_parser.recv("+CGATT: 1") &&m_parser.recv("OK")){
        cgatt = true;
    }

    if (cgdccont && cgatt){
        ret = RET_OK;
    }

    LOGDEBUG("APN Check Done\r\n");
        
    return ret;
}

int8_t getNetworkStatus_AMM592(void)
{
  char mode[10], stat[10];
  char buf[10];
  bool cereg = false, nsi = false;
  if ( m_parser.send(F("AT+CEREG?")) &&
       m_parser.recv(F("+CEREG: %[^,],%[^\n]\n"), mode, stat) &&
       m_parser.recv(F(RESP_OK)) ) {

    if ( (atoi(mode) == 0 || atoi(mode) == 1) && (atoi(stat) == 1) ) {
      cereg = true;
    }

    else if (( atoi(stat) != 1 )) {
      sprintf((char *)buf, "Network Status: %d, %d", atoi(mode), atoi(stat));
      LOGDEBUG(buf);
      cereg = false;
    }
  }

  if (m_parser.send("AT@NSI") && m_parser.recv("@NSI:%d, \"IN SRV\"") && m_parser.recv(F(RESP_OK))) {
    nsi = true;
  }
  if (cereg && nsi) 
  {  
    LOGDEBUG("Network Status: Attached\r\n"); 
    return RET_OK;
  } 
  else
  {
    LOGDEBUG("Network Status: Not Attached\r\n"); 
    return RET_NOK; 
  }
}


// ----------------------------------------------------------------
// Functions: Cat.M1 DNS
// ----------------------------------------------------------------

int8_t getIpAddressByName_AMM592(const char * name, char * ipstr)
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
// Functions: Cat.M1 PSM activate / deactivate
// ----------------------------------------------------------------

void setPSMActivate_AMM592(int Requested_Periodic_TAU, int Requested_Active_Time)
{
  if ( (m_parser.send(F("AT@PSMSET=1,%d,%d"), Requested_Periodic_TAU, Requested_Active_Time)
        && m_parser.recv(F(RESP_OK))) ) {
    LOGDEBUG("PSM Activate: Success\r\n");
  }
}

void setPSMDeactivate_AMM592()
{
  if ( (m_parser.send(F("AT@PSMSET=0"))
        && m_parser.recv(F(RESP_OK))) ) {
    LOGDEBUG("PSM Deactivate: Success\r\n");
  }
}

int8_t getPsmSetting_AMM592(bool * enable, int * Requested_Periodic_TAU, int * Requested_Active_Time)
{
  int8_t ret = RET_NOK;
  int en = 0;

  if (m_parser.send("AT@PSMSET?") // AMM592 only
      && m_parser.recv("@PSMSET:%d,%d,%d", &en, Requested_Periodic_TAU, Requested_Active_Time)
      && m_parser.recv("OK"))
  {
    if (en != 0)
      *enable = true;
    else
      *enable = false;

    LOGDEBUG("Get PSM setting success\r\n");
    ret = RET_OK;
  }
  return ret;
}


// ----------------------------------------------------------------
// Functions: Cat.M1 Network time
// ----------------------------------------------------------------

int8_t getNetworkTimeLocal_AMM592(char * timestr)
{
  int8_t ret = RET_NOK;
  if (m_parser.send("AT+CCLK?")
      && m_parser.recv("+CCLK: \"%[^\"]\"", timestr)
      && m_parser.recv("OK"))
  {
    //LOGDEBUG("Get current local time success\r\n");
    ret = RET_OK;
  }
  return ret;
}

void printTimerInfo_AMM592(void)
{
  char buf[20];
  timeCount ++;
  sprintf((char *)buf, "%d sec", timeCount);
  MYPRINTF(buf);
}
// ----------------------------------------------------------------
// Functions: Cat.M1 module reset
// ----------------------------------------------------------------

void catm1DeviceReset_AMM592(void)
{
  if ( (m_parser.send(F("AT@HWRESET"))
        && m_parser.recv(F(RESP_OK))) ) {
    LOGDEBUG("Reset: Success\r\n");
  }
}



