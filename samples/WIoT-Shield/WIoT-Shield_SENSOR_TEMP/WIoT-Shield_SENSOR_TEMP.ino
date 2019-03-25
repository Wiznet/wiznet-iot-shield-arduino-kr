#include "Arduino.h"
#include "at_cmd_parser.h"

#define CATM1_DEVICE_NAME_BG96             "BG96"
#define DEVNAME                             CATM1_DEVICE_NAME_BG96

#define LOGDEBUG(x)                        if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { Serial.print("["); Serial.print(F(DEVNAME)); Serial.print("] ");  Serial.println(x); }
#define MYPRINTF(x)                        { Serial.print("[MAIN] "); Serial.println(x); }

// Sensors
#define IOTSHIELD_SENSOR_CDS               A0
#define IOTSHIELD_SENSOR_TEMP              A1

// Debug message settings
#define BG96_PARSER_DEBUG                  DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG                 DEBUG_ENABLE

int val = 0;          // 온도 측정 값 넣을 변수
float voltage = 0;    // 전압 계산 값 넣을 변수
float celsius = 0;    // 섭씨 계산 값 넣을 변수
float fahrenheit = 0; // 화씨 계산 값 넣을 변수

void setup() {
  serialPcInit();

  MYPRINTF("WIZnet IoT Shield for Arduino");
  MYPRINTF("LTE Cat.M1 Version");
  MYPRINTF("=================================================");
  MYPRINTF(">> Sample Code: TEMP Test");
  MYPRINTF("=================================================\r\n");
}

void loop() {
  // read the value from the sensor:
  val = analogRead(IOTSHIELD_SENSOR_TEMP);
  // 실제 전압 계산
  // 0~5V의 값을 구하기 위해 아래와 같이 5를 곱하고 1024를 나눈다.
  // (Analog값 * IC에 걸리는 전압 / Analog 10bit 분해능(1024))
  // IC에 걸리는 전압은 측정기로 측정

  voltage = val * 4.87 / 1024.0;
  celsius = getTemperature_C(voltage);
  fahrenheit = getTemperature_F(celsius);

  delay(1000);
  Serial.println("[ Temperature Data ]");
  Serial.print("Sensor value :");
  Serial.println(val);
  Serial.print("Volt : ");
  Serial.print(voltage);
  Serial.println("mV");
  Serial.print("Celsius temp : ");
  Serial.print(celsius);
  Serial.println("C");
  Serial.print("Fahrenheit temp : ");
  Serial.print(fahrenheit);
  Serial.println("F");
  delay(1000);
}


void serialPcInit(void)
{
  Serial.begin(115200);
}

// ----------------------------------------------------------------
// Functions: Get Temperature
// ----------------------------------------------------------------

float getTemperature_F(float _celsius)
{
  // 섭씨에서 화씨 환산 -> (화씨온도 = 섭씽온*9/5+32)
  fahrenheit = celsius * 9.0 / 5.0 + 32.0;
  return fahrenheit;
}

float getTemperature_C(float _voltage)
{
  // LM35는 섭씨 1도당 10mV의 전위차를 갖는다.
  // 센서핀의 전압이 0.28V라면 280mV이므로 온도는 28도씨.
  // 100을 곱해서 섭씨 온도로 나타냄
  celsius = voltage * 100.0;
  return celsius;
}
