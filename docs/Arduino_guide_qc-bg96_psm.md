# Arduino 기반의 Cat.M1 PSM 테스트 가이드

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [AT 명령어](#Step-2-ATCommand)
-   [동작 구조 예제](#Step-3-SampleCode)
-   [예제 코드 빌드 및 실행](#Step-4-Build-and-Run)

<a name="Prerequisites"></a>
## 시작하기 전에

> * 하드웨어 설정과 개발환경 구축은 **[Arduino 기반으로 Cat.M1 디바이스 개발 시작하기][arduino-getting-started]** 문서에 상세히 설명되어 있습니다.

> * Cat.M1과 같은 Cellular IoT 디바이스는 통신 서비스 사업자의 운영 기준 및 규정에 따라 모듈 펌웨어 및 동작 방식에 차이가 있을 수 있습니다. 본 문서는 한국 **[SK Telecom Cat.M1 서비스][skt-iot-portal]** 를 기준으로 작성되었습니다.


### Development Environment
* **[Arduino IDE][link-arduino-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [Arduino Mega2560 Rev3][link-arduino Mega2560 Rev3] | WIoT-QC01 (BG96) |

<a name="Step-1-Overview"></a>
## 소개

IoT 응용에서 배터리로 동작을 하는 디바이스의 경우, 디바이스가 소모하는 전류는 디바이스 수명과 직접적인 관계가 있으므로 소모 전류를 최소화 하는 것은 매우 중요합니다. 이와 같은 이유로 Cat.M1은 PSM과 Enhanced DRX(이하 eDRX)기술을 지원하여 소모하는 전류를 크게 줄일 수 있도록 합니다. PSM(Power Saving Mode)을 사용하면 디바이스의 Active/Sleep 상태를 조절하여 소모하는 전력을 줄일 수 있습니다

본 문서에서는 Arduino IDE 기반 개발 환경에서 WIZnet IoT shield와 Arduino Mega2560 Rev3 보드를 이용하여 Cat.M1 단말의 PSM 설정 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 PSM 설정 위한 통신 과정은 다음과 같은 순서로 구현합니다.

1. PSM 설정
2. PSM 설정 확인
3. 모듈 및 모뎀 Reboot

<a name="Step-2-ATCommand"></a>
## AT 명령어

> AT 명령어에 대한 좀 더 상세한 설명은 Quectel AT Command Manual에서 확인 하실 수 있습니다.
> * [Quectel_BG96_PSM_Application_Note_V1.0][link-bg96-psm-an]
> * [BG96_AT_Commands_Manual_V2.1][link-bg96-atcommand-manual]

### 1. PSM 설정

PSM을 사용하기 위해 **AT+CPSMS** 명령어를 사용하며, 파라미터 설정을 통해 Active/Sleep 주기를 조절할 수 있습니다.
해당 명령의(Requested_Periodic-TAU)와 (Requested_Active-Time)의 시간을 설정함으로써 Active/Sleep 주기를 조절합니다. 주기의 단위를 설정하기 위해 6-8bits를 설정하며 값을 설정하기 위해 1-5bits값을 설정합니다. 예를 들어 (Requested_Periodic-TAU)를 1010011로 설정하였다면, 6-8bits가 101이므로 단위는 1분이며 1-2bit자리만 설정되었으므로 값은 6이고, 따라서 6분으로 설정한 것입니다.

**AT Command:** AT+CPSMS

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+CPSMS=[<Mode>[,(Requested_Periodic-RAU)[,(Requested_GPRS-READY-timer)[,(Requested_Periodic-TAU)[,(Requested_ActiveTime)]]]]] | OK<br><br>or<br><br>ERROR | AT+CPSMS=1,,,”10001010”,”00100100”<br>OK |
| Read | AT+CPSMS? | +CPSMS: (mode),[(Requested_Periodic-RAU)],[(Requested_GPRS-READY-timer)],[(Requested_Periodic-TAU)],[(Requested_Active-Time)]<br><br>OK | AT+CPSMS?<br>+CPSMS: 1,,,"10001010","00100100"<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (Mode) | Integer | 0 : PSM 해제<br>1 : PSM 설정<br>2 : PSM 해제 및 설정 초기화|
| (Requested_Periodic-RAU) | Integer | 000□□□□□ : (6-8 bits: 10분)   \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 10시간)  \* (1-5 bits: 이진수 값)<br>011□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>100□□□□□ : (6-8 bits: 30초)    \* (1-5 bits: 이진수 값)<br>101□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>110□□□□□ : (6-8 bits: 320시간) \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |
| (Requested_GPRS-READY-time) | Integer | 000□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |
| (Requested_Periodic-TAU) | Integer | 000□□□□□ : (6-8 bits: 10분)   \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 10시간)  \* (1-5 bits: 이진수 값)<br>011□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>100□□□□□ : (6-8 bits: 30초)    \* (1-5 bits: 이진수 값)<br>101□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>110□□□□□ : (6-8 bits: 320시간) \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |
| (Requested_Active-Time) | Integer | 000□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |

### 2. 확장 PSM 설정

AT+QPSMS 명령어는 AT+CPSMS와 거의 유사한 설정 및 파라메터를 제공합니다. 차이점은 Read 명령어 사용 시 설정 입력 값을 통해 Cat.M1 모듈에 설정된 TAU, Active time 등을 초 단위로 변환하여 사용자에게 제공 한다는 점입니다.

예제에서 Cat.M1 모듈에 설정된 PSM 시간 정보를 읽을 때 확장 PSM 설정 명령어를 활용합니다.

**AT Command:** AT+QPSMS

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+CPSMS와 동일 |  |  |
| Read | AT+CPSMS? | +CPSMS: (mode),[(Requested_Periodic-RAU)],[(Requested_GPRS-READY-timer)],[(Requested_Periodic-TAU)],[(Requested_Active-Time)]<br><br>OK | AT+QPSMS?<br>+QPSMS: 1,,,"600","240"<br><br>OK |

### 3. 최신 동기화 네트워크 시간 정보 획득

AT+QLTS 명령어는 모듈의 최근에 동기화 된 네트워크 시간 정보를 제공합니다. 더불어 동기화 시간을 기반으로 계산된 현재 GMT 시간과 현지 Local 시간 정보도 함께 제공하는 명령어입니다.

예제에서 PSM 시간의 측정 시간 정보 출력을 위해 네트워크 시간 정보 획득 명령어를 활용합니다.

**AT Command:** AT+QLTS

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+QLTS=(mode) | +QLTS: (time),(ds)<br><br>OK<br><br>ME 기능 관련 에러 발생 시,<br>+CME ERROR: (err)<br><br>그 외 에러 발생 시,<br><br>ERROR| AT+QLTS=2<br>+QLTS: "2019/03/21,16:11:59+36,0"<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (Mode) | Integer | 네트워크 시간 확인 모드 선택<br>0 : 가장 최근에 네트워크와 동기화 된 시간 정보<br>1 : 가장 최근 동기화 시간을 기반으로 계산된 현재 GMT 시간<br>2 : 가장 최근 동기화 시간을 기반으로 계산된 현재 현지(Local) 시간 |
| (time) | String | Format "yy/MM/dd,hh:mm:ss±zz" |
| (ds) | String | Daylight saving time |


<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

WIZnet IoT Shield를 Standalone 모드로 동작시켜, 터미널 프로그램으로 직접 AT 명령어를 입력해 보면서 동작 순서를 확인합니다.

```
// PSM 설정 (TAU time:300s, Active time:240s)
AT+CPSMS=1,,,"10000101","00100100"

OK

// PSM 설정 확인
AT+CPSMS?
+CPSMS: 1,,,"10010101","00100100"

OK

// PSM 설정 확인 (확장 명령어)
AT+QPSMS?
+QPSMS: 1,,,"300","240"

OK

// 설정 적용을 위한 모듈 Reboot


```

<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project
다음 링크에서 Arduino 예제 코드를 다운로드한 후, ino 확장자의 프로젝트 파일을 실행 시킵니다.

> 예제에서 활용할 Ping test sample code는 저장소의 아래 경로에 위치하고 있습니다.
> * `\samples\WIoT-QC01_BG96\WIoT-QC01_Arduino_PSM\`


### 2. Modify parameters

PSM 예제는 별도로 수정할 부분이 없습니다.

> [주의] TAU 시간은 SKTelecom 기준으로 최소 5분까지만 지원됩니다. 5분 이하의 시간을 입력할 경우 SKTelecom 망에서 지원하지 않아 PSM 진입 후 Booting하지 않을 수 있습니다.

### 3. Compile

상단 메뉴의 Compile 버튼을 클릭합니다.

![][compile1]

컴파일이 완료 되면 다음과 같이 업로드를 수행하여 최종적으로 보드에 업로드를 수행 합니다.
업로드가 정상적으로 완료되면 'avrdude done. Thank you.' 메시지를 확인 할 수 있습니다.

![][compile2]

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 PSM 설정 방법과 활용 방법에 대해 파악할 수 있습니다.

> MQTT 프로토콜은 서비스를 제공하는 서버를 MQTT broker, 단말을 MQTT client로 표현하며, 약속된 Topic에 메시지를 발행하는 Publish와 미리 지정한 Topic으로부터 메시지가 발행되면 수신하는 Subscribe로 구성되어 있습니다.

#### 4.1 Connect your board
스타터 키트와 Arduino Mega2560과 Uart 통신을 하기위해서는 아래와 같이 점퍼 연결이 필요합니다.
예제 구동을 위해 WIZnet IoT Shield의 UART TXD와 RXD 핀을 Arduino Mega2560 보드의 'Serial 3' `TX3`(14), `RX3`(15) 에 연결합니다.

| ArduinoMega2560 | TX3 (14)  | RX3 (15) |
|:----:|:----:|:----:|
| WIZnet IoT Shield | RXD<br>(UART Rx for D1/D8)  | TXD<br>(UART Tx for D0/D2) |

> 보드 상단에 위치한 UART_SEL 점퍼를 제거한 후 (실크 기준) 오른쪽 핀을 Arduino 보드와 연결합니다.

![][hw-stack]

#### 4.2 Functions

```cpp
int8_t setPsmActivate_BG96(char * Requested_Periodic_TAU, char * Requested_Active_Time);
```
* PSM TAU와 Active time의 설정 및 기능 활성화를 수행합니다.
* 파라메터는 "10010101", "00100100" 형태의 문자열입니다.

```cpp
int8_t setPsmDeactivate_BG96(void);
```
* PSM 기능을 비활성화 하는 함수입니다.

```cpp
int8_t getPsmSetting_BG96(bool * enable, int * Requested_Periodic_TAU, int * Requested_Active_Time);
```
* PSM 설정 정보를 확인합니다. 설정 정보를 저장할 변수들을 파라메터로 받아 활성화 여부 및 TAU, Active time을 돌려줍니다.
* 확장 PSM 설정 명령어 기반으로 구현되어 시간 정보들을 초 단위로 리턴합니다.


샘플 코드의 네트워크 시간 정보 관련 함수는 다음과 같습니다.

```cpp
int8_t getNetworkTimeGMT_BG96(char * timestr);
```
* 파라메터를 통해 네트워크 동기화 시간을 기반으로 계산된 GMT 시간을 문자열로 제공하는 함수입니다.

```cpp
int8_t getNetworkTimeLocal_BG96(char * timestr);
```
* 파라메터를 통해 네트워크 동기화 시간을 기반으로 계산된 Local 시간을 문자열로 제공하는 함수입니다.
* 한국의 경우 GMT+09가 적용됩니다.


#### 4.3 Set up serial monitor
![][serialMonitor]

#### 4.4 Demo
![][1]

..
...
....

![][2]

..
...
....

![][3]




[arduino-getting-started]: ./Arduino_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
[link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-psm-an]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_PSM_Application_Note_V1.0.pdf

[hw-stack]: ./imgs/hw/wiot-shield-wm01-arduinomega2560_stack.png 
[compile1]: ./imgs/arduino_guide_ide_compile.png
[compile2]: ./imgs/arduino_guide_ide_compile_finish.png
[serialMonitor]: ./imgs/arduino_guide_ide_serialmonitor.png

[1]: ./imgs/arduino_guide_bg96_psm-1.png
[2]: ./imgs/arduino_guide_bg96_psm-2.png
[3]: ./imgs/arduino_guide_bg96_psm-3.png
