# WIZnet IoT Shield for Arduino

이 저장소에는 다음 자료들이 포함되어 있습니다.
* [Arduino 플랫폼][link-arduino] 기반으로 Cat.M1 응용 개발 환경을 구축하는 **개발 시작 가이드**
* Cat.M1 장치의 TCP 데이터 송수신, SMS, GPS, SKT Thingplug 서비스 연동 등에 대한 **가이드 문서**
* [Arduino IDE][link-arduino-compiler] 상에서 동작하는 **예제 코드**

![][hw-stack-arduinomega]

## Folder Structure

### /docs
Arduino 플랫폼을 기반으로 WIZnet IoT shield의 Cat.M1 통신을 활용하는 응용 장치 구현 시 참고할 수 있는 다양한 문서가 위치합니다.
* imgs: 문서에서 활용된 이미지들이 저장된 폴더입니다.

### /samples
Arduino IDE에서 컴파일 및 업로드 하여 보드의 동작을 확인 할 수 있는 예제 코드가 위치합니다. 각 Cat.M1 모듈 벤더 별로 나뉘어 있으며, AT command 기반으로 구현되어 있습니다.
* WIoT-QC01: 앰투앰넷 BG96(Quectel) 모듈의 예제 코드
* WIoT-WM01: 우리넷 WM-N400MSE 모듈의 예제 코드
* WIoT-AM01: AM텔레콤 AMM5918K 모듈의 예제 코드

## Key features and Roadmap

:heavy_check_mark: 활용 가능  :heavy_multiplication_x: 지원 예정이며, 준비 중  :heavy_minus_sign: 지원 예정 없음

### Common Docs
|        Title       |         Doc        |                            Description                           |
|:------------------:|:------------------:|:----------------------------------------------------------------|
| [Quickstart Guide](https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/quickstartguide_standalone_mode.md) | :heavy_check_mark: | Standalone 모드를 활용한 Cat.M1 모듈 테스트 |
| [Hardware Guide](https://github.com/Wiznet/wiznet-iot-shield-hardware-kr/blob/master/docs/wiot_hw_overview_n_settings.md) | :heavy_check_mark: | WIoT 제품군의 하드웨어 구성 및 설정 가이드 |
| [SKT Thingplug Guide](https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/thingplug_guide_common.md) | :heavy_check_mark: | Cat.M1 장치의 SKT Thingplug 연동 가이드 (공통)  |


### Arduino Cat.M1 Samples and Docs

> * [QC] WIoT-QC01 예제 코드
> * [WM] WIoT-WM01 예제 코드
> * [AM] WIoT-AM01 예제 코드

|        Title       |         Doc        |      QC     |      WM     |       AM      |                            Description                           |
|:------------------:|:------------------:|:------------------:|:------------------------:|:------------------------:|:----------------------------------------------------------------|
| 개발 시작 가이드   | :heavy_check_mark: | :heavy_minus_sign: |    :heavy_minus_sign:    |  :heavy_minus_sign:    | 초기 개발환경 구축                                               |
| Ping 테스트        | :heavy_check_mark: | :heavy_check_mark: |  :heavy_check_mark:    | :heavy_multiplication_x:    | 네트워크 상의 지정된 목적지와 Ping 테스트 예제                   |
| TCP/IP 구현하기 | :heavy_check_mark: | :heavy_check_mark: |  :heavy_multiplication_x:  | :heavy_multiplication_x:    | TCP client로 동작하여 데이터 송신 및 수신하는 예제               |
| HTTP 구현하기 | :heavy_check_mark: | :heavy_check_mark: |  :heavy_multiplication_x:  | :heavy_multiplication_x:    | HTTP client로 동작하여 데이터 송신 및 수신하는 예제               |
| MQTT 구현하기 | :heavy_check_mark: | :heavy_check_mark: |  :heavy_multiplication_x:  | :heavy_multiplication_x:    | MQTT client로 동작하여 데이터 송신 및 수신하는 예제               |
| GPS 활용하기       | :heavy_check_mark: | :heavy_check_mark: |  :heavy_multiplication_x:    | :heavy_multiplication_x:    | Cat.M1 모듈에 내장된 GPS 위치 측위 예제                          |
| SMS 활용하기       | :heavy_check_mark: | :heavy_check_mark: |  :heavy_check_mark:  | :heavy_multiplication_x: | Cat.M1 모듈에서 지원하는 SMS(단문 메시지 서비스)의 활용 예제     |
| PSM 저전력 모드    | :heavy_check_mark: | :heavy_check_mark: |    :heavy_check_mark:    | :heavy_multiplication_x: | Cat.M1 모듈에서 지원하는 저전력 모드(PSM) 활용 예제              |
| ThingPlug 활용 | :heavy_multiplication_x: | :heavy_multiplication_x: | :heavy_multiplication_x: | :heavy_multiplication_x: | SKT의 클라우드 서비스 ThingPlug 연동 예제와 Widget 활용 가이드 |

### Other Arduino Samples
|        Title       |     Code      |                            Description                           |
|:------------------:|:------------------:|:----------------------------------------------------------------|
| [WIoT-Shield_SENSOR_CDS](https://github.com/Wiznet/wiznet-iot-shield-arduino-kr/tree/master/samples/WIoT-Shield/WIoT-Shield_SENSOR_CDS) | :heavy_check_mark: | IoT Shield에 탑재된 CDS(조도센서) 활용 예제 (Pin: A0) |
| [WIoT-Shield_SENSOR_TEMP](https://github.com/Wiznet/wiznet-iot-shield-arduino-kr/tree/master/samples/WIoT-Shield/WIoT-Shield_SENSOR_TEMP) | :heavy_check_mark: | IoT Shield에 탑재된 [LM35](http://www.ti.com/lit/ds/symlink/lm35.pdf)(온도센서) 활용 예제 (Pin: A1) |

## Documentation
개발 시작 가이드 및 응용 가이드 문서는 본 저장소의 **[Wiki](https://github.com/Wiznet/wiznet-iot-shield-arduino-kr/wiki/)** 에서 확인 할 수 있습니다.


## Other WIZnet IoT Shield Repository
**[wiznet-iot-shield](https://github.com/Wiznet/wiznet-iot-shield-kr/)** 저장소를 방문하면 다른 플랫폼 보드를 기반으로 동작하는 다양한 예제에 대한 저장소 리스트를 확인 할 수 있습니다.


## Support

[![WIZnet Developer Forum][forum]](https://forum.wiznet.io/c/korean-forum/oshw/)

**[WIZnet Developer Forum](https://forum.wiznet.io/c/korean-forum/oshw/)** 에서 전세계의 WIZnet 기술 전문가들에게 질문하고 의견을 전달할 수 있습니다.<br>지금 방문하세요!

## License
**WIZnet IoT Shield for Arduino** 저장소의 모든 문서와 예제는 [Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0)으로 배포됩니다.


[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-arduino]: https://www.arduino.cc/
[link-arduino-compiler]: https://www.arduino.cc/en/Main/Software
[link-arduino Mega2560 Rev3]: https://store.arduino.cc/usa/mega-2560-r3

[hw-stack-arduinomega]: ./docs/imgs/hw/wiot-shield-wm01-arduinomega2560_stack.png

[forum]: ./docs/imgs/forum.jpg


