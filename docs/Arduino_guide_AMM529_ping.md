# Arduino 기반의 Cat.M1 Ping 테스트 가이드

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
| Write | ATE[n] | OK | ATE0<br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| [n] | Integer | 0 : Echo mode OFF<br>1 : Echo mode ON |

### 2. USIM 상태 확인

**AT Command:** AT+CPIN

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT+CPIN? | +CPIN:[code]<br><br>OK | AT+CPIN?<br>+CPIN:READY<br><br>OK |

**Defined values:**


| Parameter | Type | Description |
|:--------|:--------|:--------|
| [code] | String | READY: PIN1 코드 조합이 끝난 상태<br> SIM PIN: PIN1 코드 조합 대기 <br> SIM PUK: PIN Lock 상태. PIN Lock 해제 코드 [PUK] %입력 대기 <br> |

### 3. 네트워크 접속 확인

**AT Command:** AT+CEREG

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT+CEREG=[n] | OK | AT+CEREG=0<br>OK |
| Read | AT+CEREG? | +CEREG: [n],[stat]<br>OK | AT+CEREG?<br>+CEREG:0,1<br>OK |
| Read | AT+CEREG=?| +CEREG: (list of supported [n]s) | AT+CEREG=?<br>+CEREG: (0-2,4)<br>OK|


**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| [n] | Integer | 0 : 알림하지 않습니다. <br>	1 :	+CEREG[stat]의 자동알림을 유효하게 합니다. 네트워크 등록 상태가 변화했을 경우, 외부장치에 알립니다. <br>	2 :	+CEREG[stat][[[tac],[rac_mme],[ci],[act]]의 자동알림을 유효하게 합니다. 네트워크 등록 상태가 변했을 경우, 외부장치에 알립니다. <br> 4: +CEREG: [stat][,[[tac],[[ci],[[AcT][,,[,[[Active-Time]],[[Periodic TAU]]의 PSM을 사용하는 단말의 자동알림을 유효하게 합니다. 네트워크 등록상태가 변했을 경우, 외부장치에 알립니다. 
| [stat] | Integer | 0 : 등록 되지 않음(검색하지 않음)<br>1 : 등록됨<br>2 : 등록 되지 않음(검색 중)<br>3 : 등록 거부<br>4 : Unknown<br>5 : 등록됨(로밍)<br> |
| [tac] | Max 1024 char | Tracking Area Code 값으로 Hexadecimal 형식입니다. |
| [rac_or_mme] | Max 1024 char | RAC 값 혹은 MME 값을 표기합니다. Hexadecimal 형식입니다. |
| [ci] | Max 1024 char | Cell ID 값을 표기합니다. Hexadecimal 형식입니다. |
| [act] | Integer | Access Technology 값을 표기합니다. <br>0: GSM access technology<br>1: GSM Compact access technology<br>2:UMTS access technology<br> 3:EGPRS access technology<br> 4: HSDPA access technology<br> 5:HSUPA access technology <br> 6: HSDPA & HSUPA access technology <br> 7: LTE access technology <br> 8: LTE M1 (A/Gb) access technology<br> 9: LTE NB (S1) access technology <br> 10: Unspecified access technology |
| [Active-Time] | One byte in an 8 bit format | Active Time value를 표시합니다.<br> 3Bit: (단위) <br> -"000" 2초<br>-"001" 1분<br>-"010" 6분<br>5Bit: 시간 <br><br>예) "00100100" = 4분 |
|[Periodic-TAU]|One byte in an 8 bit format | Extended periodic TAU 를 표시합니다. <br>3Bit: (단위) <br> -"000" 10분 <br>-"001" 1시간 <br>-"010" 10시간 <br>-"011" 2초 <br>-"101" 1분 <br>-"110" 320 시간 <br>5Bit: 시간<br><br>예) "01000111" = 70 시간|

### 4. PDP Context 활성화
> PDP(Packet Data Protocol)란 단말과 외부 패킷 데이터 네트워크 사이의 데이터 송수신을 위한 연결을 제공하기 위해 사용하는 네트워크 프로토콜을 뜻하며, PDP Context는 이러한 연결 과정에서 사용되는 정보의 집합을 의미합니다.

####CGATT, NETCONN 확인 필요

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

**AT Command:** AT@PING6

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT@PING6="[IPv6 Addr] -c[count]" | OK | AT@PING6="2001:4860:4860::8888 -c 8"<br><br>OK<br>PING<br>2001:4860:4860::8888(2001:4860:4860::8888) 56 data bytes<br>64 bytes from 2001:4860:4860::8888: icmp_seq=1 ttl=49 time=391 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=2 ttl=49 time=102 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=3 ttl=49 time=123 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=4 ttl=49 time=99.9 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=5 ttl=49 time=143 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=6 ttl=49 time=80.8 ms<br>80<br>64 bytes from 2001:4860:4860::8888: icmp_seq=7 ttl=49 time=118 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=8 ttl=49 time=76.0 ms<br><br><br>--- 2001:4860:4860::8888 ping statistics ---<br>8 packets transmitted, 8 received, 0% packet loss, time 7011ms<br>rtt min/avg/max/mdev = 76.066/142.028/391.598/96.569 ms |


**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| [IPv6 Addr] | String | Host IP address (URL, IPv6) |
| [count] | Integer | Ping 송신 횟수 |

ps. IPv4 (기존에 많이 사용되고 있는 IP 주소) 의 경우, IPv6로 주소를 변형하여야 합니다. (64:ff9b:: Header Use)
ex) IPv4 주소 123.123.123.123 의 경우 -> HEX로 변환 7B.7B.7B.7B
    ->64:ff9b::7b7b:7b7b

<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

```
/* WM-N400MSE TCP Send/Recv */

// AT 명령어 echo 비활성화
ATE0

OK

// USIM 상태 확인 (READY면 정상)
AT+CPIN?
+CPIN:READY

OK

// 망 접속 확인 (+CEREG: 0,1 이면 접속)
AT+CEREG?
+CEREG: 0,1

OK

###확인해야할 부분
// PDP context 활성화
AT*RNDISDATA=1
*RNDISDATA:1

OK

###확인해야할 부분

// Ping 테스트
AT@PING6="2001:4860:4860::8888 -c 8"
@PING6:ping6 2001:4860:4860::8888


OK
PING 
2001:4860:4860::8888(2001:4860:4860::8888) 56 data bytes
64 bytes from 2001:4860:4860::8888: icmp_seq=1 ttl=49 time=329 ms
64 bytes from 2001:4860:4860::8888: icmp_seq=2 ttl=49 time=121 ms
64 bytes from 2001:4860:4860::8888: icmp_seq=3 ttl=49 time=137 ms
64 bytes from 2001:4860:4860::8888: icmp_seq=4 ttl=49 time=101 ms

--- 2001:4860:4860::8888 ping statistics ---
4 packets transmitted, 4 received, 0% packet loss, time 3005ms
rtt min/avg/max/mdev = 101.432/172.622/329.946/91.726 ms

```

<a name="Step-4-Build-and-Run"></a>




[arduino-getting-started]: ./Arduino_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
[link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3
 
[hw-stack]: ./imgs/hw/wiot-shield-wm01-arduinomega2560_stack.png 
[compile1]: ./imgs/arduino_guide_ide_compile.png
[compile2]: ./imgs/arduino_guide_ide_compile_finish.png
[serialMonitor]: ./imgs/arduino_guide_ide_serialmonitor.png

[1]: ./imgs/Arduino_guide_wm-n400mse_ping_1.png
