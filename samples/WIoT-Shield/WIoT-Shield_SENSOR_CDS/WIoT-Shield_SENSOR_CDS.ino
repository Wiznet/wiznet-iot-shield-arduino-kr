#include "Arduino.h"

#define MYPRINTF(x)                        { Serial.print("[MAIN] "); Serial.println(x); }

// Sensors
#define IOTSHIELD_SENSOR_CDS               A0
#define IOTSHIELD_SENSOR_TEMP              A1
#define LEDPIN                             13

int val = 0;          // CDS 측정 값 넣을 변수

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
