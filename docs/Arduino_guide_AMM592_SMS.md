# Arduino 기반의 Cat.M1 SMS 데이터 통신 가이드

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
| [Arduino Mega2560 Rev3][link-arduino Mega2560 Rev3] | WIoT-(AMM592SK)<br>WIoT-AM01  |

<a name="Step-1-Overview"></a>
## 소개
본 문서에서는 Arduino IDE 기반 개발 환경에서 WIZnet IoT shield와 Arduino Mega2560 Rev3 보드를 이용하여 Cat.M1 단말의 SMS 데이터 송수신 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, TCP/IP 소켓 통신에 비해서 비교적 간단한 AT Command로 테스트를 할 수 있습니다.
이번 테스트에서는 일반적인 스마트폰을 이용하여, Cat M1 모듈의 SMS 송수신 예제를 설명하도록 하겠습니다.

<a name="Step-2-ATCommand"></a>
## AT 명령어

> AT 명령어 설명은 AMTelecom AT Command Manual에서 확인 하실 수 있습니다.


### 1. Cat M1 모듈의 일반적인 동작 설명은 생략

>  Cat M1 모듈의 에코 모드 설명, USIM 상태 확인, 네트워크 접속 확인 등의 일반적인 내용은 TCP 가이드를 참고하시기 바랍니다.

### 2. AM Telecom SMS 명령어 
AM Telecom 모듈에서 제공하는 SMS 명령이 많이 있지만, 본 장에서는 테스트에 사용된 명령어에 대해서만 설명합니다. 자세한 내용은 AM Telecom 매뉴얼을 참고하시기 바랍니다.

**AT Command:** AT+CMGS SMS 메시지 전송

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+CMGS=? | OK | - |
| Write | AT+CMGS=(da),[(toda)]| +CMGS:(mr),[(scts)] |  AT+CMGS="1502101xxxx",129<br>>This is a test from .... //보낼 SMS 데이터 입력 후<br>+CMGS:247<br>OK |

ps. 파라미터[(toda)]를 생략하고 사용하는 경우
- (da)의 첫번째 값이 (+)이면, International numbering scheme 로 설정 됩니다. 
- (da)의 첫번째 값이 (+)가 아닌 경우, National numbering scheme 로 설정 됩니다. 

**Defined values:**

| Parameter | 범위 | Description |
|:--------|:--------|:--------|
| (da) | 0 ~ 20 | SMS를 보낼 수신(상대방) 전화번호 |
| [(toda)] | 129, 145 | SMS 수신측 단말의 전화번호 타입<br>129:National numbering scheme<br>145:International numbering scheme("+"포함)|
| (mr) | 0 ~ 255 | SMS 참조번호. 모듈이 자동으로 설정|
|(scts) | 0 ~ 20 | SMS가 SMS 센터에 도착한 시간|



**AT Command:** AT+CMGL 문자 메시지 수신

**Syntax:**

| Type | Syntax | Response | Example|
|:--------|:--------|:--------|:--------|
| Read | AT+CMGL=(stat) | (PDU Mode)<br>+CMGL (index),(stat),(alpha),(length)<br>(pdu)<br>OK<br><br>(TEXTMode)<br>+CMGL (index),(stat),(oa/da),(alpha),(scts)[,(tooa/toda),(length)]<br>(data)<br>OK| (PDU Mode)<br>AT+CMGL=4<br>+CMGL 0,1,,33<br>079128019291900204BA11020197701F500005110023132406310D3E614442DCFE9A066793E0F9FCB<br>OK<br><br>(TEXT Mode)<br>AT+CMGL="ALL"<br>+CMGL 0,"REC READ","01029177105",,"15/01/20,13:23:04+36"<br>SMS Test Message<br>OK  |

**Defined values:**

| Parameter | 범위 |   Description |
|:--------|:--------|:--------|
| (stat) | (PDU Mode)<br> 0 ~ 4 <br><br>(TEXT Mode)<br>("REC UNREAD", "REC READ", "STO UNSENT","STO SENT","ALL")|(PDU Mode)<br> 0: 신규 SMS, 1: 읽은 SMS <br> 2: 전송되지 않은 SMS, 3: 전송된 SMS <br>4: 모든 SMS<br> (TEXT Mode)<br>"REC UNREAD" : 신규 SMS, "REC READ" : 읽은 SMS <br>"STO UNSENT" : 전송되지 않은 SMS, "STO SENT" : 전송된 SMS<br> "ALL" : 모든 SMS |
| (index) | 0 ~ 24 | 메모리에서 메시지의 위치 |
| (alpha) | 0 ~ 90 | Phonebook 항목에 해당하는 (da)/(oa)번호의 영수자 표현 |
| (length) | 0 ~ 255 | (PDU Mode)<br>PDU의 길이<br>(TEXT Mode)<br>문자의 길이 |
| (pdu) | 0 ~ 255 | PDU data (3 GPP TS23 040) |
| (oa/da) | 0 ~ 48 | 발신/착신 전화번호|
| (scts) | 0 ~ 20 | 시간 문자열 형식의 TP-Service Centre Time Stamp |
| (tooa/toda) | 129, 145 | 129 : National numbering scheme<br>145 : International numbering scheme("+"포함)|
| (data) | 0 ~ 255 | TP-User-Data|

**AT Command:** AT+CSCS Character Set 설정 명령어

| Type | Syntax | Response | Example |
|:--------|:--------|:--------|:--------|
| READ | AT+CSCS? | +CSCS:(chset)<br>OK | AT+CSCS?<br>+CSCS:"GSM"<br>OK|
| READ | AT+CSCS=? | +CSCS:(파라미터범위)<br>OK | AT+CSCS=?<br>+CSCS:"GSM","UCS2")<br>OK|
| Write | AT+CSCS:(chset) | OK | AT+CSCS="GSM"<br>OK|


**AT Command:** AT+CNMI 수신 SMS의 표시 설정 명령어

**Syntax:**

| Type | Syntax | Response | Example|
|:--------|:--------|:--------|:--------|
| Write | AT+CNMI=[(mode)[,(mt)[,(bm)[,(ds)[,(bfr)]]]]] | OK | AT+CNMI=2,1,0,0,0<br>OK|
| Read | AT+CNMI? | +CNMI:(mode),(mt),(bm),(ds),(bfr)<br>OK | AT+CNMI?<br>+CNMI:2,1,0,0,0<br>OK|

**Defined values:**

| Parameter | 범위 |  Description |
|:--------|:--------|:--------|
| (mode) | 0 ~ 2 | 수신한 SMS의 알림 방법 설정<br>0: SMS 알림과 SMS 전송알림을 모듈의 SMS 알림용 버퍼에 저장하고, 외부장치에는 전송하지 않음<br>1: SMS 알림과 SMS 전송 알림을 직접 외부장치에 전송, 전송불가 시 알림을 파기<br>2: SMS 알림과 SMS 전송알림을 직접 외부장치에 전송, 전송불가 시 SMS 알림을 모듈의 SMS 알림용 버퍼에 저장하고, 전송 가능한 때에 외부장치에 전송함 |
| (mt) | 0 ~ 3 | 수신한 SMS의 저장 방법이나 알림 방법을 설정<br>0: SMS의 수신을 외부 장치에 알림하지 않음<br>1: 수신한 SMS를 모듈 또는 USIM 카드에 저장하고, 저장 위치를 외부 장치로 전송<br>2: 수신한 SMS를 모듈 또는 USIM 카드에 저장하지 않고, 직접 외부 장치에 전송<br>3: Class3 SMS의 경우는, 직접 전송(+CMT)수행함(mt=2 인 경우와 동일). Class3 이외의 SMS인 경우는, 모듈 내에 저장하고, 저장 위치를 전송(+CMTI)함(m5=1 인 경우와 동일)|
| (bm) | 0, 2 | 0: Cell Broadcast Messages를 DTE에 전송하지 않음<br>2: Cell Broadcast Messages를 unsolicited result code와 함께 DTE에 전송 |
| (ds) | 0 ~ 2 | SMS 전송 알림에 대해 설정<br>0: SMS 전송 알림을 모듈에 저장하지 않고, 외부 장치에도 전송하지 않음<br>1: SMS 전송 알림을 모듈에 저장하지 않고, 직접 외부 장치에 전송하고, Unsolicited result code(+CDS)로 전송 (mode)를 0또는 2로 설정하는 경우, 본 파라미터를 1로 설정하면 안됨<br>2: SMS의 전송 알림을 모듈에 저장하고, Unsolicited result code(+CDSI)로 저장 위치를 외부 장치로 전송|
| (bfr) | 0, 1 | 0: (mode)=1..2가 입력된 경우, Unsolicited result code의 TA buffer를 TE에 전송<br>1: (mode)=1..2가 입력된 경우, Unsolicited result code의 TA buffer를 초기화|



## 동작 구조 예제 (AM Telecom Cat.M1 SMS)

#작성예정

```
AT*SMSMO=0107576****,,1,1,48656C6C6F            // “Hello” 전송
OK

*SMSACK

AT*SMSMO=0107576****,,1,1,BFC0C7C2C7CFBFECBDBA  // “오픈하우스” 전송
OK

*SMSACK

AT*SKT*READMT=0                                  // “Hi” 수신
*SKT*READMT:0,20190225165426,0107576****,4098,Hi

OK

AT*SKT*READMT=0                                  // “안녕하세요?” 수신
*SKT*READMT:0,20190225165505,0107576****,4098,안녕하세요?

OK
```
**[ Note ]**
EUC-KR이 적용되지 않은 PC 일부 터미널에서는 한글이 깨져 보일 수 있으니 주의 하시기 바랍니다.
> AT*SKT*READMT=0
> *SKT*READMT:0,20190225165505,0107576****,4098,¾E³cCI¼¼¿a?
>
> OK

스마트폰에서 SMS 로그는 아래와 같습니다.
![][1]







[arduino-getting-started]: ./Arduino_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
[link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-mqtt-an]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_MQTT_Application_Note_V1.0.pdf

[hw-stack]: ./imgs/hw/wiot-shield-wm01-arduinomega2560_stack.png 
[compile1]: ./imgs/arduino_guide_ide_compile.png
[compile2]: ./imgs/arduino_guide_ide_compile_finish.png
[serialMonitor]: ./imgs/arduino_guide_ide_serialmonitor.png

[1]: ./imgs/arduino_guide_bg96_sms-1.png
[2]: ./imgs/arduino_guide_bg96_sms-2.png
[3]: ./imgs/arduino_guide_bg96_sms-3.png
[4]: ./imgs/arduino_guide_bg96_sms-4.png
