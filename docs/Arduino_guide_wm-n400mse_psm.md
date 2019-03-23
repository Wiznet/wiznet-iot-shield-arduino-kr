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
| [Arduino Mega2560 Rev3][link-arduino Mega2560 Rev3] | WIoT-WM01 (WM-N400MSE) |

<a name="#Step-1-Overview"></a>
## 소개

IoT 응용에서 배터리로 동작을 하는 디바이스의 경우, 디바이스가 소모하는 전류는 디바이스 수명과 직접적인 관계가 있으므로 소모 전류를 최소화 하는 것은 매우 중요합니다. 이와 같은 이유로 Cat.M1은 PSM과 Enhanced DRX(이하 eDRX)기술을 지원하여 소모하는 전류를 크게 줄일 수 있도록 합니다. PSM을 사용하면 디바이스의 Active/Sleep 상태를 조절하여 소모하는 전력을 줄일 수 있습니다.

본 문서에서는 Arduino IDE 기반 개발 환경에서 WIZnet IoT shield와 Arduino Mega2560 Rev3 보드를 이용하여 Cat.M1 단말의 PSM(Power Saving Moe) 설정 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 PSM 설정 위한 통신 과정은 다음과 같은 순서로 구현합니다.

1. PSM 설정
2. 모듈 및 모뎀 상태 조회
3. 모듈 및 모뎀 재부팅


<a name="#Step-2-ATCommand"></a>
## AT 명령어

### 1. PSM 설정
PSM을 사용하기 위해 **AT+CPSMS** 명령어를 사용하며, 파라미터 설정을 통해 Active/Sleep 주기를 조절할 수 있습니다.
해당 명령의(Requested_Periodic-TAU)와 (Requested_Active-Time)의 시간을 설정함으로써 Active/Sleep 주기를 조절합니다. 주기의 단위를 설정하기 위해 6-8bits를 설정하며 값을 설정하기 위해 1-5bits값을 설정합니다. 예를 들어 (Requested_Periodic-TAU)를 1010011로 설정하였다면, 6-8bits가 101이므로 단위는 1분이며 1-2bit자리만 설정되었으므로 값은 6이고, 따라서 6분으로 설정한 것입니다.

**AT Command:** AT+CPSMS

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write Command | AT+CPSMS=[<Mode>[,(Requested_Periodic-RAU)[,(Requested_GPRS-READY-timer)[,(Requested_Periodic-TAU)[,(Requested_ActiveTime)]]]]] | OK | AT+CPSMS=1,,,”00000100”,”00001111”<br>OK |


**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (Mode) | Integer | 0 : PSM 해제<br>1 : PSM 설정<br>2 : PSM 해제 및 설정 초기화|
| (Requested_Periodic-RAU) | Integer | 000□□□□□ : (6-8 bits: 10분)   \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 10시간)  \* (1-5 bits: 이진수 값)<br>011□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>100□□□□□ : (6-8 bits: 30초)    \* (1-5 bits: 이진수 값)<br>101□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>110□□□□□ : (6-8 bits: 320시간) \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |
| (Requested_GPRS-READY-time) | Integer | 000□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |
| (Requested_Periodic-TAU) | Integer | 000□□□□□ : (6-8 bits: 10분)   \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 10시간)  \* (1-5 bits: 이진수 값)<br>011□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>100□□□□□ : (6-8 bits: 30초)    \* (1-5 bits: 이진수 값)<br>101□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>110□□□□□ : (6-8 bits: 320시간) \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |
| (Requested_Active-Time) | Integer | 000□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |

### 2. 모듈 및 모뎀 상태 조회
**AT$$DBS** 명령의 응답 중, (PSM-ACTIVE), (PSM-PERIODIC) 파라미터 값으로  Active/Sleep 시간을 계산할 수 있습니다.

**AT Command:** AT$$DBS

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT$$DBS | $$DBS:<br>[LTE]<br>(DL E-ARFCN)<br>(BAND)<br>(DL_BANDWIDTH)<br>(PLMN)<br>(NAS_REJECT_CAUSE)<br>(PCI)<br>(CELL_ID<br>(TAC<br>(ESM_CAUSE)<br>(EMM_RRC_STATE)<br>(EMM_STATE)<br>(EMM_SUBSTATE)<br>(RSSI)<br>(RSRP)<br>(RSRQ)<br>(SINR)<br>(TX_POWER)<br>(NUM_ANTENA)<br>(TMSI)<br>(SRV_STATUS)<br>(NBR_CELL_INFO)<br>(DRX)<br>(L2W)<br>(RI)<br>(CQI)<br>(EDRX)<br>(EDRX-PTW)<br>(EDRX-CycLen)<br>(PSM)<br>(PSM-ACTIVE)<br>(PSM-PERIODIC)<br> | AT$$DBS<br>$$DBS:<br>[LTE]<br>DL E-ARFCN:2500-20500<br>BAND:5<br>DL_BANDWIDTH:0<br>PLMN:45012<br>NAS_REJECT_CAUSE:FFH<br>PCI:448<br>CELL_ID:2294-35<br>TAC:8473<br>ESM_CAUSE:FFH<br>EMM_RRC_STATE:IDLE<br>EMM_STATE:REGISTRED<br>EMM_SUBSTATE:NORMAL<br>RSSI:-125<br>RSRP:0<br>RSRQ:0<br>SINR:-20<br>TX_POWER:0<br>NUM_ANTENA:0<br>TMSI:0000007F<br>SRV_STATUS:Available<br>NBR_CELL_INFO:N/A<br>DRX:1280<br>L2W:120<br>RI: - <br>CQI: - <br>EDRX:Disabled<br>EDRX-PTW:0<br>EDRX-CycLen:0<br>PSM:Enabled<br>PSM-ACTIVE:60s<br>PSM-PERIODIC:180s<br><br><br>OK<br> |


### 3. 디바이스 재부팅
PSM을 사용하기 위해 위의 AT 명령어를 이용하여 설정 하였다면 재부팅하여야 해당 설정이 저장됩니다.

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
## 동작 구조 예제
Active/Sleep 상태 진입 시점을 판단하기 위해서는 AT 명령어 주기적으로 보내고, 응답을 확인해야 하며 아래의 예제는 PSM-ACTIVE를 60초로 설정하고 PSM-PERIODIC을 180초로 설정하여 테스트한 결과입니다.
```
/* WM-N400MSE PSM 설정 */

// PSM 설정 (PSM-ACTIVE:60s, PSM-PERIODIC:180s)
AT+CPSMS=1,,,"10000110","00100001"
OK

// 네트워크 상태 조회
AT$$DBS
$$DBS:
[LTE]
DL E-ARFCN:2500-20500
BAND:5
DL_BANDWIDTH:0
PLMN:45012
NAS_REJECT_CAUSE:FFH
PCI:448
CELL_ID:2294-35
TAC:8473
ESM_CAUSE:FFH
EMM_RRC_STATE:IDLE
EMM_STATE:REGISTRED
EMM_SUBSTATE:NORMAL
RSSI:-125
RSRP:0
RSRQ:0
SINR:-20
TX_POWER:0
NUM_ANTENA:0
TMSI:0000007F
SRV_STATUS:Available
NBR_CELL_INFO:N/A
DRX:1280
L2W:120
RI: - 
CQI: - 
EDRX:Disabled
EDRX-PTW:0
EDRX-CycLen:0
PSM:Enabled
PSM-ACTIVE:60s
PSM-PERIODIC:180s


OK

// PSM 적용을 위해 모듈 재부팅
AT*SKT*RESET
*SKT*RESET:1

OK

```


Active/Sleep 상태는 **AT$$MSTIME?** 명령어를 이용하여 현재 시간을 받아오는 것으로 확인 하였습니다. 아래의 테스트는 PSM-ACTIVE을 60s, PSM-PERIODIC을 180s로 설정하여 테스트한 결과 입니다. **AT$$MSTIME?**의 응답으로서 현재 시간을 받는 시점을 Active Start로 표기하였고, 현재 시간을 받지 못하는 시점을 Sleep Start로 표기하였습니다. 

```
[WM-N400MSE] Current Time: 16h 24m 9s
[WM-N400MSE] Current Time: 16h 24m 10s
[WM-N400MSE] Current Time: 16h 24m 11s <-- Sleep Start
    .                                      Sleep Time  : 2m 14s
	.       
[WM-N400MSE] Current Time: 16h 26m 25s <-- Active Start
    .                                      Active Time : 1m 25s
	.  
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
<a name="#Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project
다음 링크에서 Arduino 예제 코드를 다운로드한 후, ino 확장자의 프로젝트 파일을 실행 시킵니다.

> 예제에서 활용할 Ping test sample code는 저장소의 아래 경로에 위치하고 있습니다.
> * `\samples\WIoT-WM01_WM-N400MSE\WIoT-WM01_Arduino_PSM\`

### 2. Modify parameters

PSM 설정을 위해  테스트 예제 코드는 별도로 수정할 Parameter가 없습니다.

### 3. Compile

상단 메뉴의 Complie 버튼을 클릭합니다.

![][1]


컴파일이 완료 되면 다음과 같이 업로드를 수행하여 최종적으로 보드에 업로드를 수행 합니다.
업로드가 정상적으로 완료되면 'avrdude done. Thank you.' 메시지를 확인 할 수 있습니다.

![][2]


### 4. Run
#### 4.1 Connect your board
스타터 키트와 Arduino Mega2560과 Uart 통신을 하기위해서는 아래와 같이 점퍼 연결이 필요합니다.
예제 구동을 위해 WIZnet IoT Shield의 UART TXD와 RXD 핀을 Arduino Mega2560 보드의 'Serial 3' `TX3`(14), `RX3`(15) 에 연결합니다.

| ArduinoMega2560 | TX3 (14)  | RX3 (15) |
|:----:|:----:|:----:|
| WIZnet IoT Shield | RXD<br>(UART Rx for D1/D8)  | TXD<br>(UART Tx for D0/D2) |
> 보드 상단에 위치한 UART_SEL 점퍼를 제거한 후 (실크 기준) 오른쪽 핀을 Arduino 보드와 연결합니다.


![][hw-stack]

#### 4.2 Set up serial terminal
![][4]


#### 4.3 Print current time to check an Active/Sleep state
![][5]





 [arduino-getting-started]: ./Arduino_get_started.md
 [skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
 [link-woorinet]: http://www.woori-net.com
 [link-wiznet]: https://www.wiznet.io
 [link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
 [link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3

 [hw-stack]: ./imgs/hw/wiot-shield-wm01-arduinomega2560_stack.png
 [1]: ./imgs/Arduino_guide_wm-n400mse_psm_1.png
 [2]: ./imgs/Arduino_guide_wm-n400mse_psm_2.png
 [3]: ./imgs/Arduino_guide_wm-n400mse_psm_3.png
 [4]: ./imgs/Arduino_guide_wm-n400mse_psm_4.png
 [5]: ./imgs/Arduino_guide_wm-n400mse_psm_5.png
