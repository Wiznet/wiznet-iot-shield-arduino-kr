# MBED 기반으로 Cat.M1 GPS 활용하기


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

본 문서에서는 Arduino IDE 기반 개발 환경에서 WIZnet IoT shield와 Arduino Mega2560 Rev3 보드를 이용하여 Cat.M1 단말의 GPS 활용 방법에 대한 가이드를 제공합니다.

대부분의 Cat.M1 장치는 GPS(Global Positioning System)를 지원하고 있습니다. GPS는 범지구 위성 항법 시스템(GNSS, Global Navigation Satellite System)의 한 종류로, 대중적으로 활용되는 글로벌 위성 위치 확인 시스템입니다.

![][gnss]

Cat.M1 모듈의 GPS기능을 활용하면 아이나 반려동물, 귀중품의 위치를 실시간으로 파악하는 위치 트래커(GPS tracking unit) 등의 위치기반 서비스를 손쉽게 구현 할 수 있습니다. Cat.M1 기반의 위치 트래커는 기존의 블루투스 스마트 태그(Bluetooth Smart Tags)와 달리, 모듈 자체적으로 파악한 GPS 정보를 LTE망을 이용하여 사용자에게 전달함으로써 보다 빠르고 정확한 위치 파악이 가능한 것이 큰 장점입니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. GPS 정보의 획득 기능도 AT 명령어를 이용하여 활성화 할 수 있습니다.

<a name="Step-2-ATCommand"></a>
## AT 명령어

> AT 명령어에 대한 좀 더 상세한 설명은 Quectel AT Command Manual에서 확인 하실 수 있습니다.
> * [BG96_GNSS_AT_Commands_Manual_V1.1][link-bg96-gnss-manual]
> * [BG96_AT_Commands_Manual_V2.1][link-bg96-atcommand-manual]

### 1. GPS 기능 활성화

GPS 기능을 활성화 합니다.

**AT Command:** AT+QGPS

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Read | AT+QGPS? | +QGPS: (gnssstate)<br><br>OK | - |
| Write | AT+QGPS=(gnssmode)[,(fixmaxtime)[,(fixmaxdist)[,(fixcount)[,(fixrate)]]]] | OK<br><br>ME 관련 기능 Error 발생 시,<br>+CME ERROR: (errcode) | AT+QGPS=2<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (gnssstate) | Integer | GNSS state<br>0: GNSS OFF<br>1 GNSS ON |
| (gnssmode) | Integer | GNSS 동작 모드<br>1: Stand-alone<br>2: MS-based<br>3: MS-assisted<br>4: Speed-optimal |
| (fixmaxtime) | Integer | The maximum positioning time (unit: s.) |
| (fixmaxdist) | Integer | Accuracy threshold of positioning. (Unit: m.) |
| (fixcount) | Integer | Number of attempts for positioning.<br>(0–1000, 0 indicates continuous positioning)  |
| (fixrate) | Integer | The interval time between the first and second time positioning. (unit: s.) |
| (errcode) | Integer |  에러 코드<br>상세 내용은 [BG96_GNSS AT_Commands_Manual: Chapter 4][link-bg96-gnss-manual] 참조 |

### 2. GPS 기능 비활성화

GPS 기능을 비활성화 합니다.

**AT Command:** AT+QGPSEND

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Write | AT+QGPSEND | OK<br><br>ME 관련 기능 Error 발생 시,<br>+CME ERROR: (errcode) | AT+QGPSEND<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (errcode) | Integer |  에러 코드<br>상세 내용은 [BG96_GNSS AT_Commands_Manual: Chapter 4][link-bg96-gnss-manual] 참조 |


### 3. GPS 위치 정보 출력

GPS 위치 정보를 출력합니다.

> GPS 위치 정보가 확정되지 않은 경우, (errorcode) `+CME ERROR: 516` (not fixed now)를 출력합니다. 약간의 시간 경과 후에 위치 정보가 확정되면 정상적인 메시지의 출력을 확인 할 수 있습니다.


**AT Command:** AT+QGPSLOC

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Write | AT+QGPSLOC=(mode) | +QGPSLOC: (UTC),(latitude),(longitude),(hdop),(altitude),(fix),(cog),(spkm),(spkn),(date),(nsat)<br><br>ME 관련 기능 Error 발생 시,<br>+CME ERROR: (errcode) | +QGPSLOC: 062122.0,37.37827,127.11290,1.4,88.0,2,143.07,0.0,0.0,180319,08<br><br>OK |

**Defined values:**

> GNSS 동작 모드는 Stand-alone(1), MS-based(2), MS-assisted(3), Speed-optimal(4)를 지원합니다. 예제 코드에서는 망 정보를 통해 보다 정확한 위치 정보를 제공하는 (mode) 2 관련 설명만 기술합니다.



| Parameter | Type | Description |
|:--------|:--------|:--------|
| (mode) | Integer | 위도(latitude) 및 경도(longitude)의 display format<br>0 :(latitude),(longitude) format: ddmm.mmmm N/S,dddmm.mmmm E/W<br>1 :(latitude),(longitude) format: ddmm.mmmmmm N/S,dddmm.mmmmmm E/W <br> 2 :(latitude),(longitude) format: (-)dd.ddddd,(-)ddd.ddddd |
| (UTC) | Float | UTC time. Format: hhmmss.sss |
| (latitude) | Float | If (mode) is 2:<br>Format: (-)dd.ddddd (Quoted from GPGGA sentence)<br>dd.ddddd: -89.99999-89.99999 (degree)<br>- : South latitude |
| (longitude) | Float | If (mode) is 2:<br>Format: (-)dd.ddddd Quoted from GPGGA sentence)<br>dd.ddddd: -179.99999-179.99999 (degree)<br>- : West longitude |
| (hdop) | Float | Horizontal precision: 0.5-99.9 (Quoted from GPGGA sentence). |
| (altitude) | Float | The altitude of the antenna away from the sea level (unit: m), accurate to one decimal place (Quoted from GPGGA sentence) |
| (fix) | Float | GNSS positioning mode (Quoted from GNGSA/GPGSA sentence).<br>2: 2D positioning<br>3: 3D positioning |
| (cog) | Float | Course Over Ground based on true north. Format: ddd.mm (Quoted from GPVTG sentence).<br>ddd: 000-359 (degree)<br> mm: 00-59 (minute) |
| (spkm) | Float | Speed over ground. Format: xxxx.x; unit: Km/h; accurate to one decimal place (Quoted from GPVTG sentence). |
| (spkn) | Float | Speed over ground. Format: xxxx.x; unit: knots; accurate to one decimal place (Quoted from GPVTG sentence). |
| (date) | String | UTC time when fixing position. Format: ddmmyy (Quoted from GPRMC sentence). |
| (nsat) | Integer | Number of satellites, from 00 (The first 0 should be retained) to 12 (Quoted from GPGGA sentence). |
| (errcode) | Integer |  에러 코드<br>상세 내용은 [BG96_GNSS AT_Commands_Manual: Chapter 4][link-bg96-gnss-manual] 참조 |



<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

WIZnet IoT Shield를 Standalone 모드로 동작시켜, 터미널 프로그램으로 직접 AT 명령어를 입력해 보면서 동작 순서를 확인합니다.
기본적인 GPS 기능 활용을 위해서는 `AT+QGPS`(GPS on), `AT+QGPSEND`(GPS off), `AT+QGPSLOC` (GPS location) AT 명령어를 이용합니다.

다음과 같이 확인 할 수 있습니다.

![][1]

> GPS 기능 활용 시, Quectel USB NMEA Port를 통해서도 GPS 정보가 출력됩니다.
> NMEA Port는 WIZnet IoT Shield의 `P2 USB HOST` USB 포트를 PC와 연결하여 확인할 수 있습니다.
> * [NMEA 0183](https://en.wikipedia.org/wiki/NMEA_0183) 구조의 log 메시지가 출력됩니다. (NMEA, National Marine Electronics Association)
> 1. (윈도우의 경우) 장치 관리자에서 Quectel USB NMEA Port 의 COM 포트 확인 (그 외 OS의 경우, COM 포트 확인)
> 2. 시리얼 터미널 프로그램의 Baudrate를 115200으로 설정하여 확인된 COM 포트로 연결
> 3. NMEA 포트 출력 정보 확인

![][usbport]


<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project
다음 링크에서 Arduino 예제 코드를 다운로드한 후, ino 확장자의 프로젝트 파일을 실행 시킵니다.

> 예제에서 활용할 Ping test sample code는 저장소의 아래 경로에 위치하고 있습니다.
> * `\samples\WIoT-QC01_BG96\WIoT-QC01_Arduino_GPS\`


### 2. Modify parameters

Ping 테스트 예제 코드는 별도로 수정할 Parameter가 없습니다.

### 3. Compile

상단 메뉴의 Compile 버튼을 클릭합니다.

![][compile1]

컴파일이 완료 되면 다음과 같이 업로드를 수행하여 최종적으로 보드에 업로드를 수행 합니다.
업로드가 정상적으로 완료되면 'avrdude done. Thank you.' 메시지를 확인 할 수 있습니다.

![][compile2]

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 GPS 서비스 운용에 대해 파악할 수 있습니다.

#### 4.1 Connect your board
스타터 키트와 Arduino Mega2560과 Uart 통신을 하기위해서는 아래와 같이 점퍼 연결이 필요합니다.
예제 구동을 위해 WIZnet IoT Shield의 UART TXD와 RXD 핀을 Arduino Mega2560 보드의 'Serial 3' `TX3`(14), `RX3`(15) 에 연결합니다.

| ArduinoMega2560 | TX3 (14)  | RX3 (15) |
|:----:|:----:|:----:|
| WIZnet IoT Shield | RXD<br>(UART Rx for D1/D8)  | TXD<br>(UART Tx for D0/D2) |

> 보드 상단에 위치한 UART_SEL 점퍼를 제거한 후 (실크 기준) 오른쪽 핀을 Arduino 보드와 연결합니다.

![][hw-stack]

#### 4.2 Functions

샘플 코드의 GPS관련 함수는 다음과 같습니다.

````cpp
int8_t setGpsOnOff_BG96(bool onoff);
````
* 파라메터에 따라 GPS On / Off를 수행합니다.


````cpp
int8_t getGpsLocation_BG96(gps_data *data);
````
* gps_data 구조체를 파라메터로 받아, Cat.M1 모듈로부터 획득한 GPS 정보를 저장하여 리턴합니다.


#### 4.3 Set up serial monitor
![][serialMonitor]

#### 4.3 Get GPS information through a serial terminal

성공적으로 연결이 완료된 후 보드를 리셋하면 다음과 같은 시리얼 출력을 확인 할 수 있습니다.
예제 코드는 Cat.M1 모듈의 상태를 확인한 후 GPS를 구동하여 정보를 출력하도록 구성되어 있습니다.

> GPS 위치 정보가 확인 될 때까지 약간의 시간이 필요할 수 있습니다.

![][2]

![][3]



#### 4.4 See the location via Google Maps service (latitude, longitude)

GPS의 위치 정보 중 위도(latitude)와 경도(longitude)를 Google Maps에 입력하면 손쉽게 측정된 위치를 확인 할 수 있습니다.

> Google Maps: https://www.google.com/maps/

확인 결과, 분당구 수내동에 위치한 위즈네트 위치가 정확하게 표시되는 것을 알 수 있습니다.

![][4]




[arduino-getting-started]: ./Arduino_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
[link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-gnss-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_GNSS_AT_Commands_Manual_V1.1.pdf

[mbed-guide-bg96-tcp]: ./mbed_guide_bg96_tcp.md
[mbed-guide-bg96-http]: ./mbed_guide_bg96_http.md
[mbed-guide-bg96-mqtt]: ./mbed_guide_bg96_mqtt.md

[hw-stack]: ./imgs/hw/wiot-shield-wm01-arduinomega2560_stack.png
[compile1]: ./imgs/arduino_guide_ide_compile.png
[compile2]: ./imgs/arduino_guide_ide_compile_finish.png
[serialMonitor]: ./imgs/arduino_guide_ide_serialmonitor.png

[1]: ./imgs/arduino_guide_bg96_gps-1.png
[2]: ./imgs/arduino_guide_bg96_gps-2.png
[3]: ./imgs/arduino_guide_bg96_gps-3.png
[4]: ./imgs/arduino_guide_bg96_gps-4.png
[gnss]: ./imgs/arduino_guide_bg96_gps-gnss.png
[usbport]: ./imgs/device_manager_quectel_usbport.png

