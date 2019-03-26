#include "Arduino.h"
#include "at_cmd_parser.h"

#define CATM1_DEVICE_NAME_BG96             "BG96"
#define DEVNAME                             CATM1_DEVICE_NAME_BG96

#define LOGDEBUG(x)                        if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { Serial.print("["); Serial.print(F(DEVNAME)); Serial.print("] ");  Serial.println(x); }
#define MYPRINTF(x)                        { Serial.print("[MAIN] "); Serial.println(x); }

// Sensors
#define IOTSHIELD_SENSOR_CDS               A0
#define IOTSHIELD_SENSOR_TEMP              A1
#define LEDPIN                             13

// Debug message settings
#define BG96_PARSER_DEBUG                  DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG                 DEBUG_ENABLE

int val = 0;          // 온도 측정 값 넣을 변수
float voltage = 0;    // 전압 계산 값 넣을 변수
float celsius = 0;    // 섭씨 계산 값 넣을 변수
float fahrenheit = 0; // 화씨 계산 값 넣을 변수

void setup() {
  serialPcInit();
  pinMode(LEDPIN, OUTPUT); // LED 출력으로 지정
  
  MYPRINTF("WIZnet IoT Shield for Arduino");
  MYPRINTF("LTE Cat.M1 Version");
  MYPRINTF("=================================================");
  MYPRINTF(">> Sample Code: CDS Test");
  MYPRINTF("=================================================\r\n");
}

void loop() {
  //map : 특정 범위에 속하는 값을 다른 범위의 값으로 변환해주는 명령어
  val = map(analogRead(IOTSHIELD_SENSOR_CDS), 0, 1023, 0, 256);
  //val = (analogRead(IOTSHIELD_SENSOR_CDS) * 5v); // 다른 계산 방법
  
  if(val > 128){ // 조도 저항 값이 낮기 때문에 아날로그 값이 높음
    digitalWrite(LEDPIN, LOW); 
  }
  else{ // 손가락으로 가리면 조도 저항값이 높아지기 때문에 아날로그 값이 낮아진다
    digitalWrite(LEDPIN, HIGH);
  }
}


void serialPcInit(void)
{
  Serial.begin(115200);
}
