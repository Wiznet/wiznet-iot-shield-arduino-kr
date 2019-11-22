#서버 클라이언트 테스트 방법 (TCP/IP, Hercules)


## 준비물

####1. PC (혹은 노트북)

####2. Hercules 프로그램 [https://www.hw-group.com/software/hercules-setup-utility]("https://www.hw-group.com/software/hercules-setup-utility")

####3. 테스트 기기 (PC, Cat.m1 과 같은 TCP/IP를 지원하는 모든 디바이스)

---
***

## 체크리스트

#### 네트워크 환경

#####->PC와 테스트 기기가 접근 가능한 네트워크(혹은 동일 네트워크)에 있는지 확인

---
***

## PC에 TCP/IP 서버 생성하기

####Step 1. 네트워크 환경 확인

#####-> PC(서버) 및 테스트 기기 IP 확인

* 테스트 기기가 LTE Cat.m1과 같이 통신사 망(외부 네크워크)을 이용하는 경우 
-- PC(서버) IP도 통신사 망에서 할당받은 IP만 바로 사용 가능하다. 
-- 통신사 망에서 할당받은 IP가 아닌 공유기 IP(내부 네트워크, 주로 192.xxx.xxx.xxx)를 사용하는 경우, 공유기에서 포트포워딩을 하여야만 사용 가능하다. 

* PC와 테스트 기기가 내부 네트워크(공유기)를 이용하는 경우
-- 동일한 게이트웨이와 네트워크 대역이면, 바로 사용 가능하다. 
-- PC와 테스트 기기 그리고 게이트웨이 IP 주소가 A.B.C.D 인 경우 D를 제외한 앞의 3개의 주소 A.B.C 가 같아야 한다. 
-- Ex) PC(192.168.11.3), 테스트 기기(192.168.11.5), 게이트웨이(192.168.11.1)

---


####Step 2. Hercules를 이용해 서버 만들기 (PC)

#####-> License information (Close)

![Hercules License Information](https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/imgs/Hercules%20License%20information.png?raw=true "Hercules License Information)

#####-> TCP Server 정보 입력 후 서버 생성



#####-> 아래 그림과 같이, Hercules를 실행 -> TCP Server Click -> Server Port 설정 후, (그림에는 없지만) Server Port 설정 옆 Listen 버튼을 Click 하면, TCP Server가 생성됩니다. 


![Hercules TCP Server](https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/imgs/Hercules%20TCP%20Server%20Contents.png?raw=true "Hercules TCP Server Contents") 


---


####Step 3. 테스트 기기에서 TCP/IP 서버 접속

-> 아래 내용과 같이 테스트 기기를 TCP Client로 설정하여, PC(서버)의 ip 주소와 Server Port로 접속하면 됩니다. 

#####테스트 기기가 PC인 경우

-> Hercules TCP Client 를 이용하여 PC(서버)에 접속할 수 있습니다. 

#####Cat.m1인 경우

-> 일반적인 TCP Connection AT Command를 사용하여 PC(서버)에 접속할 수 있습니다. 


#####스마트폰인 경우

-> TCP Client 클라이언트 스마트폰 앱을 설치하여 테스트를 진행할 수 있습니다. 

![SmartPhone TCP Client](https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/imgs/Smart%20Phone%20TCP%20Client%20Set.png?raw=true "SmartPhone App TCP Client") 


####Step 4. 데이터 송수신 테스트 결과

#####-> 테스트 기기에서 PC(서버)로 데이터를 송신하면, PC(서버) 에 아래 내용과 같이 수신한 정보를 확인할 수 있다. 또한 서버에서 테스트 기기로 데이터를 송신할 수 있습니다. 

-> 테스트 기기에서 PC(서버)로 데이터 송신의 예

![Send to Server](https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/imgs/TCP%20Client%20Send.png?raw=true "Send to Server")  

-> PC(서버)에서 테스트 기기로 데이터 송신의 예

![Server Reply](https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/imgs/TCP%20Server%20Send.png?raw=true "TCP Server Send")  


####작성자: Matthew
####연락처: matthew@wiznet.io














