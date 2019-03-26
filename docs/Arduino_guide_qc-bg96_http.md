# Arduino 기반의 Cat.M1 HTTP 테스트 가이드

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
본 문서에서는 Arduino IDE 기반 개발 환경에서 WIZnet IoT shield와 Arduino Mega2560 Rev3 보드를 이용하여 Cat.M1 단말의 HTTP 데이터 송수신 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 HTTP Client의 통신 과정은 다음과 같은 순서로 구현합니다.

1. 네트워크 인터페이스 활성화
2. HTTP 설정 - 목적지 URL, 옵션 
3. HTTP Request 전송
4. HTTP Response 확인
5. 네트워크 인터페이스 비활성화

<a name="Step-2-ATCommand"></a>
## AT 명령어

> 좀 더 상세한 AT 명령어 설명은 Quectel AT Command Manual에서 확인 하실 수 있습니다.
> * [Quectel_BG96_MQTT_Application_Note_V1.0][link-bg96-mqtt-an]
> * [BG96_AT_Commands_Manual_V2.1][link-bg96-atcommand-manual]

### 1. Cat M1 모듈의 일반적인 동작 설명은 생략

>  Cat M1 모듈의 에코 모드 설명, USIM 상태 확인, 네트워크 접속 확인, PDP Context 활성화 등의 일반적인 내용은 TCP 가이드를 참고하시기 바랍니다.

### 2. HTTP 설정 
HTTP 서버의 URL을 설정합니다. "http://" 또는 "https://"로 시작하는 URL 주소를 입력하면 됩니다.

**AT Command:** AT+QHTTPURL

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QHTTPURL=? | +QHTTPURL: (1-700),(1-65535)<br><br>OK | - |
| Read | AT+QHTTPURL? | +QHTTPURL: (URL)<br><br>OK | AT+QHTTPURL? <br>+QHTTPURL: http://www.wiznet.io<br><br>OK |
| Write | AT+QHTTPURL=(URL_length),(timeout) | OK or +CME ERROR | AT+QHTTPURL=21,5<br>CONNECT<br>(http://www.wiznet.io/)<br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (URL_length) | Numeric | The length of URL. The range is 1-700.  |
| (context_state) | Numeric |  The maximum time for inputting URL. The range is 1-65535, and the default value is 60. |

### 3. HTTP Request 전송

AT+QHTTPGET 명령은 HTTP 서버에게 GET Request를 전송합니다. 이 명령을 입력하고, 일정 시간 동안 HTTP Response를 기다려야 합니다.

**AT Command:** AT+QHTTPGET

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QHTTPGET=? | +QHTTPGET: (1-65535),(1-2048),(1-65535)<br><br>OK | -|
| Write | AT+QHTTPGET=(rsptime)| OK or +CME ERROR<br><br>+QHTTPGET: (err),(httprspcode),(content_length) | AT+QHTTPGET=10<br>OK<br>+QHTTPGET: 0,200,26 |


**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (rsptime) | Numeric | HTTP Response Timeout. Default 60s  |
| (httprspcode) | Numeric | HTTP(S) Response Codes |
| (err) | Integer | Error code of operation |
| (content_length) | Numeric | The length of HTTP(S) response body |

### 4. HTTP Response 확인

AT+QHTTPREAD 명령은 HTTP 서버에게 받은 HTTP Response 정보를 확인 합니다. 

**AT Command:** AT+QHTTPREAD

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QHTTPREAD=? | +QHTTPREAD: (1-65535)<br><br>OK | -|
| Write | AT+QHTTPREAD=(waittime)| CONNECT<br>(HTTP Response)<br>OK<br>+QHTTPREAD: (err) <br>or<br> +CME ERROR | AT+QHTTPREAD=80<br>CONNECT<br>.....<br>OK<br>+QHTTPREAD: 0



**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (waittime) | Numeric | maximum interval time between receiving two packets of data.  |
| (err) | Integer | Error code of operation |


<a name="Step-3-SampleCode"></a>

## 동작 구조 예제 #1 (헤더 없는 HTTP Get Response 받기)

```
AT+QHTTPCFG="responseheader",0
OK

AT+QHTTPURL=38,5
CONNECT
OK

AT+QHTTPGET=10
OK

// Content-Length:26 수신 (아래는 입력할 필요가 없음)
+QHTTPGET: 0,200,26

// HTTP Response 확인. 웹브라우저로 URL로 접속하면 동일한 데이터를 확인할 수 있음 
AT+QHTTPREAD=10
CONNECT
<BODY>Hello World!</BODY>

OK

+QHTTPREAD: 0
```

## 동작 구조 예제 #2 (헤더 포함한 HTTP Get Response 받기)
```
// HTTP Header 포함
AT+QHTTPCFG="responseheader",1
OK

// 접속할 HTTP 서버의 URL 입력 
AT+QHTTPURL=38,5
CONNECT
https://www.beyondlogic.org/hello.html
OK

// Send HTTP Get Request. 환경에 따라 수초 이상 소요될 수 있음
AT+QHTTPGET=10
OK

// Send HTTP Get Request. 오류 보드 702(HTTP Timeout) 발생
+QHTTPGET: 702

// 다시 한번 Send HTTP Get Request.
AT+QHTTPGET=10
OK

// Content-Length:26 수신
+QHTTPGET: 0,200,26

// HTTP Response 확인 (HTTP 헤더 포함)
AT+QHTTPREAD=10
CONNECT
HTTP/1.1 200 OK
Date: Sat, 16 Mar 2019 03:50:45 GMT
Server: Apache/2.4.38
Last-Modified: Sun, 28 Oct 2018 10:58:22 GMT
ETag: "1a-57947d4b7af80"
Accept-Ranges: bytes
Content-Length: 26
Keep-Alive: timeout=5, max=100
Connection: Keep-Alive
Content-Type: text/html

<BODY>Hello World!</BODY>

OK

+QHTTPREAD: 0
```

## 동작 구조 예제 #3 (기상청 웹서버에 접속하여 분당구 날씨 확인)
```
// 접속할 HTTP 서버의 URL 입력
AT+QHTTPURL=56,5
CONNECT
http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=4113552000
OK

// Send HTTP Get Request. 환경에 따라 수초 이상 소요될 수 있음
AT+QHTTPGET=20
OK

// Content-Length:8782 수신 
+QHTTPGET: 0,200,8782

// HTTP Response 확인 (분당구 수내동의 기상 정보 확인)
AT+QHTTPREAD=20
CONNECT
<?xml version="1.0" encoding="UTF-8" ?>
<rss version="2.0">
<channel>
<title>기상청 동네예보 웹서비스 - 경기도 성남시분당구 수내1동 도표예보</title>
<link>http://www.kma.go.kr/weather/main.jsp</link>
<description>동네예보 웹서비스</description>
<language>ko</language>
.....................................................................
<wfKor>맑음</wfKor>
.....................................................................
OK

+QHTTPREAD: 0
```


<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project
다음 링크에서 Arduino 예제 코드를 다운로드한 후, ino 확장자의 프로젝트 파일을 실행 시킵니다.

> 예제에서 활용할 Ping test sample code는 저장소의 아래 경로에 위치하고 있습니다.
> * `\samples\WIoT-QC01_BG96\WIoT-QC01_Arduino_HTTP\`


### 2. Modify parameters

만약 다른 서비스의 API 테스트를 위해 URL을 변경하려는 경우, 다음 변수의 내용을 변경하면 됩니다.

````cpp
// Sample HTTP URL: Weather info by Korea Meteorological Administration
char request_url[] = "http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=4113552000";
````

### 3. Compile

상단 메뉴의 Compile 버튼을 클릭합니다.

![][compile1]

컴파일이 완료 되면 다음과 같이 업로드를 수행하여 최종적으로 보드에 업로드를 수행 합니다.
업로드가 정상적으로 완료되면 'avrdude done. Thank you.' 메시지를 확인 할 수 있습니다.

![][compile2]

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 HTTP client 운용 방법에 대해 파악할 수 있습니다.

> * 예제 샘플 코드는 `동작 구조 예제 #3` 을 코드로 구현한 것입니다. 만약 구체적인 응용 프로젝트로 구현될 경우, 수신된 HTTP response 메시지를 parsing하여 원하는 정보를 획득 후 이에 해당하는 동작 구현이 추가되는 것이 일반적입니다.
> 
> * 상세한 활용 예는 [BG96_HTTP(S)_AT_Commands_Manual_V1.0][link-bg96-http-manual]를 참고하시기 바랍니다.
> 

#### 4.1 Connect your board
스타터 키트와 Arduino Mega2560과 Uart 통신을 하기위해서는 아래와 같이 점퍼 연결이 필요합니다.
예제 구동을 위해 WIZnet IoT Shield의 UART TXD와 RXD 핀을 Arduino Mega2560 보드의 'Serial 3' `TX3`(14), `RX3`(15) 에 연결합니다.

| ArduinoMega2560 | TX3 (14)  | RX3 (15) |
|:----:|:----:|:----:|
| WIZnet IoT Shield | RXD<br>(UART Rx for D1/D8)  | TXD<br>(UART Tx for D0/D2) |

> 보드 상단에 위치한 UART_SEL 점퍼를 제거한 후 (실크 기준) 오른쪽 핀을 Arduino 보드와 연결합니다.

![][hw-stack]

#### 4.2 Functions

샘플 코드의 HTTP 관련 함수는 다음과 같습니다.

```cpp
int8_t setHttpRequest_BG96(char * url, int len);
```
 * HTTP client로 동작하며 request 메시지를 전송할 HTTP server의 접속 정보 URL과 URL의 길이를 설정합니다.

```cpp
int8_t sendHttpRequest_BG96(int timeout, int * rsp_code, int * content_len);
```
 * setHttpRequest 함수에 설정한 URL로 HTTP Request를 전송합니다. timeout을 파라메터로 함께 전달합니다. (unit: second)
 * HTTP request가 전송되고 응답을 수신할 경우, 응답에 대한 HTTP response code와 response length를 파라메터로 넘겨줍니다.
 * HTTP 200 OK면 rep_code는 200이 됩니다.

```cpp
int8_t getHttpResponse_BG96(int timeout, char * buf, int len);
```
  * 모듈 버퍼로 수신된 HTTP response를 읽어옵니다. 이 때 파라메터로 timeout 및 읽어올 길이를 입력합니다. 수신 데이터는 파라메터의 buf로 전달됩니다.
  * sendHttpRequest 함수의 res_code가 200이라면 정상 응답을 수신한 것이므로 getHttpResponse를 수행해야 합니다.

```cpp
void dumpHttpRespones_BG96(char * buf);
```
 * HTTP response 메시지를 출력하여 사용자에게 보여주기 위한 임시 함수입니다.


#### 4.3 Set up serial monitor
![][serialMonitor]

#### 4.4 Demo
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

[1]: ./imgs/arduino_guide_bg96_http-1.png
