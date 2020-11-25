# Arduino 기반으로 Cat.M1 GPS 활용하기

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [AT 명령어](#Step-2-ATCommand)
-   [동작 구조 예제](#Step-3-SampleCode)
-   [예제 코드 빌드 및 실행](#Step-4-Build-and-Run)

<a name="Prerequisites"></a>
## 시작하기 전에

> * 하드웨어 설정과 개발환경 구축은  **[Arduino 기반으로 Cat.M1 디바이스 개발 시작하기][arduino-getting-started]** 문서에 상세히 설명되어 있습니다.

> * Cat.M1과 같은 Cellular IoT 디바이스는 통신 서비스 사업자의 운영 기준 및 규정에 따라 모듈 펌웨어 및 동작 방식에 차이가 있을 수 있습니다. 본 문서는 한국 **[SK Telecom Cat.M1 서비스][skt-iot-portal]** 를 기준으로 작성되었습니다.


### Development Environment
* **[Arduino IDE][link-arduino-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [Arduino Mega2560 Rev3][link-arduino Mega2560 Rev3] | WIoT-WM01 (WM-N400MSE) |

> * 별도의 GNSS Antenna가 필요합니다.
> * WM-N400MSE의 펌웨어 버전 S130xx.027 이후부터 GPS가 지원됩니다. ATI 혹은 AT$$SWVER 를 이용하여 펌웨어 버전을 확인 후, 이전 버전인 경우, 오픈하우스(openhouse@wiznet.io)에 문의하시기 바랍니다. 

<a name="Step-1-Overview"></a>
## 소개

본 문서에서는 Arduino 기반 개발 환경에서 WIZnet IoT shield와 Arduino Mega2560 Rev3 보드를 이용하여 Cat.M1 단말의 GPS 활용 방법에 대한 가이드를 제공합니다.

대부분의 Cat.M1 장치는 GPS(Global Positioning System)를 지원하고 있습니다. GPS는 범지구 위성 항법 시스템(GNSS, Global Navigation Satellite System)의 한 종류로, 대중적으로 활용되는 글로벌 위성 위치 확인 시스템입니다.

![][gnss]

Cat.M1 모듈의 GPS기능을 활용하면 아이나 반려동물, 귀중품의 위치를 실시간으로 파악하는 위치 트래커(GPS tracking unit) 등의 위치기반 서비스를 손쉽게 구현 할 수 있습니다. Cat.M1 기반의 위치 트래커는 기존의 블루투스 스마트 태그(Bluetooth Smart Tags)와 달리, 모듈 자체적으로 파악한 GPS 정보를 LTE망을 이용하여 사용자에게 전달함으로써 보다 빠르고 정확한 위치 파악이 가능한 것이 큰 장점입니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. GPS 정보의 획득 기능도 AT 명령어를 이용하여 활성화 할 수 있습니다.

<a name="Step-2-ATCommand"></a>
## AT 명령어

> AT 명령어에 대한 좀 더 상세한 설명은 우리넷의 AT Command Manual에서 확인 하실 수 있습니다.
> * [WM-N400MSE_AT_Commands_Guide_v1.1][link-wm-n400mse-atcommand-manual]
> * [WM-N400S series_GPS_Guide][link-wm-n400mse-gnss-manual]

### 0. S/W Version 확인

S/W Version S430XX.015 (2019년 12월 1일 이후 빌드 버전)의 경우 GPS 출력 인터페이스 설정을 해주어야만 GPS 기능이 활성화 됩니다. 

이전 버전의 경우에는 "1. GPS 기능 활성화" AT Command를 바로 사용하시면 됩니다. 

**AT Command:** AT$$SWVER

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT$$SWVER | $$SWVER:<S/W Version>,<빌드 시간 정보> | AT$$SWVER<br>$$SWVER: Sxx0XXX.000, Dec 01 2017 15:15:15<br>OK |

### 0-1. S/W Version 이 430XX.015 (2019년 12월 1일 이후 빌드 버전) 인 경우 GPS 기능 출력 인터페이스 설정

S/W Version 430XX.015 (2019년 12월 1일 이후 빌드 버전)의 경우 아래 명령어를 사용하여 GPS Interface를 설정하고 활성화 합니다. 

**AT Command:** AT$$GPSCONF
AT$$GPSCONF=1,0,1000,252,1,0,1,1 

ps. 각각의 파라미터 값은 수정이 가능하며, 상세한 파라미터에 대한 내용은 추후에 업데이트 할 예정입니다. 


### 1. GPS 기능 활성화

GPS 기능을 활성화 합니다.

**AT Command:** AT$$GPS

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT$$GPS | OK | AT$$GPS<br>OK |

### 2. GPS 기능 비활성화

GPS 기능을 비활성화 합니다.

**AT Command:** AT$$GPSSTOP

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT$$GPSSTOP | OK | AT$$GPSSTOP<br>OK |

<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

WIZnet IoT Shield를 Standalone 모드로 동작시켜, 터미널 프로그램으로 직접 AT 명령어를 입력해 보면서 동작 순서를 확인합니다.
기본적인 GPS 기능 활용을 위해서는 `AT$$GPS`(GPS on), `AT$$GPSSTOP`(GPS off) AT 명령어를 이용합니다.

다음과 같이 확인 할 수 있습니다.

![][1]

<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행



> Google Maps: https://www.google.com/maps/



[arduino-getting-started]: ./Arduino_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
[link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-mqtt-an]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_MQTT_Application_Note_V1.0.pdf
[link-wm-n400mse-atcommand-manual]: ./datasheet/WM-N400MSE_AT_Commands_Guide_v1.1.pdf
[link-wm-n400mse-gnss-manual]: ./datasheet/WM-N400S%20series_GPS_Guide.pdf

[hw-stack]: ./imgs/hw/wiot-shield-wm01-arduinomega2560_stack.png 
[compile1]: ./imgs/arduino_guide_ide_compile.png
[compile2]: ./imgs/arduino_guide_ide_compile_finish.png
[serialMonitor]: ./imgs/arduino_guide_ide_serialmonitor.png

[1]: ./imgs/arduino_guide_bg96_tcp-1.png
[2]: ./imgs/arduino_guide_wmn400_tcp-2.png
[3]: ./imgs/arduino_guide_wmn400_tcp-3.png
[4]: ./imgs/arduino_guide_wmn400_tcp-4.png
[5]: ./imgs/arduino_guide_wmn400_tcp-5.png


[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/mbed_guide_wm-n400mse_gps-2.png
[2]: ./imgs/mbed_guide_bg96_gps-2.png
[3]: ./imgs/mbed_guide_bg96_gps-3.png
[4]: ./imgs/mbed_guide_bg96_gps-4.png
[gnss]: ./imgs/mbed_guide_bg96_gps-gnss.png


