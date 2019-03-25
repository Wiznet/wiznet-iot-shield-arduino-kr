# Arduino 기반의 Cat.M1 PING 테스트 가이드

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
본 문서에서는 Arduino IDE 기반 개발 환경에서 WIZnet IoT shield와 Arduino Mega2560 Rev3 보드를 이용하여 Cat.M1 단말의 Ping 테스트 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 Ping 테스트를 위한 통신 과정은 다음과 같은 순서로 구현합니다.

1. Echo 모드 설정
2. USIM 상태 확인
3. 네트워크 접속 확인
4. 네트워크 인터페이스(PDP Context) 활성화
5. Ping 테스트

<a name="Step-2-ATCommand"></a>
## AT 명령어

### 1. Echo 모드 설정

**AT Command:** ATE

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | ATE(value) | OK | ATE0<br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value) | Integer | 0 : Echo mode OFF<br>1 : Echo mode ON |

### 2. USIM 상태 확인

**AT Command:** AT$$STAT

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT$$STAT? | $$STAT:(status)<br><br>OK | AT$$STAT?<br>$$STAT:READY<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (status) | String | INSERT : USIM card inserted<br>OPEN : MSISDN NULL<br>READY : 정상적으로 Card Initialization 마친 상태 <br>MNCCARD	: 다른 사업자 USIM<br>MCCRAD : 해외 사업자 USIM<br>TESTCARD : 장비 테스트 USIM<br>ONCHIP : onChip SIM mode<br>PIN : SIM PIN, SIM PUK 등 남은 시도 횟수<br>NET PIN : PLMN ID 이외의 값을 가진 카드<br>SIM PERM BLOCK : PUK 모두 실패. 카드 교체 필요<br>SIM PIN VERIFIED : PIN code 입력 성공<br>SIM PERSO OK : Personalization unlock 성공<br>FAILURE, REMOVED : USIM removed<br>FAILURE,NO_CARD : USIM 삽입 안됨<br>FAILURE,ERROR : USIM 인식 Error<br> |

### 3. 네트워크 접속 확인

**AT Command:** AT$$CEREG

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT$$CEREG? | $$STAT:(value),(status)<br><br>OK | AT+CEREG?<br>+CEREG:0,1<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value) | Integer | 0 : 네트워크 등록 unsolicited result code를 사용하지 않음<br>1 :	네트워크 등록 unsolicited result code를 사용: +CEREG: <stat><br>2 :	네트워크 등록 및 위치정보 unsolicited result code를 사용: +CEREG: <stat>[, <tac>, <ci> [, <Act>]]<br>4 :	PSM을 적용하려는 사용자 단말(UE)에 대해 네트워크 등록 및 위치 정보 unsolicited result code를 사용: +CEREG: <stat>[,[<tac>],[<ci>],[<AcT>][,,[,[<Active-Time>],[<Periodic-TAU> ]]]] |
| (status) | Integer | 0 : 등록 되지 않음(검색하지 않음)<br>1 : 등록됨<br>2 : 등록 되지 않음(검색 중)<br>3 : 등록 거부<br>4 : Unknown<br>5 : 등록됨(로밍)<br> |


### 4. PDP Context 활성화
> PDP(Packet Data Protocol)란 단말과 외부 패킷 데이터 네트워크 사이의 데이터 송수신을 위한 연결을 제공하기 위해 사용하는 네트워크 프로토콜을 뜻하며, PDP Context는 이러한 연결 과정에서 사용되는 정보의 집합을 의미합니다.

**AT Command:** AT*RNDISDATA 

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write  | AT*RNDISDATA=(value1) | *RNDISDATA:(value1)<br><br>OK | AT\*RNDISDATA=1<br>*RNDISDATA: 1<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value) | Integer | 0 : Packet Data interface 와 연결 해제<br>1 :	Packet Data interface 와 연결 |


### 5. Ping 테스트

**AT Command:** AT*PING

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT*PING=(Host),(Count) | OK | AT*PING=8.8.8.8,2<br>OK<br>PING 64:ff9b::8.8.8.8(64:ff9b::808:808) 56 data bytes<br>64 bytes from 64:ff9b::808:808: icmp_seq=1 ttl=115 time=150 ms<br>64 bytes from 64:ff9b::808:808: icmp_seq=2 ttl=115 time=102 ms<br><br>--- 64:ff9b::8.8.8.8 ping statistics ---<br>2 packets transmitted, 2 received, 0% packet loss, time 1004ms<br>rtt min/avg/max/mdev = 102.291/126.535/150.780/24.247 ms<br>rtt min/avg/max/mdev = 102.291/126.535/150.780/24.247 ms<br>AT+WSOCR=1,www.kma.go.kr,80,1,0<br>+WSOCR:1,1,64:ff9b::794e:2229/80,TCP |


**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (Host) | String | IP address (URL, IPv4, IPv6) |
| (Count) | Integer | Ping 송신 횟수 |

<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

```
/* WM-N400MSE TCP Send/Recv */

// AT 명령어 echo 비활성화
ATE0

OK

// USIM 상태 확인 (READY면 정상)
AT$$STAT?
$$STAT:READY

OK

// 망 접속 확인 (+CEREG: 0,1 이면 접속)
AT+CEREG?
+CEREG: 0,1

OK

// PDP context 활성화
AT*RNDISDATA=1
*RNDISDATA:1

OK

// Ping 테스트
AT*PING=8.8.8.8,2
OK
PING 64:ff9b::8.8.8.8(64:ff9b::808:808) 56 data bytes
64 bytes from 64:ff9b::808:808: icmp_seq=1 ttl=115 time=150 ms
64 bytes from 64:ff9b::808:808: icmp_seq=2 ttl=115 time=102 ms

--- 64:ff9b::8.8.8.8 ping statistics ---
2 packets transmitted, 2 received, 0% packet loss, time 1004ms
rtt min/avg/max/mdev = 102.291/126.535/150.780/24.247 ms
rtt min/avg/max/mdev = 102.291/126.535/150.780/24.247 ms
AT+WSOCR=1,www.kma.go.kr,80,1,0
+WSOCR:1,1,64:ff9b::794e:2229/80,TCP

OK

```

<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project
다음 링크에서 Arduino 예제 코드를 다운로드한 후, ino 확장자의 프로젝트 파일을 실행 시킵니다.

> 예제에서 활용할 Ping test sample code는 저장소의 아래 경로에 위치하고 있습니다.
> * `\samples\WIoT-WM01_WM-N400MSE\WIoT-WM01_Arduino_Ping\`


### 2. Modify parameters

Ping 테스트 예제 코드는 별도로 수정할 Parameter가 없습니다.

### 3. Compile

상단 메뉴의 Complie 버튼을 클릭합니다.

![][compile1]

컴파일이 완료 되면 다음과 같이 업로드를 수행하여 최종적으로 보드에 업로드를 수행 합니다.
업로드가 정상적으로 완료되면 'avrdude done. Thank you.' 메시지를 확인 할 수 있습니다.

![][compile2]

### 4. Run
#### 4.1 Connect your board
스타터 키트와 Arduino Mega2560과 Uart 통신을 하기위해서는 아래와 같이 점퍼 연결이 필요합니다.
예제 구동을 위해 WIZnet IoT Shield의 UART TXD와 RXD 핀을 Arduino Mega2560 보드의 'Serial 3' `TX3`(14), `RX3`(15) 에 연결합니다.

| ArduinoMega2560 | TX3 (14)  | RX3 (15) |
|:----:|:----:|:----:|
| WIZnet IoT Shield | RXD<br>(UART Rx for D1/D8)  | TXD<br>(UART Tx for D0/D2) |

> 보드 상단에 위치한 UART_SEL 점퍼를 제거한 후 (실크 기준) 오른쪽 핀을 Arduino 보드와 연결합니다.

![][hw-stack]



#### 4.2 Set up serial monitor
![][serialMonitor]

#### 4.3 Ping test to Google DNS server
![][1]




[arduino-getting-started]: https://
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
[link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3
 
[hw-stack]: ./imgs/hw/wiot-shield-wm01-arduinomega2560_stack.png 
[compile1]: ./imgs/arduino_guide_ide_compile.png
[compile2]: ./imgs/arduino_guide_ide_compile_finish.png
[serialMonitor]: ./imgs/arduino_guide_ide_serialmonitor.png

[1]: ./imgs/Arduino_guide_wm-n400mse_ping_1.png