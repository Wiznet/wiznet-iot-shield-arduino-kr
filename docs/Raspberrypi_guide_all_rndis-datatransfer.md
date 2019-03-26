# RaspberryPi와 Cat.M1 RNDIS를 활용한 사진 전송 가이드

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [주요 소스 코드](#Step-2-SourceCode)
-   [동작 영상](#Step-3-Video)
-   [속도 검증](#Step-4-verification)

<a name="Prerequisites"></a>
## 시작하기 전에

> * 하드웨어 설정과 개발환경 구축은 **[Raspberry Pi 기반으로 Cat.M1 디바이스 개발 시작하기][raspberry-pi-getting-started]** 문서에 상세히 설명되어 있습니다.

> * Cat.M1과 같은 Cellular IoT 디바이스는 통신 서비스 사업자의 운영 기준 및 규정에 따라 모듈 펌웨어 및 동작 방식에 차이가 있을 수 있습니다. 본 문서는 한국 **[SK Telecom Cat.M1 서비스][skt-iot-portal]** 를 기준으로 작성되었습니다.


### Development Environment
* **[Raspbian OS][link-raspbian]**
* **Python 2.72**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------|
| [Raspberry Pi 2 Model B][link-raspberry-pi purchase] | WIoT-QC01 (BG96)<br>WIoT-WM01 (WM-N400MSE)<br>WIoT-AM01 (AMM5918K) |

<a name="Step-1-Overview"></a>
## 소개

일반적으로 LPWAN(Low-Power Wide-Area Network)은 저전력 장거리 통신이라는 특성으로 인해 Data Rate이 낮으며, 아래 표와 같이 LPWAN 통신 방식들은 최대 12bps~ 375Kbps의 전송 속도를 지원합니다.

LTE Cat.M1의 경우 Sigfox, LoRa, NB-IoT 대비 높은 전송 속도를 지원 가능한 특성 덕분에 실시간 트래킹 서비스 또는 사진 전송, 생체정보 전송과 같은 중용량 및 실시간 응용에 사용하기 적합합니다.

![][1]

본 가이드에서는 Raspberry Pi와 WIZnet IoT Shield Cat.M1 보드를 기반으로 다음 내용을 다룹니다.
* Cat.M1 망의 이미지 데이터 전송을 위한 Raspberry Pi 클라이언트 및 PC GUI 프로그램 구현
* RNDIS 모드를 활용한 Cat.M1 망의 데이터 전송 성능 측정


<a name="Step-2-SourceCode"></a>
## 주요 소스 코드

### 1. Raspberry Pi Client

Raspberry Pi의 이미지 전송 Client는 Python 2.72 버전으로 구현하였습니다.

국내 SK Telecom과에서 서비스 중인 Cat.M1의 경우 IPv6 주소만 사용하므로 서버 IPv4 주소를 다음과 같이 변환하여 사용해야 합니다.

```python
import socket
from os.path import exists
import time,datetime
import os

HOST = '64:ff9b::de62:adcb'             // Prefix: 64:ff9b
# HOST = '222.98.163.203'               // 각 IP 주소를 Hexa값으로 변환 (222->de)
```

전체 소스 코드의 동작은 Raspberry Pi Camera를 이용하여 사진을 찍은 후, 지정된 IP의 서버로 이미지 파일을 전송하도록 구현되어 있습니다.

```python
os.system("raspistill -o test.jpg -t 500 -q 5")       // 사진 촬영

filename = "test.jpg"                                 // 보낼 파일 이름
fileSize = os.path.getsize("test.jpg")                // 파일 사이즈 출력
print("File size is : ", str(fileSize))
fileInfo = filename + ","+str(fileSize)
sendFileToServer(fileInfo)                            // sendFileToServer 메소드를 이용하여 파일 전송

```

### 2. PC GUI Program
PC 환경에서 이미지 데이터를 수신 할 서버를 구현합니다.

데이터 수신 서버는 pyQt를 이용하여 GUI 프로그램으로 구현 하였으며, `Server Open` 버튼을 눌러 동작을 시작합니다. 

프로그램 상단의 Progress status bar를 통해 Raspberry Pi + WIZnet IoT Shield 에서 보내 온 이미지 데이터의 수신 현황을 알 수 있으며, 이미지도 함께 확인 할 수 있도록 구성하였습니다.

![][2]

아래와 소스와 같이 TCP 서버는 pyQT 관련 동작과 독립적으로 수행할 수 있도록 Thread로 실행하였습니다.

```python
def runServer():
    print('[Server Start]')
    print("[To terminate, Press 'Ctrl + C']")

    try:
        server = socketserver.TCPServer((HOST,PORT),MyTcpHandler)
        #server.serve_forever()
        threading.Thread(target=server.serve_forever).start()
    except KeyboardInterrupt:
        print('[Terminate]')
```

이미지 및 진행 상태 막태의 Refresh를 위해서 Signal-pyqtSlot() 기능을 사용하였습니다.

```python
class MySignal(QtCore.QObject):
    signal1 = QtCore.pyqtSignal(QtCore.QObject)
    def run(self):
        self.signal1.emit(self)

class MySignal2(QtCore.QObject):
    signal2 = QtCore.pyqtSignal(QtCore.QObject)
    def run(self):
        self.signal2.emit(self)
```

```python
@pyqtSlot()
def imageRefresh(self):
    #self.setupUi.label.setText("첫번째 버튼")
    im = Image.open('download/test.jpg')
    size = (500,500)
    im.thumbnail(size)
    im.save('download/test.jpg')
    self.label.setPixmap(QPixmap("download/test.jpg"))

@pyqtSlot()
def resetProgressbar(self):
    while progressBar_statusValue < 100:
        self.progressBar.setValue(progressBar_statusValue)
    self.progressBar.setValue(100)
```

전체 소스 코드는 아래 링크에서 확인 할 수 있습니다.
> GitHub Repository: **[LTECatM1-RaspberryPi-Photo-Transfer-Application][link-github]**

<a name="Step-3-Video"></a>
## 동작 영상

동작 영상은 **[Youtube 데모 동영상][link-youtube]** 을 참고해 주기시 바랍니다.

![][3]

<a name="Step-4-Verification"></a>
## 속도 검증

 아래 표와 같은 형태로 총 50번 테스트를 진행하였으며, 평균 데이터 전송률은 298.37Kbps 입니다.
 해당 테스트 결과는 LTE Cat.M1의 이론 상 전송 속도의 약 80% 정도 수준이며, 다른 LPWAN 통신 방식에 비해 월등한 속도 차이를 보입니다.

![][4]


 [raspberry-pi-getting-started]: ./Raspberrypi_get_started.md
 [skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
 [link-woorinet]: http://www.woori-net.com
 [link-wiznet]: https://www.wiznet.io
 [link-raspbian]: https://www.raspberrypi.org/downloads/raspbian/
 [link-raspberry-pi purchase]: https://www.raspberrypi.org/products/raspberry-pi-2-model-b/
 [link-github]: https://github.com/stkim92/LTECatM1-RaspberryPi-Photo-Transfer-Application
 [link-youtube]: https://youtu.be/xJbm8BLaym0

 [1]: ./imgs/Raspberry_pi_guide_picture_1.PNG
 [2]: ./imgs/Raspberry_pi_guide_picture_2.png
 [3]: ./imgs/Raspberry_pi_guide_picture_3.png
 [4]: ./imgs/Raspberry_pi_guide_picture_4.PNG
