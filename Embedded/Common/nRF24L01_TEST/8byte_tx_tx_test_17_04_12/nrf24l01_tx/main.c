#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL

#define CS1 PORTB |=  (1<<0);
#define CS0 PORTB &= ~(1<<0);
#define CE1 PORTB |=  (1<<5);
#define CE0 PORTB &= ~(1<<5);

#define T1 PORTD |=  (1<<0);
#define T0 PORTD &= ~(1<<0);

/* Memory Map */
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define OBSERVE_TX  0x08
#define CD          0x09
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
#define RX_ADDR_P2  0x0C
#define RX_ADDR_P3  0x0D
#define RX_ADDR_P4  0x0E
#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_P0    0x11
#define RX_PW_P1    0x12
#define RX_PW_P2    0x13
#define RX_PW_P3    0x14
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17

/* 컨트롤 명령 */
#define R_REGISTER    0x00
#define W_REGISTER    0x20
#define REGISTER_MASK 0x1F
#define R_RX_PAYLOAD  0x61
#define W_TX_PAYLOAD  0xA0
#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2
#define REUSE_TX_PL   0xE3
#define ACTIVATE      0x50 
#define R_RX_PL_WID   0x60
#define NOP           0xFF

volatile unsigned char flag = 1, status_now;
volatile unsigned int j = 2;
unsigned char spi_WR(unsigned char data) {
    SPDR = data;
    while((SPSR & 0x80) == 0);
    return SPDR;
}

unsigned char Write_1b(unsigned char addr, unsigned char data) {
	CS0; // 쓰기 시작
	spi_WR(W_REGISTER | addr);
	spi_WR(data);
	CS1;
}

void write_addr(unsigned char addr,
				unsigned char data1,
				unsigned char data2,
				unsigned char data3,
				unsigned char data4,
				unsigned char data5)
{
	CS0; // 쓰기 시작
	spi_WR(W_REGISTER | addr);
	spi_WR(data1);
	spi_WR(data2);
	spi_WR(data3);
	spi_WR(data4);
	spi_WR(data5);
	CS1;
}

unsigned char Read_1b(unsigned char addr) {
	unsigned char byte;
	CS0; // 쓰기 시작
	spi_WR(R_REGISTER | addr);
	byte = spi_WR(0x00);
	CS1;
	return byte;
}
void Cmd(unsigned char cmd) {
	CS0; // 쓰기 시작
	spi_WR(cmd);
	CS1;
}
void Transmit_1b(unsigned char data) {
	CS0;
	spi_WR(W_TX_PAYLOAD);
	spi_WR(data);
	CS1;
}
void Transmit_nb(unsigned char n, unsigned char* datapointer) {
	char i;
	CS0;
	spi_WR(W_TX_PAYLOAD);
	for(i = 0; i<n; i++)
		spi_WR(*(datapointer + i));
	CS1;
}
unsigned char Receive_1b( void ) {
	unsigned char tmp;
	CS0;
	spi_WR(R_RX_PAYLOAD);
	tmp = spi_WR(0x00);
	CS1;
	return tmp;
}
void Receive_nb( unsigned char n, unsigned char* datapointer ) {
	char i;
	CS0;
	spi_WR(R_RX_PAYLOAD);
	for(i = 0; i<n; i++)
		datapointer[i] = spi_WR(0x00);
	CS1;
}

/* ========================= 초기화 함수 ==================== */

void spi_init(void)
{		/* SPI 설정보다 포트 입출력이 먼저 정의되어야 SPI 통신이 정상적으로 됨. */
	DDRB = 0b00100111; // CS, SCK, MOSI, CE 출력 포트로 사용
	SPCR = 0b01010000; // spi 설정
}

void RF_init(void)
{
	CE0;
	Write_1b(RF_CH, 0x01); // RF 사용채널 지정 

	Write_1b(RX_PW_P0, 0x00);
	Write_1b(RX_PW_P1, 0x08); // 파이프라인 1번 수신, 2바이트
	Write_1b(RX_PW_P2, 0x00);
	Write_1b(RX_PW_P3, 0x00);
	Write_1b(RX_PW_P4, 0x00);
	Write_1b(RX_PW_P5, 0x00);

	Write_1b(RF_SETUP, 0x06); // 출력 최대, 속도 1Mbps
	Write_1b(CONFIG, 0b01001000); // CRC 사용
	Write_1b(EN_AA, 0b00000011); // 데이터 파이프 0,1번 auto ack 사용
	Write_1b(EN_RXADDR, 0b00000011); // 데이터 파이프라인 0,1번 사용
	Write_1b(SETUP_RETR, 0x2f); // 750us동안 재전송, 15번 재전송 시도

	Cmd(FLUSH_TX); // tx fifo 비우기
	Cmd(FLUSH_RX); // rx fifo 비우기

	Write_1b(STATUS, 0b01110000); // rx_dr, tx_ds, max_rt 초기화

}

volatile char rx[8];

int main(void) {
	DDRA = 0xff;
	DDRD = 0xff;
	spi_init();
	_delay_ms(11);
	RF_init();
	CE0;
	unsigned int i = 1;

//	TCCR0 = 0b00000111;

	//int7 하강엣지
	EICRB = 0b10000000;
	EIMSK = 0b10000000;
	sei();

	write_addr(TX_ADDR, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7);
	write_addr(RX_ADDR_P0, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7);
	write_addr(RX_ADDR_P1, 0xD7, 0xD7, 0xD7, 0xD7, 0xD7);

	Write_1b(CONFIG, 0b01001010); // TX 모드
	_delay_ms(2);
	Cmd(FLUSH_RX); // RX fifo 비우기
	Cmd(FLUSH_TX); // TX fifo 비우기	
	_delay_ms(1000);

	for(;;){
		if(i == 0xff)
			i = 0;
		CE0;
		Write_1b(CONFIG, 0b01001010); // TX 모드
		Write_1b(STATUS, 0b01110000);
		Cmd(FLUSH_TX); // TX fifo 비우기
		Transmit_1b(i++); // 데이터 전송
		CE1;
		flag = 1;
		while(flag);
		_delay_ms(100);
	}
	return 0;   /* never reached */
}

ISR(INT7_vect) { //데이터 송신이 완료된다면? 혹은 데이터 전송을 실패 한다면???????
	cli();
	status_now = Read_1b(STATUS);
	CE0; // standby
	if ((status_now & 0x20) == 0x20) { // 데이터 전송을 성공한다면		
		Write_1b(STATUS, 0b01110000); // rx_dr, tx_ds, max_rt 초기화
		Write_1b(CONFIG, 0b01111011); // RX 모드	
		Cmd(FLUSH_RX); // rx fifo 비우기			
		_delay_us(300);
		CE1; // rx 모드 시작
		T1;
		TCCR0 = 0b00000111; // timer active		
		while((TCNT0 <= 0xfe) & ( (Read_1b(STATUS) & 0x40) != 0x40 ) ); // 16.384ms동안 대기하거나 (timeout) 데이터가 수신되면
		CE0; // standby모드
		T0;
		TCNT0 = 0x00; // counter init
		TCCR0 = 0x00; // timer disable
		Write_1b(STATUS, 0b01110000); // rx_dr, tx_ds, max_rt 초기화
		Receive_nb(8,rx); // 데이터 받기
		PORTA = rx[4];
		Cmd(FLUSH_RX); // rx fifo 비우기
	} else if ((status_now & 0x10) == 0x10) { // 데이터 전송을 실패한다면
		Write_1b(STATUS, 0b01110000); // rx_dr, tx_ds, max_rt 초기화
	} else {
		PORTA = 0xff;
		Write_1b(STATUS, 0b01110000); // rx_dr, tx_ds, max_rt 초기화
	}
	flag = 0; // rx 루틴 플래그
	sei();
}
