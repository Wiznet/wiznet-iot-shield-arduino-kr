#include "Arduino.h"
#include "at_cmd_parser.h"

#define RESP_OK                            "OK\r\n"
#define RET_OK                             1
#define RET_NOK                            -1
#define DEBUG_ENABLE                       1
#define DEBUG_DISABLE                      0
#define ON                                 1
#define OFF                                0

#define BG96_APN_PROTOCOL_IPv4             1
#define BG96_APN_PROTOCOL_IPv6             2
#define BG96_DEFAULT_TIMEOUT               1000
#define BG96_CONNECT_TIMEOUT               15000

#define BG96_APN_PROTOCOL                  BG96_APN_PROTOCOL_IPv6
#define WM_N400MSE_DEFAULT_BAUD_RATE       115200
#define BG96_PARSER_DELIMITER              "\r\n"

#define CATM1_APN_SKT                      "lte-internet.sktelecom.com"

#define CATM1_DEVICE_NAME_BG96             "BG96"
#define DEVNAME                             CATM1_DEVICE_NAME_BG96

#define LOGDEBUG(x)                        if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { Serial.print("["); Serial.print(F(DEVNAME)); Serial.print("] ");  Serial.println(x); }
#define MYPRINTF(x)                        { Serial.print("[MAIN] "); Serial.println(x); }

// Sensors
#define MBED_CONF_IOTSHIELD_SENSOR_CDS     A0
#define MBED_CONF_IOTSHIELD_SENSOR_TEMP    A1

// Debug message settings
#define BG96_PARSER_DEBUG                  DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG                 DEBUG_ENABLE

#define REQUESTED_PERIODIC_TAU            "10100101"
#define REQUESTED_ACTIVE_TIME             "00100100"

ATCmdParser m_parser = ATCmdParser(&Serial3);

unsigned long getLocationTime = 0;

char timeBuf[35];
char utcBuf[15];
char latBuf[10];
char lonBuf[10];

typedef struct gps_data_t {
  float utc;      // hhmmss.sss
  float lat;      // latitude. (-)dd.ddddd
  float lon;      // longitude. (-)dd.ddddd
  float hdop;     // Horizontal precision: 0.5-99.9
  float altitude; // altitude of antenna from sea level (meters)
  int fix;        // GNSS position mode 2=2D, 3=3D
  float cog;      // Course Over Ground ddd.mm
  float spkm;     // Speed over ground (Km/h) xxxx.x
  float spkn;     // Speed over ground (knots) xxxx.x
  char date[7];   // data: ddmmyy
  int nsat;       // number of satellites 0-12
} gps_data;

gps_data gps_info;

void setup() {

  char buf[30];
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
  MYPRINTF(">> Sample Code: PSM Test");
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

  

}

void loop() {


  if (setGpsOnOff_BG96(ON) == RET_OK) {
    MYPRINTF("GPS On\r\n")

    
    while (1) {
      if (getGpsLocation_BG96(&gps_info) == RET_OK) {
        MYPRINTF("Get GPS information >>>");
        sprintf((char *)utcBuf, "gps_info - utc: %6.3f", gps_info.utc);
        MYPRINTF(utcBuf)             // utc: hhmmss.sss
        sprintf((char *)latBuf, "gps_info - lat: %2.5f", gps_info.lat);
        MYPRINTF(latBuf)             // latitude: (-)dd.ddddd
        sprintf((char *)lonBuf, "gps_info - lon: %2.5f", gps_info.lon);
        MYPRINTF(lonBuf)             // longitude: (-)dd.ddddd
        //MYPRINTF("gps_info - hdop: %2.1f", gps_info.hdop)           // Horizontal precision: 0.5-99.9
        //MYPRINTF("gps_info - altitude: %2.1f", gps_info.altitude)   // altitude of antenna from sea level (meters)
        //MYPRINTF("gps_info - fix: %d", gps_info.fix)                // GNSS position mode: 2=2D, 3=3D
        //MYPRINTF("gps_info - cog: %3.2f", gps_info.cog)             // Course Over Ground: ddd.mm
        //MYPRINTF("gps_info - spkm: %4.1f", gps_info.spkm)           // Speed over ground (Km/h): xxxx.x
        //MYPRINTF("gps_info - spkn: %4.1f", gps_info.spkn)           // Speed over ground (knots): xxxx.x
        //MYPRINTF("gps_info - date: %s", gps_info.date)              // data: ddmmyy
        //MYPRINTF("gps_info - nsat: %d\r\n", gps_info.nsat)          // number of satellites: 0-12
      } else {
        MYPRINTF("Failed to get GPS information\r\n");
      }
      delay(1000);
    }
#if 0
    if (setGpsOnOff_BG96(OFF) == RET_OK) {
      MYPRINTF("GPS Off\r\n")
    }
#endif

  } else {
    MYPRINTF("GPS On failed\r\n")
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

  else if ( m_parser.send(F("AT+CPIN?")) &&
            m_parser.recv(F("+CPIN: %[^,],%[^\n]\n"), usim_stat, detail) &&
            m_parser.recv(F(RESP_OK)) ) {
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
// Functions: Cat.M1 GPS
// ----------------------------------------------------------------

int8_t setGpsOnOff_BG96(bool onoff)
{
  int8_t ret = RET_NOK;
  char _buf[15];
  char _buf1[30];
  char _buf2[30];

  sprintf((char *)_buf, "%s", onoff ? "AT+QGPS=2" : "AT+QGPSEND");

  if (m_parser.send(_buf) && m_parser.recv("OK")) {
    sprintf((char *)_buf1, "GPS Power: %s\r\n", onoff ? "On" : "Off");
    LOGDEBUG(_buf1);
    ret = RET_OK;
  } else {
    sprintf((char *)_buf2, "Set GPS Power %s failed\r\n", onoff ? "On" : "Off");
    LOGDEBUG(_buf2);
  }
  return ret;
}


int8_t getGpsLocation_BG96(gps_data *data)
{
  int8_t ret = RET_NOK;
  char _buf[100];

  bool ok = false;
  //Timer t;

  // Structure init: GPS info
  data->utc = data->lat = data->lon = data->hdop = data->altitude = data->cog = data->spkm = data->spkn = data->nsat = 0.0;
  data->fix = 0;
  memset(&data->date, 0x00, 7);

  // timer start
  //t.start();
  getLocationTime = millis();

  while ( !ok && ( (millis() - getLocationTime) < BG96_CONNECT_TIMEOUT ) ) {
    m_parser.flush();
    m_parser.send((char*)"AT+QGPSLOC=2"); // MS-based mode
    ok = m_parser.recv("+QGPSLOC: ");
    if (ok) {
      m_parser.recv("%s\r\n", _buf);
      sscanf(_buf, "%f,%f,%f,%f,%f,%d,%f,%f,%f,%6s,%d",
             data->utc, data->lat, data->lon, data->hdop,
             data->altitude, data->fix, data->cog,
             data->spkm, data->spkn, data->date, data->nsat);
      ok = m_parser.recv("OK");
    }
  }

  if (ok == true) ret = RET_OK;

  return ret;
}
