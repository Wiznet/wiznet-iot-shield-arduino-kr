# Raspberry Pi 기반으로 Cat.M1 디바이스 개발 시작하기

## 목차

-   [소개](#Introduction)
-   [Step 1: 필수 구성 요소](#Step-1-Prerequisites)
-   [Step 2: 디바이스 준비](#Step-2-PrepareDevice)
-   [Step 3: Raspberry Pi 상에서 Cat.M1 연결 확인](#Step-3-ConnectConfirm)



<a name="Introduction"></a>

## 소개
Raspberry Pi를 이용하면 Cat.M1 모듈의 RNDIS(Remote Network Driver Interface Specification) 기능을 사용할 수 있습니다. RNDIS는 주로 USB 인터페이스를 통해 사용할 수 있으며, 대부분의 Window, Linux, FreeBSD 등 운영체제에 대한 가상 이더넷 링크를 제공합니다.

데스크 탑 PC에서 무선 인터넷을 사용하기 위해 무선 랜카드를 PC USB 포트에 연결하는 것을 예로 들 수 있습니다.이와 같이 Cat.M1 모듈 및 외장형 모뎀을 RNDIS 형태로 사용하면 연결된 기기에서 Cat.M1 망을이용할 수 있습니다.

UART 인터페이스를 통해 AT 커맨드를 이용하여 데이터 송수신을 하면 UART 인터페이스 속도의 제한이 있기 때문에 Cat.M1이 제공하는 최대 속도를 사용할 수 없습니다. 그러나, RNDIS 기능을 활용할 경우 USB 인터페이스를 사용하기 때문에 Cat.M1 이 제공하는 최대 속도인 375kbps 와 근접하게 사용할 수 있습니다.


**문서의 주요 내용**

이 문서는 **[Raspberry Pi][link-raspberry-pi]** 를 기반으로 Cat.M1 장치의 개발 환경 구축 과정에 대해 설명합니다.

각 과정에는 다음 내용들이 포함되어 있습니다:
- WIZnet IoT Shield와 Raspberry Pi 하드웨어 설정
- Cat.M1 모듈 소프트웨어 설정
- Raspberry Pi 상에서 Cat.M1 통신 기능 확인 과정


## Step 1: 필수 구성 요소
이 문서를 따라하기 전에 다음과 같은 것들이 준비되어야 합니다.

- **Cat.M1 디바이스 개발용 하드웨어**
  - **[Raspberry Pi 2 Model B][link-raspberry-pi purchase]**
  - WIZnet IoT Shield
  - Cat.M1 Interface Board (택 1)
    - WIoT-QC01 (앰투앰넷 BG96) :heavy_check_mark:
    - WIoT-WM01 (우리넷 WM-N400MSE) :heavy_check_mark:
    - WIoT-AM01 (AM텔레콤 AMM5918K)

- **Cat.M1 모듈의 (시험 망)개통**
  - Cat.M1 모듈로 통신 기능을 구현하려면 **망 개통 과정** 이 선행되어야 합니다.
    - 한국의 경우, 국내 Cat.M1 서비스 사업자인 SK Telecom의 망 개통 과정이 필요합니다.

> 모듈은 개발 단계에 따라 시험망 개통 - 상용망 개통 단계를 거쳐야 하며 외장형 모뎀은 즉시 상용망 개통이 가능합니다.
> * 시험망 개통의 경우 **[SKT IoT OpenHouse][skt-iot-openhouse]** 에 기술 지원 문의
> * 상용망 개통의 경우 USIM 구매 대리점이나 디바이스 구매처에 개통 문의

<a name="Step-2-PrepareDevice"></a>
## Step 2: 디바이스 준비

### 하드웨어 설정

WIZnet IoT Shield를 Raspberry Pi 하드웨어와 연결합니다.
- Raspberry Pi와 연결하기 위해서는 Cat.M1 모듈을 RNDIS 모드로 설정해야 하며, RNDIS 모드로 사용할 경우에는 IoT Shield의 `P2 USB HOST` 포트를 사용합니다.

![][hw-raspberrypi-connect-qc01]


IoT Shield는 다양한 밴더의 Cat.M1 모듈을 활용 할 수 있도록 하드웨어 설정을 제공합니다. 따라서 선택한 Cat.M1 Interface Board를 확인하여 장치 설정이 필요합니다.

- 각각 밴더의 모듈은 동작 전압, PWRKEY 동작 등에 차이가 있습니다.
- 따라서 Jumper 설정을 통해 인터페이스 보드에 적합한 하드웨어 설정이 선행되어야 합니다.

Raspberry Pi와 WIZnet IoT Shield를 연결하는 경우, 인터페이스 보드에 따른 하드웨어 설정은 다음과 같습니다.


| :heavy_check_mark: WIoT-QC01 Jumper settings<bR> | WIoT-WM01 Jumper settings | WIoT-AM01 Jumper settings |
|:--------------------------:|:--------------------------:|:--------------------------:|
|![][hw-settings-nucleo-qc01]|![][hw-settings-nucleo-wm01]|![][hw-settings-nucleo-am01]|

>  * 본 가이드에서는 RNDIS 모드 접속 활성화를 목적으로 하므로, USB와 IoT shield를 연결합니다. 때문에 보드 Jumper 중 UART 연결 Pin을 설정하는 `UART_SEL`은 설정하지 않아도 무방하며, 영향을 끼치지 않습니다.
>
>  * 단 인터페이스 보드의 모듈 별로 동작 전압과 PWRKEY 운용 방식 등에 차이가 있으므로, 반드시 `PWRKEY_SEL` `MODULE_UART` Jumper를 설정 해 주십시오.



![][hw-usb-port]

> * 이미지를 기준으로 오른쪽에 위치한 `P2 USB HOST` 포트를 Raspberry Pi와 연결합니다.



### Cat.M1 모듈의 RNDIS 모드 설정

#### 1. WIoT-QC01 RNDIS 설정

WIoT-QC01의 경우 별도의 설정이 필요 없습니다.


#### 2. WIoT-WM01 RNDIS 설정

Cat.M1 모듈을 RNDIS 모드로 사용하기 위해서는 다음과 같은 AT 명령어를 통해 설정이 필요하며 설정 저장을 위하여 반드시 재부팅이 필요합니다.

##### 2.1 RNDIS 모드 설정

**AT Command:** AT\*RNDISMODE

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write  | AT\*RNDISMODE=(Mode) | \*RNDISMODE:(Mode)<br><br>OK | AT\*RNDISMODE=1<br>\*RNDISMODE=1<br><br>OK<br> |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (Mode) | Integer | 0 : RNDISMODE 해제<br>1 : RNDISMODE 설정 |



##### 2.2 디바이스 재부팅

**AT Command:** AT\*SKT\*RESET

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write  | AT\*SKT*RESET | \*SKT*RESET:(Result)<br><br>OK | AT\*SKT\*RESET<br>\*SKT*RESET:1<br><br>OK<br> |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (Result) | Integer | 0 : 실패<br>1 : 성공 |


<a name="#Step-3-SampleCode"></a>

##### 2.3 WIoT-WM01 RNDIS 동작 구조 예제

```
/* WM-N400MSE RNDIS 모드 설정 */

// RNDIS 모드 설정
AT*RNDISMODE=1
*RNDISMODE=1

OK

// RNDIS 모드 적용을 위해 모듈 재부팅
AT*SKT*RESET
*SKT*RESET:1

OK

```

#### 3. WIoT-AM01 RNDIS 설정

> 준비 중입니다.

<a name="Step-3-ConnectConfirm"></a>

## Step 3: Raspberry Pi 상에서 Cat.M1 연결 확인

위와 같이 하드웨어 연결이 정상적으로 되었고 Cat.M1 모듈 또한 RNDIS 모드를 이용하기 위한 설정이 정상적으로 되었다면 다음과 같이 Raspberry Pi에서 Cat.M1 네트워크가 연결된 것을 확인 할 수 있습니다.

![][5]

연결된 Cat.M1 네트워크를 사용하면 인터넷 브라우저를 통해 Google 사이트와 같은 외부 인터넷 망에 접속할 수 있습니다.

![][6]


<a name="ReadMore"></a>
# 더 보기

* [Raspberry Pi 기반의 Cat.M1 RNDIS를 활용한 사진 전송 가이드][raspberrypi-guide-bg96-rndis]

[link-raspberry-pi]: https://www.raspberrypi.org/
[link-raspberry-pi purchase]: https://www.raspberrypi.org/products/raspberry-pi-2-model-b/
[skt-iot-openhouse]: https://www.sktiot.com/iot/support/openhouse/reservation/openhouseMain
[link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
[link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3

[hw-raspberrypi-connect-qc01]: ./imgs/hw/wiot-shield-qc01-raspberrypi.png
[hw-raspberrypi-connect-wm01]: ./imgs/hw/wiot-shield-wm01-raspberrypi.png
[hw-settings-nucleo-qc01]: ./imgs/hw/WIoT-QC01_JUMP_Arduino_serialD2_D8.png
[hw-settings-nucleo-wm01]: ./imgs/hw/WIoT-WM01_JUMP_Arduino_serialD2_D8.png
[hw-settings-nucleo-am01]: ./imgs/hw/WIoT-AM01_JUMP_Arduino_serialD2_D8.png
[hw-usb-port]: ./imgs/hw/wiot-shield-usbport.png

[1]: ./imgs/Raspberry_pi_get_started_1.png
[2]: ./imgs/Raspberry_pi_get_started_2.png
[3]: ./imgs/Raspberry_pi_get_started_3.png
[4]: ./imgs/Raspberry_pi_get_started_4.png
[5]: ./imgs/Raspberry_pi_get_started_5.png
[6]: ./imgs/Raspberry_pi_get_started_6.png

[raspberrypi-guide-bg96-rndis]: ./Raspberrypi_guide_all_rndis-datatransfer.md

