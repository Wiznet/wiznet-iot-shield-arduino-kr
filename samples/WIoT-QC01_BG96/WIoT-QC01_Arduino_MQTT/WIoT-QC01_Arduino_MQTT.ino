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
#define MYPRINTF(x)                        { Serial.print("[MAIN] "); Serial.println(x); }

// Sensors
#define MBED_CONF_IOTSHIELD_SENSOR_CDS     A0
#define MBED_CONF_IOTSHIELD_SENSOR_TEMP    A1

// Debug message settings
#define BG96_PARSER_DEBUG                  DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG                 DEBUG_ENABLE

/* MQTT */
#define MQTT_EOF                           0x1A

// Connection info:
// HiveMQ Websocket client(http://www.hivemq.com/demos/websocket-client/)
#define MQTT_CLIENTID                     "clientExample"
#define MQTT_USERID                       "testid"
#define MQTT_PASSWORD                     "testpassword"
#define MQTT_QOS0                         0
#define MQTT_QOS1                         1
#define MQTT_QOS2                         2
#define MQTT_RETAIN                       0

#define MQTT_SAMPLE_TOPIC_A               "topic/openhouse-A"
#define MQTT_SAMPLE_TOPIC_B               "topic/openhouse-B"
#define MQTT_SAMPLE_TOPIC_C               "topic/openhouse-C"
#define MQTT_SAMPLE_TOPIC_D               "topic/openhouse-D"

/* MQTT Sample */
// MQTT connection state
enum {
  MQTT_STATE_OPEN = 0,
  MQTT_STATE_CONNECT,
  MQTT_STATE_CONNECTED,
  MQTT_STATE_DISCON
};

char mqtt_broker_url[] = "broker.hivemq.com";
int mqtt_broker_port = 1883;
int mqtt_socket_id = 0;

ATCmdParser m_parser = ATCmdParser(&Serial3);

void setup() {
  char buf[100];

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
  MYPRINTF(">> Sample Code: MQTT Test");
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
  int mqtt_state = MQTT_STATE_OPEN;

  // MQTT Connect phase
  //----------------------------------------
  sprintf((char *)buf, "[MQTT] Try to connect to broker \"%s:%d\"\r\n", mqtt_broker_url, mqtt_broker_port);
  MYPRINTF(buf);


  do {
    switch (mqtt_state) {
      case MQTT_STATE_OPEN:
        if (openMqttBroker_BG96(mqtt_broker_url, mqtt_broker_port) == RET_OK) {
          MYPRINTF("[MQTT] Socket open success\r\n");
          mqtt_state = MQTT_STATE_CONNECT;
        } else {
          MYPRINTF("[MQTT] Socket open failed\r\n");
        }
        break;

      case MQTT_STATE_CONNECT:
        if (connectMqttBroker_BG96(MQTT_CLIENTID, MQTT_USERID, MQTT_PASSWORD) == RET_OK) {
          mqtt_state = MQTT_STATE_CONNECTED;
        } else {
          MYPRINTF("[MQTT] Connect failed\r\n");
          mqtt_state = MQTT_STATE_DISCON;
        }
        break;

      case MQTT_STATE_DISCON:
        if (closeMqttBroker_BG96() == RET_OK) {
          MYPRINTF("[MQTT] Disconnected\r\n");
        }
        mqtt_state = MQTT_STATE_OPEN;
        break;

      default:
        mqtt_state = MQTT_STATE_OPEN;
        break;
    }
  } while (mqtt_state != MQTT_STATE_CONNECTED);

  // MQTT Publish & Subscribe phase
  //----------------------------------------
  if (mqtt_state == MQTT_STATE_CONNECTED) {
    char mqtt_message_b[] = "Hello IoT Open House B";
    char mqtt_message_c[] = "Hello IoT Open House C";

    // MQTT Subscribe
    if (setMqttSubscribeTopic_BG96(MQTT_SAMPLE_TOPIC_A, 1, MQTT_QOS0) == RET_OK) {
      dumpMqttSubscribeTopic_BG96(MQTT_SAMPLE_TOPIC_A, 1, MQTT_QOS0);
    }
#if 0
    if (setMqttSubscribeTopic_BG96(MQTT_SAMPLE_TOPIC_D, 2, MQTT_QOS2) == RET_OK) {
      dumpMqttSubscribeTopic_BG96(MQTT_SAMPLE_TOPIC_D, 2, MQTT_QOS0);
    }
#endif
    // MQTT Publish test
    if (sendMqttPublishMessage_BG96(MQTT_SAMPLE_TOPIC_B, MQTT_QOS1, MQTT_RETAIN, mqtt_message_b, strlen(mqtt_message_b)) == RET_OK) {
      dumpMqttPublishMessage_BG96(MQTT_SAMPLE_TOPIC_B, mqtt_message_b);

    }

    if (sendMqttPublishMessage_BG96(MQTT_SAMPLE_TOPIC_C, MQTT_QOS1, MQTT_RETAIN, mqtt_message_c, strlen(mqtt_message_c)) == RET_OK) {
      dumpMqttPublishMessage_BG96(MQTT_SAMPLE_TOPIC_C, mqtt_message_c);
    }
  }

  char buf_mqtt_topic[100] = {0, };
  char buf_mqtt_msg[200] = {0, };
  int mqtt_msgid = 0;

  // MQTT Subscribe test
  while (1)
  {
    // MQTT message received
    if (checkRecvMqttMessage_BG96(buf_mqtt_topic, &mqtt_msgid, buf_mqtt_msg) == RET_OK) {
      sprintf((char *)buf, "[MQTT] Subscribed Topic: \"%s\" ID: %d, Message: \"%s\"\r\n", buf_mqtt_topic, mqtt_msgid, buf_mqtt_msg);
      MYPRINTF(buf);
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
// Functions: Cat.M1 MQTT Publish & Subscribe
// ----------------------------------------------------------------

int8_t openMqttBroker_BG96(char * url, int port)
{
  int8_t ret = RET_NOK;
  int id = 0;
  int result = 0;
  unsigned long lastOpenedTime = 0;         // last time you connected to the server, in milliseconds
  bool done = false;
  //Timer t;

  //t.start();
  lastOpenedTime = millis();

  if (m_parser.send("AT+QMTOPEN=%d,\"%s\",%d", id, url, port) && m_parser.recv("OK")) {
    do {
      done = (m_parser.recv("+QMTOPEN: %d,%d", &id, &result) && (result == 0));

      // MQTT Open: result code sample, refer to BG96_MQTT_Application_Note
      if (result == 1) {
        LOGDEBUG("AT+QMTOPEN result[1]: Wrong parameter");
      } else if (result == 2) {
        LOGDEBUG("AT+QMTOPEN result[2]: MQTT identifier is occupied");
      } else if (result == 3) {
        LOGDEBUG("AT+QMTOPEN result[3]: Failed to activate PDP");
      } else if (result == 4) {
        LOGDEBUG("AT+QMTOPEN result[4]: Failed to parse domain name");
      } else if (result == 5) {
        LOGDEBUG("AT+QMTOPEN result[5]: Network disconnection error");
      }
    } while (!done && (millis() - lastOpenedTime) < BG96_CONNECT_TIMEOUT);

    if (done) {
      ret = RET_OK;
    }
  }
  m_parser.flush();

  return ret;
}

int8_t connectMqttBroker_BG96(char * clientid, char * userid, char * password)
{
  int8_t ret = RET_NOK;
  int id = 0;
  int result = 0;
  int ret_code = 0;
  unsigned long lastConnectedTime = 0;         // last time you connected to the server, in milliseconds
  char buf[100];
  bool done = false;
  //Timer t;

  if ((userid != NULL) && (password != NULL)) {
    m_parser.send("AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"", id, clientid, userid, password);
  } else {
    m_parser.send("AT+QMTCONN=%d,\"%s\"", id, clientid);
  }

  //t.start();
  lastConnectedTime = millis();
  
  if (m_parser.recv("OK"))
  {
    do {
      done = (m_parser.recv("+QMTCONN: %d,%d,%d", &id, &result, &ret_code)
              && (result == 0) && (ret_code == 0));

      // MQTT Connect: result sample, refer to BG96_MQTT_Application_Note
      if (result == 1) {
        LOGDEBUG("AT+QMTCONN result[1]: Packet retransmission");
      } else if (result == 2) {
        LOGDEBUG("AT+QMTCONN result[2]: Failed to send packet");
      }

      // MQTT Connect: ret_code sample, refer to BG96_MQTT_Application_Note
      if (result == 1) {
        LOGDEBUG("AT+QMTCONN ret_code[1]: Connection Refused: Unacceptable Protocol Version");
      } else if (result == 2) {
        LOGDEBUG("AT+QMTCONN ret_code[2]: Connection Refused: Identifier Rejected");
      } else if (result == 3) {
        LOGDEBUG("AT+QMTCONN ret_code[3]: Connection Refused: Server Unavailable");
      } else if (result == 4) {
        LOGDEBUG("AT+QMTCONN ret_code[4]: Connection Refused: Bad User Name or Password");
      } else if (result == 5) {
        LOGDEBUG("AT+QMTCONN ret_code[5]: Connection Refused: Not Authorized");
      }
    } while (!done &&(millis() - lastConnectedTime) < BG96_CONNECT_TIMEOUT * 2);

    if (done) {
      ret = RET_OK;
    }
  }
  m_parser.flush();

  return ret;
}

int8_t closeMqttBroker_BG96(void)
{
  int8_t ret = RET_NOK;
  int id = 0;
  int result = 0;
  unsigned long lastClosedTime = 0;            // last time you connected to the server, in milliseconds

  bool done = false;
  //Timer t;

  //t.start();
  lastClosedTime = millis();
  
  if (m_parser.send("AT+QMTDISC=%d", id) && m_parser.recv("OK")) {
    do {
      done = (m_parser.recv("+QMTDISC: %d,%d", &id, &result));
    } while (!done && (millis() - lastClosedTime) < BG96_CONNECT_TIMEOUT * 2);

    if (done) {
      ret = RET_OK;
    }
  }
  m_parser.flush();

  return ret;
}


int8_t sendMqttPublishMessage_BG96(char * topic, int qos, int retain, char * msg, int len)
{
  int8_t ret = RET_NOK;
  int id = 0;
  int result = 0;
  int sent_msgid = 0;
  static int msgid = 0;
  unsigned long lastSentTime = 0;            // last time you connected to the server, in milliseconds
  char buf[100];
  
  bool done = false;
  //Timer t;

  if (qos != 0) {
    if (msgid < 0xffff)
      msgid++;
    else
      msgid = 0;
  }

  //t.start();
  lastSentTime = millis();
  
  m_parser.send("AT+QMTPUB=%d,%d,%d,%d,\"%s\"", id, qos ? msgid : 0, qos, retain, topic);

  if ( !done && m_parser.recv(">") )
    done = (m_parser.write(msg, len) <= 0) & m_parser.send("%c", MQTT_EOF);

  if (m_parser.recv("OK")) {
    do {
      done = (m_parser.recv("+QMTPUB: %d,%d,%d", &id, &sent_msgid, &result));
    } while (!done && (millis() - lastSentTime) < BG96_CONNECT_TIMEOUT * 2);

    if (done) {
      ret = RET_OK;
    }
  }
  m_parser.flush();

  return ret;
}

int8_t setMqttSubscribeTopic_BG96(char * topic, int msgid, int qos)
{
  int8_t ret = RET_NOK;
  int id = 0;
  int result = 0;

  int sent_msgid = 0;
  int qos_level = 0;
  unsigned long lastSetTime = 0;            // last time you connected to the server, in milliseconds

  bool done = false;
  //Timer t;

  m_parser.set_timeout(BG96_CONNECT_TIMEOUT);

  //t.start();
  lastSetTime = millis();
  
  if (m_parser.send("AT+QMTSUB=%d,%d,\"%s\",%d", id, msgid, topic, qos) && m_parser.recv("OK")) {
    do {
      done = (m_parser.recv("+QMTSUB: %d,%d,%d,%d", &id, &sent_msgid, &result, &qos_level));
    } while (!done && (millis() - lastSetTime) < BG96_CONNECT_TIMEOUT);

    if (done) {
      ret = RET_OK;
    }
  }
  m_parser.set_timeout(BG96_DEFAULT_TIMEOUT);
  m_parser.flush();

  return ret;
}

int8_t checkRecvMqttMessage_BG96(char * topic, int * msgid, char * msg)
{
  int8_t ret = RET_NOK;
  int id = 0;
  bool received = false;

  m_parser.set_timeout(1);
  received = m_parser.recv("+QMTRECV: %d,%d,\"%[^\"]\",\"%[^\"]\"", &id, msgid, topic, msg);
  m_parser.set_timeout(BG96_DEFAULT_TIMEOUT);

  if (received) ret = RET_OK;
  return ret;
}

void dumpMqttSubscribeTopic_BG96(char * topic, int msgid, int qos)
{
  char buf[100];
  sprintf((char *)buf, "[MQTT] Subscribe Topic: \"%s\", ID: %d, QoS: %d\r\n", topic, msgid, qos);
  MYPRINTF(buf);
}

void dumpMqttPublishMessage_BG96(char * topic, char * msg)
{
  char buf[100];
  sprintf((char *)buf, "[MQTT] Published Topic: \"%s\", Message: \"%s\"\r\n", topic, msg);
  MYPRINTF(buf);
}

