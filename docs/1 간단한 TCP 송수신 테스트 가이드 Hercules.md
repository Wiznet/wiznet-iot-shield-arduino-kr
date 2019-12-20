<h1 id="서버-클라이언트-테스트-방법-(tcp/ip,-hercules)"><a name="서버-클라이언트-테스트-방법-(tcp/ip,-hercules)" href="#서버-클라이언트-테스트-방법-(tcp/ip,-hercules)"></a>서버 클라이언트 테스트 방법 (TCP/IP, Hercules)</h1>
<h2 id="준비물"><a name="준비물" href="#준비물"></a>준비물</h2>
<h4 id="1.-pc-(혹은-노트북)"><a name="1.-pc-(혹은-노트북)" href="#1.-pc-(혹은-노트북)"></a>1. PC (혹은 노트북)</h4>
<h4 id="2.-hercules-프로그램-[https://www.hw-group.com/software/hercules-setup-utility]("https://www.hw-group.com/software/hercules-setup-utility")"><a name="2.-hercules-프로그램-[https://www.hw-group.com/software/hercules-setup-utility]("https://www.hw-group.com/software/hercules-setup-utility")" href="#2.-hercules-프로그램-[https://www.hw-group.com/software/hercules-setup-utility]("https://www.hw-group.com/software/hercules-setup-utility")"></a>2. Hercules 프로그램 <a href="&quot;https://www.hw-group.com/software/hercules-setup-utility&quot;"><a href="https://www.hw-group.com/software/hercules-setup-utility">https://www.hw-group.com/software/hercules-setup-utility</a></a></h4>
<h4 id="3.-테스트-기기-(pc,-cat.m1-과-같은-tcp/ip를-지원하는-모든-디바이스)"><a name="3.-테스트-기기-(pc,-cat.m1-과-같은-tcp/ip를-지원하는-모든-디바이스)" href="#3.-테스트-기기-(pc,-cat.m1-과-같은-tcp/ip를-지원하는-모든-디바이스)"></a>3. 테스트 기기 (PC, Cat.m1 과 같은 TCP/IP를 지원하는 모든 디바이스)</h4>
<hr class="section">
<hr class="page">
<h2 id="체크리스트"><a name="체크리스트" href="#체크리스트"></a>체크리스트</h2>
<h4 id="네트워크-환경"><a name="네트워크-환경" href="#네트워크-환경"></a>네트워크 환경</h4>
<h5 id="->pc와-테스트-기기가-접근-가능한-네트워크(혹은-동일-네트워크)에-있는지-확인"><a name="->pc와-테스트-기기가-접근-가능한-네트워크(혹은-동일-네트워크)에-있는지-확인" href="#->pc와-테스트-기기가-접근-가능한-네트워크(혹은-동일-네트워크)에-있는지-확인"></a>-&gt;PC와 테스트 기기가 접근 가능한 네트워크(혹은 동일 네트워크)에 있는지 확인</h5>
<hr class="section">
<hr class="page">
<h2 id="pc에-tcp/ip-서버-생성하기"><a name="pc에-tcp/ip-서버-생성하기" href="#pc에-tcp/ip-서버-생성하기"></a>PC에 TCP/IP 서버 생성하기</h2>
<h4 id="step-1.-네트워크-환경-확인"><a name="step-1.-네트워크-환경-확인" href="#step-1.-네트워크-환경-확인"></a>Step 1. 네트워크 환경 확인</h4>
<h5 id="->-pc(서버)-및-테스트-기기-ip-확인"><a name="->-pc(서버)-및-테스트-기기-ip-확인" href="#->-pc(서버)-및-테스트-기기-ip-확인"></a>-&gt; PC(서버) 및 테스트 기기 IP 확인</h5>
<ul>
<li><p>테스트 기기가 LTE Cat.m1과 같이 통신사 망(외부 네크워크)을 이용하는 경우<br>— PC(서버) IP도 통신사 망에서 할당받은 IP만 바로 사용 가능하다.<br>— 통신사 망에서 할당받은 IP가 아닌 공유기 IP(내부 네트워크, 주로 192.xxx.xxx.xxx)를 사용하는 경우, 공유기에서 포트포워딩을 하여야만 사용 가능하다. </p>
</li><li><p>PC와 테스트 기기가 내부 네트워크(공유기)를 이용하는 경우<br>— 동일한 게이트웨이와 네트워크 대역이면, 바로 사용 가능하다.<br>— PC와 테스트 기기 그리고 게이트웨이 IP 주소가 A.B.C.D 인 경우 D를 제외한 앞의 3개의 주소 A.B.C 가 같아야 한다.<br>— Ex) PC(192.168.11.3), 테스트 기기(192.168.11.5), 게이트웨이(192.168.11.1)</p>
</li></ul>
<hr class="section">
<h4 id="step-2.-hercules를-이용해-서버-만들기-(pc)"><a name="step-2.-hercules를-이용해-서버-만들기-(pc)" href="#step-2.-hercules를-이용해-서버-만들기-(pc)"></a>Step 2. Hercules를 이용해 서버 만들기 (PC)</h4>
<h5 id="->-license-information-(close)"><a name="->-license-information-(close)" href="#->-license-information-(close)"></a>-&gt; License information (Close)</h5>
<p><img src="https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/imgs/Hercules%20License%20information.png?raw=true &quot;Hercules License Information" alt="Hercules License Information"></p>
<h5 id="->-tcp-server-정보-입력-후-서버-생성"><a name="->-tcp-server-정보-입력-후-서버-생성" href="#->-tcp-server-정보-입력-후-서버-생성"></a>-&gt; TCP Server 정보 입력 후 서버 생성</h5>
<h5 id="->-아래-그림과-같이,-hercules를-실행-->-tcp-server-click-->-server-port-설정-후,-(그림에는-없지만)-server-port-설정-옆-listen-버튼을-click-하면,-tcp-server가-생성됩니다."><a name="->-아래-그림과-같이,-hercules를-실행-->-tcp-server-click-->-server-port-설정-후,-(그림에는-없지만)-server-port-설정-옆-listen-버튼을-click-하면,-tcp-server가-생성됩니다." href="#->-아래-그림과-같이,-hercules를-실행-->-tcp-server-click-->-server-port-설정-후,-(그림에는-없지만)-server-port-설정-옆-listen-버튼을-click-하면,-tcp-server가-생성됩니다."></a>-&gt; 아래 그림과 같이, Hercules를 실행 -&gt; TCP Server Click -&gt; Server Port 설정 후, (그림에는 없지만) Server Port 설정 옆 Listen 버튼을 Click 하면, TCP Server가 생성됩니다.</h5>
<p><img src="https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/imgs/Hercules%20TCP%20Server%20Contents.png?raw=true" alt="Hercules TCP Server" title="Hercules TCP Server Contents"> </p>
<hr class="section">
<h4 id="step-3.-테스트-기기에서-tcp/ip-서버-접속"><a name="step-3.-테스트-기기에서-tcp/ip-서버-접속" href="#step-3.-테스트-기기에서-tcp/ip-서버-접속"></a>Step 3. 테스트 기기에서 TCP/IP 서버 접속</h4>
<p>-&gt; 아래 내용과 같이 테스트 기기를 TCP Client로 설정하여, PC(서버)의 ip 주소와 Server Port로 접속하면 됩니다. </p>
<h5 id="테스트-기기가-pc인-경우"><a name="테스트-기기가-pc인-경우" href="#테스트-기기가-pc인-경우"></a>테스트 기기가 PC인 경우</h5>
<p>-&gt; Hercules TCP Client 를 이용하여 PC(서버)에 접속할 수 있습니다. </p>
<h5 id="cat.m1인-경우"><a name="cat.m1인-경우" href="#cat.m1인-경우"></a>Cat.m1인 경우</h5>
<p>-&gt; 일반적인 TCP Connection AT Command를 사용하여 PC(서버)에 접속할 수 있습니다. </p>
<h5 id="스마트폰인-경우"><a name="스마트폰인-경우" href="#스마트폰인-경우"></a>스마트폰인 경우</h5>
<p>-&gt; TCP Client 클라이언트 스마트폰 앱을 설치하여 테스트를 진행할 수 있습니다. </p>
<p><img src="https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/imgs/Smart%20Phone%20TCP%20Client%20Set.png?raw=true" alt="SmartPhone TCP Client" title="SmartPhone App TCP Client"> </p>
<h4 id="step-4.-데이터-송수신-테스트-결과"><a name="step-4.-데이터-송수신-테스트-결과" href="#step-4.-데이터-송수신-테스트-결과"></a>Step 4. 데이터 송수신 테스트 결과</h4>
<h5 id="->-테스트-기기에서-pc(서버)로-데이터를-송신하면,-pc(서버)-에-아래-내용과-같이-수신한-정보를-확인할-수-있다.-또한-서버에서-테스트-기기로-데이터를-송신할-수-있습니다."><a name="->-테스트-기기에서-pc(서버)로-데이터를-송신하면,-pc(서버)-에-아래-내용과-같이-수신한-정보를-확인할-수-있다.-또한-서버에서-테스트-기기로-데이터를-송신할-수-있습니다." href="#->-테스트-기기에서-pc(서버)로-데이터를-송신하면,-pc(서버)-에-아래-내용과-같이-수신한-정보를-확인할-수-있다.-또한-서버에서-테스트-기기로-데이터를-송신할-수-있습니다."></a>-&gt; 테스트 기기에서 PC(서버)로 데이터를 송신하면, PC(서버) 에 아래 내용과 같이 수신한 정보를 확인할 수 있다. 또한 서버에서 테스트 기기로 데이터를 송신할 수 있습니다.</h5>
<p>-&gt; 테스트 기기에서 PC(서버)로 데이터 송신의 예</p>
<p><img src="https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/imgs/TCP%20Client%20Send.png?raw=true" alt="Send to Server" title="Send to Server">  </p>
<p>-&gt; PC(서버)에서 테스트 기기로 데이터 송신의 예</p>
<p><img src="https://github.com/Wiznet/wiznet-iot-shield-kr/blob/master/docs/imgs/TCP%20Server%20Send.png?raw=true" alt="Server Reply" title="TCP Server Send">  </p>
<h4 id="작성자:-matthew"><a name="작성자:-matthew" href="#작성자:-matthew"></a>작성자: Matthew</h4>
<h4 id="연락처:-matthew@wiznet.io"><a name="연락처:-matthew@wiznet.io" href="#연락처:-matthew@wiznet.io"></a>연락처: matthew@wiznet.io</h4>
