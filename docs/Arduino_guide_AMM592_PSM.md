# Arduino 기반의 Cat.M1 PSM 설정 가이드

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
| [Arduino Mega2560 Rev3][link-arduino Mega2560 Rev3] | WIoT-AM01 (AMM592SK) |

<a name="Step-1-Overview"></a>

## 소개

IoT 응용에서 배터리로 동작을 하는 디바이스의 경우, 디바이스가 소모하는 전류는 디바이스 수명과 직접적인 관계가 있으므로 소모 전류를 최소화 하는 것은 매우 중요합니다. 이와 같은 이유로 Cat.M1은 PSM과 Enhanced DRX(이하 eDRX)기술을 지원하여 소모하는 전류를 크게 줄일 수 있도록 합니다. PSM을 사용하면 디바이스의 Active/Sleep 상태를 조절하여 소모하는 전력을 줄일 수 있습니다.

본 문서에서는 Arduino IDE 기반 개발 환경에서 WIZnet IoT shield와 Arduino Mega2560 Rev3 보드를 이용하여 Cat.M1 단말의 PSM(Power Saving Moe) 설정 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 PSM 설정 위한 통신 과정은 다음과 같은 순서로 구현합니다.

1. PSM 설정
2. 모듈 및 모뎀 상태 조회
3. 모듈 및 모뎀 재부팅

<a name="Step-2-ATCommand"></a>

## AT 명령어

### 1. PSM 설정

PSM을 사용하기 위해 **AT@PSMSET** 명령어를 사용하며, 파라미터 설정을 통해 Active/Sleep 주기를 조절할 수 있습니다.

해당 명령의(Requested_Periodic-TAU)와 (Requested_Active-Time)의 시간을 설정함으로써 Active/Sleep 주기를 조절합니다. 주기의 단위를 설정하기 위해 6-8bits를 설정하며 값을 설정하기 위해 1-5bits값을 설정합니다. 예를 들어 (Requested_Periodic-TAU)를 1010011로 설정하였다면, 6-8bits가 101이므로 단위는 1분이며 1-2bit자리만 설정되었으므로 값은 6이고, 따라서 6분으로 설정한 것입니다.

**AT Command:** AT@PSMSET

**Syntax:**

| Type | Syntax | Response | Example |
|:--------|:--------|:--------|:--------|
| Write Command | AT@PSMSET=(psm_enable),(active_timer),(psm_timer) | AT@PSMSET:(psm_enable),(active_timer),(psm_timer)<br><br>OK | AT@PSMSET=1,240,300<br>@PSMSET:1,240,300<br>OK <br><br>OR<br><br>AT@PSMSET=0<br>@PSMSET:0 |
|Write Command | AT@PSMSET? | 위와 동일 | AT@PSMSET?<br>@PSMSET:1,240,300<br><br>OK|

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (psm_enable) | Integer | 초기값: 0 <br>0 : PSM OFF<br>1 : PSM ON<br>|
| (active_timer) | Integer | 사용 가능 범위(0 ~ 62), (120 ~ 1860), (2160 ~ 11160)<br>(0 ~ 62): 2의 배수 값 사용 <br> (120 ~ 1860): 60의 배수 값 사용 <br> (2160 ~ 11160): 360의 배수 값 사용 |
| (psm_timer) | Integer | 사용 가능 범위(0 ~ 62), (120 ~ 1860), (2400 ~ 18600), (21600 ~ 111600), (144000 ~ 1116000)<br>(0 ~ 62): 2의 배수 값 사용 <br> (120 ~ 1860): 60의 배수 값 사용 <br> (2400 ~ 18600): 600의 배수 값 사용 <br> (21600 ~ 111600): 3600의 배수 값 사용 <br> (144000 ~ 1116000): 360000의 배수 값 사용 |



### 2. 모듈 및 모뎀 상태 조회

**AT$$DBS** 명령의 응답 중, (PSM-ACTIVE), (PSM-PERIODIC) 파라미터 값으로  Active/Sleep 시간을 계산할 수 있습니다.

**AT Command:** AT$$DBS

**Syntax:**

| Type | Syntax | Response | Example |
|:--------|:--------|:--------|:--------|
| Read | AT@DBG | @DBG:OP:(operator_mode), MODE:(mode_pref), SERVICE:(service_status), SIM:(sim_status), EMM state:(emm), EMM Cause:(emm_cause), ESM Cause:(esm_cause), [SYS:(system_mode)[, NB OP Mode:(nb_operator_mode)],[PLMN:(plmn)],[ANTBAR:(ant_bar), Band:(band), BW:(bandwidth), TAC:(tac), Cell-ID:(cellid), EARFCN:(earfcn), PCI:(pci), GUTI:(guti), DRX:(drx), CDRX short:(cdrx_short), CDRX long:(cdrx_long), AVG RSRP:(avr_rsrp), RSRP:(rsrp), RSRQ:(rsrq), RSSI:(rssi), SINR:(sinr), Tx Power:(tx_pwr), L2W:(l2w), CQI:(cqi), RRC state:(rrc_state)[, PSM enable:(psm_enable)[, PSM active time:(active_timer), PSM periodic update time:(psm_timer)], eDRX enable:(edrx)[, PTW:(ptw), Cycle len:(cycle)]]][,(ip_addr_list)]]<br>OK| M1 In Service : <br> @DBG:OP:ONLINE, MODE:LTE ONLY, SERVICE:IN SRV, SIM:SKT USIM, EMM state:REGISTERED, EMM Cause:0, ESM Cause:0, SYS:LTE-M1, PLMN:45012, ANTBAR:4, Band:5, BW:10MHz, TAC:2059, Cell-ID:5c4-23, EARFCN:2500, PCI:165, GUTI:450f12-8031-81-c7780000, DRX:1280, CDRX short:0, CDRX long:0, AVG RSRP:-87, RSRP:-87, RSRQ:-8, RSSI:-61, SINR:5.6, Tx Power:-, L2W:-, CQI:0, RRC state:IDLE, PSM enable:0, eDRX enable:0, <br>(1)IPv4:10.198.241.68,(1)IPv6:2001:02d8:13b0:415e:0000:0000:bc8b:3802<br><br>M1 No Service<br>@DBG:OP:ONLINE, MODE:LTE ONLY, SERVICE:PWR SAVE, SIM:SKT USIM, EMM state:DEREGISTERED, EMM Cause:0, ESM Cause:0, SYS:NO SRV|


###확인해야할 사항, 예제 돌리면서 확인해 보아야 함!
### 3. 디바이스 재부팅

PSM을 사용하기 위해 위의 AT 명령어를 이용하여 설정 하였다면 재부팅하여야 해당 설정이 저장됩니다.

**AT Command:** AT\*SKT\*RESET

**Syntax:**

| Type | Syntax | Response | Example |
|:--------|:--------|:--------|:--------|
| Write  | AT\*SKT*RESET | \*SKT*RESET:(Result)<br><br>OK | AT\*SKT\*RESET<br>\*SKT*RESET:1<br><br>OK<br> |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (Result) | Integer | 0 : 실패<br>1 : 성공 |

<a name="Step-3-SampleCode"></a>

## 동작 구조 예제

Active/Sleep 상태 진입 시점을 판단하기 위해서는 AT 명령어 주기적으로 보내고, 응답을 확인해야 하며 아래의 예제는 PSM active_timer를 240초로 설정하고 PSM psm_timer를 300초로 설정하여 테스트한 결과입니다.

```

/* AMM592SK PSM 설정 */

// PSM 설정 (active_timer:240s, psm_timer:300s)
AT@PSMSET=1,240,300
@PSMSET:1,240,300

OK

// 네트워크 상태 조회
AT@DBG
@DBG:OP:ONLINE< MODE:LTE ONLY, SERVICE:IN SRV, SIM:SKT USIM, EMM state:REGISTERED, EMM Cause:0, ESM Cause:0, SYS:LTE-M1, PLMN:45012, ANTBAR:4, Band:5, BW:10MHz, TAC:2059, Cell-ID:5c4-23, EARFCN:2500, PCI:165, GUTI:450f12-8031-81-c7780000, DRX:1280, CDRX short0, CDRX long:0, AVG RSRP:-87, RSRP:-87, RSRQ:-8, RSSI:-61, SINR:5.6, Tx Power:-, L2W:-, CQI:0, RRC state:IDLE, PSM enable:0, eDRX enable:0,
(1)IPv4:10.198.241.68,(1)IPv6:2001:02d8:13b0:415e:0000:0000:bc8b:3802

// 재부팅은 확인해보아야함!!
// PSM 적용을 위해 모듈 재부팅
AT*SKT*RESET
*SKT*RESET:1

OK
```
###예제 작성하면서 디버깅 메시지를 통해 추후 검토
Active/Sleep 상태는 **AT$$MSTIME?** 명령어를 이용하여 현재 시간을 받아오는 것으로 확인 하였습니다. 아래의 테스트는 PSM-ACTIVE을 60s, PSM-PERIODIC을 180s로 설정하여 테스트한 결과 입니다. **AT$$MSTIME?**의 응답으로서 현재 시간을 받는 시점을 Active Start로 표기하였고, 현재 시간을 받지 못하는 시점을 Sleep Start로 표기하였습니다. 

```
[WM-N400MSE] Current Time: 16h 24m 9s
[WM-N400MSE] Current Time: 16h 24m 10s
[WM-N400MSE] Current Time: 16h 24m 11s <-- Sleep Start
    .                                      Sleep Time  : 2m 14s	.       

[WM-N400MSE] Current Time: 16h 26m 25s <-- Active Start
    .                                      Active Time : 1m 25s	.  

[WM-N400MSE] Current Time: 16h 27m 50s <-- Sleep Start
    .                                      Sleep Time  : 2m 13s
    .
[WM-N400MSE] Current Time: 16h 30m 3s  <-- Active Start
    .                                      Active Time : 1m 29s
    .  
[WM-N400MSE] Current Time: 16h 31m 32s <-- Sleep Start
    .                                      Sleep Time  : 2m 13s
    .  
[WM-N400MSE] Current Time: 16h 33m 45s <-- Active Start
    .                                      Active Time : 1m 26s
    .  
[WM-N400MSE] Current Time: 16h 35m 11s <-- Sleep Start
    .                                      Sleep Time  : 2m 15s
    .  
[WM-N400MSE] Current Time: 16h 37m 26s <-- Active Start
    .                                      Active Time : 1m 25s
    .  
[WM-N400MSE] Current Time: 16h 38m 51s <-- Sleep Start

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
int8_t setPsmActivate_AMM592(char * Requested_Periodic_TAU, char * Requested_Active_Time);
```
* PSM TAU와 Active time의 설정 및 기능 활성화를 수행합니다.
* 파라메터는 "10010101", "00100100" 형태의 문자열입니다.

```cpp
int8_t setPsmDeactivate_AMM592(void);
```
* PSM 기능을 비활성화 하는 함수입니다.

```cpp
int8_t getPsmSetting_AMM592(bool * enable, int * Requested_Periodic_TAU, int * Requested_Active_Time);
```
* PSM 설정 정보를 확인합니다. 설정 정보를 저장할 변수들을 파라메터로 받아 활성화 여부 및 TAU, Active time을 돌려줍니다.
* 확장 PSM 설정 명령어 기반으로 구현되어 시간 정보들을 초 단위로 리턴합니다.


샘플 코드의 네트워크 시간 정보 관련 함수는 다음과 같습니다.

```cpp
int8_t getNetworkTimeLocal_AMM592(char * timestr);
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

[1]: ./imgs/arduino_guide_am01_psm-1.png
[2]: ./imgs/arduino_guide_am01_psm-2.png
[3]: ./imgs/arduino_guide_am01_psm-3.png

