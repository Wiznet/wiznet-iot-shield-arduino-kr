# Arduino 기반으로 Cat.M1 개발 시작하기

## 목차

-   [소개](#Introduction)
-   [Step 1: 필수 구성 요소](#Step-1-Prerequisites)
-   [Step 2: 디바이스 준비](#Step-2-PrepareDevice)
-   [Step 3: 예제 코드 빌드 및 실행](#Step-3-Build)
-   [더 보기](#ReadMore)


<a name="Introduction"></a>
## 소개

**문서의 주요 내용**

이 문서는 [Arduino][link-arduino]를 기반으로 Cat.M1 장치의 개발 환경 구축 및 예제 코드 실행 과정에 대해 설명합니다.

각 과정에는 다음 내용들이 포함되어 있습니다:
- WIZnet IoT Shield와 Arduino Mega2560 Rev3 하드웨어 설정
- Arduino 예제 코드 실행 과정
- 디바이스 상에서 Cat.M1 통신 기능 확인 과정


## Step 1: 필수 구성 요소
이 문서를 따라하기 전에 다음과 같은 것들이 준비되어야 합니다.

- **Arduino IDE 개발환경을 활용 할 수 있는 컴퓨터**
  - 프로그래밍 및 디버깅을 위한 IDE 프로그램 ([Arduino IDE][link-arduino-compiler])

- **Cat.M1 디바이스 개발용 하드웨어**
  - [Arduino Mega2560 Rev3][link-arduino Mega2560 Rev3]
  - WIZnet IoT Shield
  - Cat.M1 Interface Board (택 1)
    - WIoT-QC01 (앰투앰넷 BG96)
    - WIoT-WM01 (우리넷 WM-N400MSE)
    - WIoT-AM01 (AM텔레콤 AMM5918K)

- **Cat.M1 모듈의 (시험 망)개통**
  - Cat.M1 모듈로 통신 기능을 구현하려면 **망 개통 과정**이 선행되어야 합니다.
    - 한국의 경우, 국내 Cat.M1 서비스 사업자인 SK Telecom의 망 개통 과정이 필요합니다.

> 모듈은 개발 단계에 따라 시험망 개통 - 상용망 개통 단계를 거쳐야 하며 외장형 모뎀은 즉시 상용망 개통이 가능합니다.
> * 시험망 개통의 경우 [SKT IoT OpenHouse][skt-iot-portal]에 기술 지원 문의
> * 상용망 개통의 경우 USIM 구매 대리점이나 디바이스 구매처에 개통 문의

<a name="Step-2-PrepareDevice"></a>
## Step 2: 디바이스 준비

### 하드웨어 설정

WIZnet IoT Shield를 Arduino Mega2560 Rev3 하드웨어와 결합합니다. 
- 두 장치 모두 Arduino UNO Rev3 호환 핀 헤더를 지원하므로 손쉽게 결합(Stacking) 할 수 있습니다.

![][1]

IoT Shield는 다양한 밴더의 Cat.M1 모듈을 활용 할 수 있도록 하드웨어 설정을 제공합니다. 따라서 선택한 Cat.M1 Interface Board를 확인하여 장치 설정이 필요합니다.

- 각각 밴더의 모듈은 동작 전압, PWRKEY 동작 등에 차이가 있습니다.
- 따라서 Jumper 설정을 통해 인터페이스 보드에 적합한 하드웨어 설정이 선행되어야 합니다.

Arduino 플랫폼 보드 활용 시, 각 인터페이스 보드에 따른 하드웨어 설정은 다음과 같습니다.

> Arduino의 경우, 일반적으로 펌웨어 다운로드 및 디버그 용도 `D0`, `D1` UART 인터페이스를 활용합니다.
> * 시리얼 모니터를 통한 디버그 메시지의 확인이 필요한 경우, `D0`, `D1` 이외의 추가 UART를 활용 하시기 바랍니다.
>   * Arduino M0 보드 등 여러 개의 UART 포트를 지원하는 경우 `D0`, `D1` 핀을 Cat.M1과 연결하고 다른 UART로 디버그 메시지를 출력하는 방법도 있습니다.
> * 단, Arduino UNO 등 하나의 UART 포트만을 지원하는 경우에 Cat.M1 연결과 디버그 메시지 출력을 함께 수행하려는 경우, Software Serial을 활용하여 `D2`, `D8` 핀으로 Cat.M1 모듈에 연결 하시기 바랍니다. 
>   * Baudrate에 따라 올바른 출력이 이루어지지 않을 수 있습니다. Arduino Software Serial 라이브러리 관련 내용을 참고 하십시오.

#### Pin `D0` `D1`의 UART를 활용하는 경우

| WIoT-QC01 Jumper settings<bR> | WIoT-WM01 Jumper settings | WIoT-AM01 Jumper settings |
|:--------------------------:|:--------------------------:|:--------------------------:|
|![][hw-settings-arduino-qc01]|![][hw-settings-arduino-wm01]|![][hw-settings-arduino-am01]|


#### Pin `D2` `D8`의 UART를 활용하는 경우

| WIoT-QC01 Jumper settings<bR> | :heavy_check_mark: WIoT-WM01 Jumper settings | WIoT-AM01 Jumper settings |
|:--------------------------:|:--------------------------:|:--------------------------:|
|![][hw-settings-nucleo-qc01]|![][hw-settings-nucleo-wm01]|![][hw-settings-nucleo-am01]|


WIZnet IoT Shield와 Arduino Mega2560과 Uart 통신을 하기위해서는 아래와 같이 점퍼 연결이 필요합니다.
예제 구동을 위해 WIZnet IoT Shield의 UART TXD와 RXD 핀을 Arduino Mega2560 보드의 'Serial 3' `TX3`(14), `RX3`(15) 에 연결합니다.

| ArduinoMega2560 | TX3 (14)  | RX3 (15) |
|:----:|:----:|:----:|
| WIZnet IoT Shield | RXD<br>(UART Rx for D1/D8)  | TXD<br>(UART Tx for D0/D2) |

> 보드 상단에 위치한 UART_SEL 점퍼를 제거한 후 (실크 기준) 오른쪽 핀을 Arduino 보드와 연결합니다.

### 디바이스 연결

하드웨어 설정 후 USB 커넥터를 이용하여 Arduino Mega2560 Rev3 보드와 PC를 연결합니다. PC 운영체제 장치 관리자에서 장치와 연결된 COM 포트를 확인할 수 있습니다.

![][5]

- Arduino IDE를 정상적으로 설치하면, 위와 같이 장치 관리자에서 COM 포트를 확인할 수 있습니다.



<a name="Step-3-Build"></a>
## Step 3: 예제 코드 빌드 및 실행

### Arduino 예제 코드 다운로드

다음 링크에서 Arduino 예제 코드를 다운로드한 후, ino 확장자의 프로젝트 파일을 실행 시킵니다.

> 예제에서 활용할 Ping test sample code는 저장소의 아래 경로에 위치하고 있습니다.
> * `\samples\WIoT-WM01_WM-N400MSE\WIoT-WM01_Arduino_Ping\`

본 가이드에서는 우리넷 WM-N400MSE 모듈이 탑재된 WIoT-WM01 인터페이스 보드를 기준으로 설명합니다.


## 프로그램 Build와 실행
다음 그림과 같이 Arduino Mega2560 보드와 포트를 선택하고, 컴파일을 수행합니다.

![][7]

![][8]

컴파일이 완료 되면 다음과 같이 업로드를 수행하여 최종적으로 보드에 업로드를 수행 합니다.
업로드가 정상적으로 완료되면 'avrdude done. Thank you.' 메시지를 확인 할 수 있습니다.

![][9]

업로드를 완료한 후, 시리얼 모니터를 이용하여 정상적으로 Arduino Mega2560 보드에 업로드 되었는지 확인할 수 있습니다. 

![][10]


## 축하합니다
이제 여러분은 Arduino 보드 기반에서 WIZnet IoT Shield를 활용하여 LTE Cat.M1 응용을 개발할 모든 준비를 마쳤습니다!

<a name="ReadMore"></a>
## 더 보기
* [Arduino 기반의 Cat.M1 Ping 테스트 가이드][arduino-guide-wm-n400mse-ping]
* [Arduino 기반의 Cat.M1 PSM 설정 가이드][arduino-guide-wm-n400mse-psm]

 
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-arduino]: https://www.arduino.cc/
[link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
[link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3

[arduino-guide-wm-n400mse-ping]: ./Arduino_guide_wm-n400mse_ping.md
[arduino-guide-wm-n400mse-psm]: ./Arduino_guide_wm-n400mse_psm.md

[hw-settings-arduino-qc01]: ./imgs/hw/WIoT-QC01_JUMP_Arduino_serialD0_D1.png
[hw-settings-arduino-wm01]: ./imgs/hw/WIoT-WM01_JUMP_Arduino_serialD0_D1.png
[hw-settings-arduino-am01]: ./imgs/hw/WIoT-AM01_JUMP_Arduino_serialD0_D1.png

[hw-settings-nucleo-qc01]: ./imgs/hw/WIoT-QC01_JUMP_Arduino_serialD2_D8.png
[hw-settings-nucleo-wm01]: ./imgs/hw/WIoT-WM01_JUMP_Arduino_serialD2_D8.png
[hw-settings-nucleo-am01]: ./imgs/hw/WIoT-AM01_JUMP_Arduino_serialD2_D8.png

[1]: ./imgs/hw/wiot-shield-wm01-arduinomega2560_stack.png
[2]: ./imgs/
[3]: ./imgs/
[4]: ./imgs/
[5]: ./imgs/arduino_get_started_5.png
[6]: ./imgs/arduino_get_started_6.png
[7]: ./imgs/arduino_get_started_7.png
[8]: ./imgs/arduino_get_started_8.png
[9]: ./imgs/arduino_get_started_9.png
[10]: ./imgs/arduino_get_started_10.png
