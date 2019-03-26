# Arduino 기반의 Cat.M1 MQTT 테스트 가이드

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [AT 명령어](#Step-2-ATCommand)
-   [동작 구조 예제](#Step-3-SampleCode)
-   [예제 코드 빌드 및 실행](#Step-4-Build-and-Run)

<a name="Prerequisites"></a>
## 시작하기 전에

> * 하드웨어 설정과 개발환경 구축은 **[Arduino 기반으로 Cat.M1 디바이스 개발 시작하기][arduino-getting-started]** 문서에 상세히 설명되어 있습니다.

> * Cat.M1과 같은 Cellular IoT 디바이스는 통신 서비스 사업자의 운영 기준 및 규정에 따라 모듈 펌웨어 및 동작 방식에 차이가 있을 수 있습니다. 본 문서는 한국 **[SK Telecom Cat.M1 서비스][skt-iot-portal]**를 기준으로 작성되었습니다.


### Development Environment
* **[Arduino IDE][link-arduino-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [Arduino Mega2560 Rev3][link-arduino Mega2560 Rev3] | WIoT-WM01 (WM-N400MSE) |

<a name="Step-1-Overview"></a>
## 소개
본 문서에서는 Arduino IDE 기반 개발 환경에서 WIZnet IoT shield와 Arduino Mega2560 Rev3 보드를 이용하여 Cat.M1 단말의 MQTT 데이터 송수신 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 MQTT Client의 통신 과정은 다음과 같은 순서로 구현합니다.

1. 네트워크 인터페이스 활성화
2. MQTT 접속 
3. MQTT 데이터 Publish 
4. MQTT 데이터 Subscribe 
5. MQTT 접속 해제 

이번 MQTT 테스트에서는 MQTT Broker로 HiveMQ를 사용하였습니다. HiveMQ의 Public MQTT Broker와 Websocker Client를 통하여 Cat M1 모듈의 Pub/Sub 데이터 송수신을 비교, 확인하실 수 있습니다.
### [HiveMQ](https://www.hivemq.com)
* Host: broker.hivemq.com
* TCP Port: 1883


<a name="Step-2-ATCommand"></a>
## AT 명령어

> 좀 더 상세한 AT 명령어 설명은 Quectel AT Command Manual에서 확인 하실 수 있습니다.
> * [Quectel_BG96_MQTT_Application_Note_V1.0][link-bg96-mqtt-an]
> * [BG96_AT_Commands_Manual_V2.1][link-bg96-atcommand-manual]

### 1. Cat M1 모듈의 일반적인 동작 설명은 생략

>  Cat M1 모듈의 에코 모드 설명, USIM 상태 확인, 네트워크 접속 확인, PDP Context 활성화 등의 일반적인 내용은 TCP 가이드를 참고하시기 바랍니다.

### 2. MQTT 접속 
입력된 IP  주소와 포트 번호를 이용하여, MQTT Broker에 접속합니다.

**AT Command:** AT+QMTOPEN

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QMTOPEN=? | +QMTOPEN: (0-5),"host_name",(1-65536)<br><br>OK | - |
| Read | AT+QMTOPEN? | +QMTOPEN: (tcpconnectID),"(host_name)",(port)<br>OK | AT+QMTOPEN?<br>+QMTOPEN: 0,"broker.hivemq.com",1883<br>OK |
| Write | AT+QMTOPEN=(tcpconnectID),(host_name),(port) | OK<br>+QMTOPEN: (tcpconnectID),(result)<br>+CME ERROR: <err> | AT+QMTOPEN=0,"broker.hivemq.com",1883<br>OK<br>+QMTOPEN: 0,0 |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (tcpconnectID) | MQTT socket identifier. The range is 0-5. |
| (host_name) | The address of the server. It could be an IP address or a domain name. |
| (port) | The port of the server. The range is 1-65535. |
| (result) | Result of the command execution |

**AT Command:** AT+QMTCONN

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QMTCONN=? | +QMTCONN: (list of supported (tcpconnectID)s),"(clientID)"[,"(username)"[,"(password)"]] | - |
| Read | AT+QMTCONN? | [+QMTCONN: (tcpconnectID),(state)]<br>OK | - |
| Write | AT+QMTCONN=(tcpconnectID),"(clientID)"[,"(username)"[,"(password)"]] | OK<br>+QMTCONN: (tcpconnectID),(result)[,(ret_code)]<br>If there is an error related to ME functionality, response:<br>+CME ERROR: (err)|  AT+QMTCONN=0,"clientExample","testid","testpassword"<br>OK<br>+QMTCONN: 0,0,0 |


**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (tcpconnectID) | MQTT socket identifier. The range is 0-5. |
| (clientID) | The client identifier string. |
| (username) | User name of the client. It can be used for authentication |
| (password) | Password corresponding to the user name of the client. It can be used for authentication |


### 3. MQTT 데이터 Publish 

AT+QMTPUB 명령은 MQTT Broker에게 데이터를 전송할 때 사용됩니다. 토픽 이름으로 구분되어지는 메시지를 전송하며, 해당 토픽을 Subscrive하는 MQTT 클라이언트에게 이를 전송합니다.

**AT Command:** AT+QMTPUB

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QMTPUB=? | +QMTPUB: (0-5), (msgid),(0-2),(0,1),"topic",(1-1548)<br>OK | -|
| Write | AT+QMTPUB=(tcpconnectID),(msgID),(qos),(retain),"(topic)"<br> 명령어 이후에 전송할 메시지 입력이 끝나면 Ctrl + Z를 입력합니다.| OK<br>+QMTPUB: (tcpconnectID),(msgID),(result),(value)<br>+CME ERROR: (err) | AT+QMTPUB=0,0,0,0,"topic/openhouse-B"<br> > Hello IoT Open House<br>OK |



**Defined values:**

| Parameter | Description |
|:--------|:--------|
| (tcpconnectID) | MQTT socket identifier. |
| (msgID) | Message identifier of packet. | 
| (qos) | The QoS level at which the client wants to publish the messages.| 
| (retain) | Whether or not the server will retain the message after it has been delivered to the current subscribers.| 
| (topic) | Topic that needs to be published| 
| (msg) | Message to be published| 
| (result) | Result of the command execution| 
| (value) | If (result) is 1, it means the times of packet retransmission. If (result) is 0 or 2, it will not be presented.| 
| (pkt_timeout) | Timeout of the packet delivery. The range is 1-60. The default value is 5. | 
| (retry_times) | Retry times when packet delivery times out. The range is 0-10. The default valueis 3.| 



### 4. MQTT 데이터 Subscribe

AT+QMTSUB 명령은 MQTT Broker에게 원하는 토픽의 데이터를 받고자 할 때 사용됩니다. 이 명령어를 이용하여 하나 이상의 토픽을 Subscribe 할 수 있습니다.

**AT Command:** AT+QMTSUB

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QMTSUB=? | AT+QMTSUB=?<br>+QMTSUB: (0-5), <msgid>,list of ["topic",qos]<br>OK| -|
| Write | AT+QMTSUB=(tcpconnectID),(msgID),(topic1),(qos1)... | OK<br>+QMTSUB: (tcpconnectID),(msgID),(result),(value)<br>+CME ERROR: (err) | AT+QMTSUB=0,1,"topic/openhouse-A",2<br>OK<br>+QMTSUB: 0,1,0,2 |

**Defined values:**

| Parameter | Description |
|:--------|:--------|
| (tcpconnectID) | MQTT socket identifier. |
| (msgID) | Message identifier of packet. |
| (qos) | The QoS level at which the client wants to publish the messages.|
| (retain) | Whether or not the server will retain the message after it has been delivered to the current subscribers.|
| (topic) | Topic that needs to be published|

### 5. MQTT 접속 해제 

AT+QMTDISC 명령은 MQTT Broker와의 연결을 해제할 때 사용합니다.

**AT Command:** AT+QMTDISC

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QMTDISC=? |+QMTDISC: (list of supported <tcpconnectID>s)<br>OK| -|
| Write |AT+QMTDISC=(tcpconnectID)| OK<br>+QMTDISC: (tcpconnectID),(result)<br>+CME ERROR: (err) | AT+QMTDISC=0<br>OK<br>+QMTDISC: 0,0 |

**Defined values:**

| Parameter | Description |
|:--------|:--------|
| (tcpconnectID) | MQTT socket identifier. |
| (result) | Result of the command execution|


<a name="Step-3-SampleCode"></a>

## 동작 구조 예제 

위에 설명한 대로, MQTT Broker는 HiveMQ(broker.hivemq.com:1883)를 사용합니다.
HiveMQ를 사용하기 위한 별도의 설정은 필요 없고, DNS 주소와 MQTT 포트 번호만 기억하시면 됩니다.

HiveMQ MQTT Websocket Client는 아래 링크에서 바로 테스트가 가능합니다.
> * [HiveMQ Websocket Client](http://www.hivemq.com/demos/websocket-client/)

먼저, HiveMQ MQTT Websocket Client를 HiveMQ Public Broker에 접속하도록 하겠습니다.
이 때, Username과 Password는 "testid"와 "testpassword"로 설정하도록 하겠습니다.
* Host: broker.hivemq.com
* TCP Port: 1883
* Username: "testid" 
* Password: "testpassword"

![][1]


테스트에서는 Cat M1 모듈이 "topic/openhouse-B" 토픽으로 데이터를 보낼 예정입니다.
따라서, Cat M1 모듈이 보내는 MQTT 데이터를 확인하기 위해서, 테스트에 사용될 토픽("topic/openhouse-B")을 아래 그림과 같이 Subscribe 합니다.

![][2]


이제, Cat M1 모듈이 MQTT Broker에 접속하고 "topic/openhouse-B" 토픽으로 데이터를 보내 보도록 하겠습니다.

```
// MQTT Broker에 접속
AT+QMTOPEN=0,"broker.hivemq.com",1883
OK

+QMTOPEN: 0,0
AT+QMTCONN=0,"clientExample","testid","testpassword"
OK

+QMTCONN: 0,0,0

// topic/openhouse-B 토픽에 Hello IoT Open House 데이터 전송
AT+QMTPUB=0,0,0,0,"topic/openhouse-B"
> Hello IoT Open House (입력 후  Ctrl + z)
OK

+QMTPUB: 0,0,0
```

Cat M1 모듈에서는 MQTT Publish가 성공적으로 이루어졌다는 메시지가 나왔습니다.
이 때, HiveMQ MQTT Websocket Client의 화면을 보시면, Cat M1 모듈이 Publish한 데이터("Hello IoT Open House")를 아래와 같이 확인할 수 있습니다.

![][3]


자, 이번에는 HiveMQ MQTT Websocket Client에서 Publish를 해서, Cat M1 모듈에서 Subscribe를 해보도록 하겠습니다.
아래 화면은 Cat M1 모듈에서, "topic/openhouse-A" 토픽을 Subscribe하는 로그 입니다.

```
// MQTT Broker에 접속 (이미 연결이 되어 있다면 생략)
AT+QMTOPEN=0,"broker.hivemq.com",1883
OK

+QMTOPEN: 0,0

// 이미 연결이 되어 있다면 생략
AT+QMTCONN=0,"clientExample","testid","testpassword"
OK

+QMTCONN: 0,0,0

// topic/openhouse-A 토픽을 Subscribe
AT+QMTSUB=0,1,"topic/openhouse-A",2
OK

+QMTSUB: 0,1,0,2
```

이제, HiveMQ MQTT Websocket Client에서 "topic/openhouse-A" 토픽으로 "Hello WIZnet" 라는 데이터를 Publish 해보도록 하겠습니다.

![][4]

그러면, 아래 화면에서처럼 Cat M1 모듈이 "topic/openhouse-A" 토픽으로 "Hello WIZnet" 라는 데이터를 수신된 것을 확인하실 수 있습니다.

```
// MQTT Broker에 접속 (이미 연결이 되어 있다면 생략)
AT+QMTOPEN=0,"broker.hivemq.com",1883
OK

+QMTOPEN: 0,0

// 이미 연결이 되어 있다면 생략
AT+QMTCONN=0,"clientExample","testid","testpassword"
OK

+QMTCONN: 0,0,0

// topic/openhouse-A 토픽을 Subscribe
AT+QMTSUB=0,1,"topic/openhouse-A",2
OK

+QMTSUB: 0,1,0,2

+QMTRECV: 0,0,"topic/openhouse-A","Hello WIZnet"
```

MQTT Broker와의 연결을 끊고 싶다면, AT+QMTDISC 명령어를 사용하여, MQTT 연결을 해제할 수 있습니다.

```
// MQTT Broker와의 연결 해제
AT+QMTDISC=0
OK

+QMTDISC: 0,0
```

아래는 위에서 설명한 MQTT 개별 테스트에 사용했던 Cat M1 모듈의 AT Command 전체 로그입니다.

```
AT+QMTOPEN=0,"broker.hivemq.com",1883
OK

+QMTOPEN: 0,0
AT+QMTCONN=0,"clientExample","testid","testpassword"
OK

+QMTCONN: 0,0,0
AT+QMTPUB=0,0,0,0,"topic/openhouse-B"
> Hello IoT Open House
OK

+QMTPUB: 0,0,0
AT+QMTSUB=0,1,"topic/openhouse-A",2
OK

+QMTSUB: 0,1,0,2

+QMTRECV: 0,0,"topic/openhouse-A","Hello WIZnet"
AT+QMTDISC=0
OK

+QMTDISC: 0,0
```


<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project
다음 링크에서 Arduino 예제 코드를 다운로드한 후, ino 확장자의 프로젝트 파일을 실행 시킵니다.

> 예제에서 활용할 Ping test sample code는 저장소의 아래 경로에 위치하고 있습니다.
> * `\samples\WIoT-QC01_BG96\WIoT-QC01_Arduino_MQTT\`


### 2. Modify parameters

만약 HiveMQ 이외의 다른 MQTT broker와 테스트해 보려는 경우, 다음 define 및 변수들을 변경 할 수 있습니다.

````cpp
#define MQTT_CLIENTID               "clientExample"
#define MQTT_USERID                 "testid"
#define MQTT_PASSWORD               "testpassword"
#define MQTT_SAMPLE_TOPIC_A         "topic/openhouse-A"
#define MQTT_SAMPLE_TOPIC_B         "topic/openhouse-B"
#define MQTT_SAMPLE_TOPIC_C         "topic/openhouse-C"
...
char mqtt_broker_url[] = "broker.hivemq.com";
int mqtt_broker_port = 1883;
````

### 3. Compile

상단 메뉴의 Compile 버튼을 클릭합니다.

![][compile1]

컴파일이 완료 되면 다음과 같이 업로드를 수행하여 최종적으로 보드에 업로드를 수행 합니다.
업로드가 정상적으로 완료되면 'avrdude done. Thank you.' 메시지를 확인 할 수 있습니다.

![][compile2]

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 MQTT connect와 Publish / Subscribe 운용에 대해 파악할 수 있습니다.

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
int8_t openMqttBroker_BG96(char * url, int port);
```
 * MQTT broker 접속을 위한 소켓을 할당하고 준비하는 과정입니다.
 * 파라메터로 접속할 broker의 URL과 Port 번호가 필요합니다.

```cpp
int8_t connectMqttBroker_BG96(char * clientid, char * userid, char * password);
```
 * MQTT broker로 접속을 시도합니다.
 * 파라메터로 Client ID, User ID, Password가 필요합니다.
   * User ID와 Password가 필요 없는 연결의 경우, NULL을 입력합니다.

```cpp
int8_t closeMqttBroker_BG96(void);
```
 * MQTT broker와의 연결을 종료합니다.

```cpp
int8_t sendMqttPublishMessage_BG96(char * topic, int qos, int retain, char * msg, int len);
```
 * 지정한 Topic에 메지시를 보냅니다. broker가 해당 topic을 subscribe 하고 있는 경우, 메시지를 확인 할 수 있습니다.

```cpp
int8_t setMqttSubscribeTopic_BG96(char * topic, int msgid, int qos);
```
 * Subscribe 할 Topic을 지정합니다. broker가 해당 topic으로 publish 하는 경우, 메시지를 수신 할 수 있습니다.


```cpp
int8_t checkRecvMqttMessage_BG96(char * topic, int * msgid, char * msg);
```
  * Subscribe 중인 Topic에 수신된 메시지가 있는지 검사하며, 만약 수신된 메시지가 있을 경우 해당 메시지의 topic, message ID, 메시지를 파라메터의 변수로 넘겨줍니다.
  * 메인 루프에 위치해야 합니다.

#### 4.3 Set up serial monitor
![][serialMonitor]

#### 4.4 Demo
![][5]

..
...

![][6]

..
...

![][7]




[arduino-getting-started]: https://
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
[link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-mqtt-an]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_MQTT_Application_Note_V1.0.pdf

[hw-stack]: ./imgs/hw/wiot-shield-wm01-arduinomega2560_stack.png 
[compile1]: ./imgs/arduino_guide_ide_compile.png
[compile2]: ./imgs/arduino_guide_ide_compile_finish.png
[serialMonitor]: ./imgs/arduino_guide_ide_serialmonitor.png

[1]: ./imgs/arduino_guide_bg96_mqtt-1.png
[2]: ./imgs/arduino_guide_bg96_mqtt-2.png
[3]: ./imgs/arduino_guide_bg96_mqtt-3.png
[4]: ./imgs/arduino_guide_bg96_mqtt-4.png
[5]: ./imgs/arduino_guide_bg96_mqtt-5.png
[6]: ./imgs/arduino_guide_bg96_mqtt-6.png
[7]: ./imgs/arduino_guide_bg96_mqtt-7.png
