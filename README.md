# Network 상 전원 제어 가능 멀티탭 IoT 전력량계

2017년 공모전 출품을 목적으로 작업했던 IoT 프로젝트를 재 정리한 자료입니다. 프로젝트에 대한 상세한 설명은 [해당 링크](http://www.eeic.or.kr/www/2017/capstoneAwardList.do?type=view&yearawardsId=72&year=2017)에서 열람할 수 있습니다.

## 1. 동작 환경

> 웹 서버 : Ubuntu on Raspberry PI 3

## 2. 주요 기술

### 1. 서버

> 웹 서버 : Ubuntu on Raspberry PI 3
>
> 웹앱 : PHP + HTML5(jQuery) + SQLite
>
> IoT 수신기 데이터 처리 : Python (Serial 통신을 통한 자료 처리 + DB 접근)

#### 1.1 IoT 콘센트 수신기

> > MCU : ATmege8 (@16Mhz) (Atmel)
> >
> > RF : nRF24L01 (Nordic Semiconductor)

### 2. IoT 콘센트

> 전력 측정 : ADE7753 (Analog Devices)
>
> MCU : ATTiny441 (@8Mhz) (Atmel)
>
> RF : nRF24L01 (Nordic Semiconductor)