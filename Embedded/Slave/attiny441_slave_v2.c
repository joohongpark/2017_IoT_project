#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#define DEV_ADDR 0x03


#define F_CPU 8000000UL
#define CS1 PORTA |=  (1<<2);
#define CS0 PORTA &= ~(1<<2);
#define CE1 PORTA |=  (1<<4);
#define CE0 PORTA &= ~(1<<4);
#define ADE_CS1 PORTB |=  (1<<2);
#define ADE_CS0 PORTB &= ~(1<<2);
#define OUTPUT_high PORTB |=  (1<<0);
#define OUTPUT_low PORTB &= ~(1<<0);
#define true 1
#define false 0

//PORTA(REMAP = 0x02) = //CE/SCK/SS/MOSI/MISO

#define RF_CONFIG      0x00
#define RF_EN_AA       0x01
#define RF_EN_RXADDR   0x02
#define RF_SETUP_AW    0x03
#define RF_SETUP_RETR  0x04
#define RF_RF_CH       0x05
#define RF_RF_SETUP    0x06
#define RF_STATUS      0x07
#define RF_OBSERVE_TX  0x08
#define RF_CD          0x09
#define RF_RX_ADDR_P0  0x0A
#define RF_RX_ADDR_P1  0x0B
#define RF_RX_ADDR_P2  0x0C
#define RF_RX_ADDR_P3  0x0D
#define RF_RX_ADDR_P4  0x0E
#define RF_RX_ADDR_P5  0x0F
#define RF_TX_ADDR     0x10
#define RF_RX_PW_P0    0x11
#define RF_RX_PW_P1    0x12
#define RF_RX_PW_P2    0x13
#define RF_RX_PW_P3    0x14
#define RF_RX_PW_P4    0x15
#define RF_RX_PW_P5    0x16
#define RF_FIFO_STATUS 0x17

// 컨트롤 명령
#define RF_R_REGISTER    0x00
#define RF_W_REGISTER    0x20
#define RF_REGISTER_MASK 0x1F
#define RF_R_RX_PAYLOAD  0x61
#define RF_W_TX_PAYLOAD  0xA0
#define RF_FLUSH_TX      0xE1
#define RF_FLUSH_RX      0xE2
#define RF_REUSE_TX_PL   0xE3
#define RF_ACTIVATE      0x50 
#define RF_R_RX_PL_WID   0x60
#define RF_NOP           0xFF

// RF 통신 관련 함수
uint8_t spi_WR(uint8_t data) {
    SPDR = data;
    while((SPSR & 0x80) == 0);
    return SPDR;
}

uint8_t RF_rw_1b(uint8_t addr, uint8_t data, bool RW) {
	static uint8_t byte;
	CS0;
	spi_WR(RW ? (RF_W_REGISTER | addr) : (RF_R_REGISTER | addr));
	data = spi_WR(data);
	CS1;
	return byte;
}

void RF_cmd(uint8_t cmd) {
	CS0;
	spi_WR(cmd);
	CS1;
}

void RF_write_addr(uint8_t addr, uint64_t fb_addr)
{
	CS0; // 쓰기 시작
	spi_WR(RF_W_REGISTER | addr);
	spi_WR((char) (fb_addr & 0xff) );
	spi_WR((char) ( (fb_addr >> 8) & 0xff));
	spi_WR((char) ( (fb_addr >> 16) & 0xff));
	spi_WR((char) ( (fb_addr >> 24) & 0xff));
	spi_WR((char) ( (fb_addr >> 32) & 0xff));
	CS1;
}

void RF_tran_payload(uint8_t* datap, uint8_t length) {
	char i;
	CS0;
	spi_WR(RF_W_TX_PAYLOAD);
	for(i = 0; i < length; i++)
		spi_WR(*(datap + i));
	CS1;
}

void RF_recv_payload(uint8_t* datap, uint8_t length) {
	char i;
	CS0;
	spi_WR(RF_R_RX_PAYLOAD);
	for(i = 0; i < length; i++)
		datap[i] = spi_WR(0x00);
	CS1;
}

// ========================= 초기화 함수 ====================

void port_init(void)
{		// SPI 설정보다 포트 입출력이 먼저 정의되어야 SPI 통신이 정상적으로 됨.
	REMAP = 0x02;
	DDRA = 0b00011110; // CS, SCK, MOSI, CE(nRF24l01) 출력 포트로 사용
	DDRB = 0x05; // output (PB0, PB2)
}

void RF_spi_init(void) {
	SPCR = 0b01010000; // spi 설정
}

void int_init(void)
{
	MCUCR = 0x02; // int0 (PB1) 하강엣지
	GIMSK = 0x50; // PCINT, INT0 활성화
	PCMSK0 = 0x80; // PCINT7 활성화

}

void spi_init(void)
{		// SPI 설정보다 포트 입출력이 먼저 정의되어야 SPI 통신이 정상적으로 됨.
	REMAP = 0x02;
	DDRA = 0b00011110; // CS, SCK, MOSI, CE 출력 포트로 사용
	SPCR = 0b01010000; // spi 설정
}

void RF_init(void)
{
	CE0;
	RF_rw_1b(RF_RF_CH, 0x01, true); // RF 사용채널 지정 

	RF_rw_1b(RF_RX_PW_P0, 0x01, true);
	RF_rw_1b(RF_RX_PW_P1, 0x01, true); // 파이프라인 1번 수신
	RF_rw_1b(RF_RX_PW_P2, 0x00, true);
	RF_rw_1b(RF_RX_PW_P3, 0x00, true);
	RF_rw_1b(RF_RX_PW_P4, 0x00, true);
	RF_rw_1b(RF_RX_PW_P5, 0x00, true);

	RF_rw_1b(RF_RF_SETUP, 0x06, true); // 출력 최대, 속도 1Mbps
	RF_rw_1b(RF_CONFIG, 0b01111001, true); // RX모드, CRC 사용, 모든 인터럽트 비활성화, 파워다운 모드
	RF_rw_1b(RF_EN_AA, 0b00000011, true); // 데이터 파이프 0,1번 auto ack 사용
	RF_rw_1b(RF_EN_RXADDR, 0b00000011, true); // 데이터 파이프라인 0,1번 사용
	RF_rw_1b(RF_SETUP_RETR, 0x06, true); // 250us동안 재전송, 6번 재전송 시도

	RF_cmd(RF_FLUSH_RX); // rx fifo 비우기

	RF_rw_1b(RF_STATUS, 0b01110000, true); // rx_dr, tx_ds, max_rt 초기화

}

volatile unsigned char test[6] = {0,};
volatile unsigned char recv_pay[32] = {0,};
volatile unsigned char cnt = 0;
volatile unsigned char flag = false;
volatile unsigned char toggle = false;
volatile unsigned char is_read = false;
volatile unsigned char recv_chr = 0;
volatile unsigned int recv_PERIOD = 0x0000, jj = 0; // 2byte
volatile unsigned long recv_RAENERGY = 0x00000000; // 3byte
volatile unsigned int buf = 0x0000; // 2byte
volatile uint8_t ii = 0;

int main(void) {
	port_init();

	CS1;
	_delay_ms(500);
	int_init();
	sei();
	_delay_ms(11);
	RF_spi_init();
	_delay_ms(11);
	RF_init();
	CE0;

	RF_write_addr(RF_TX_ADDR, 0xD728D728D7);
	RF_write_addr(RF_RX_ADDR_P0, 0);
	RF_write_addr(RF_RX_ADDR_P1, 0x0303030303);

	RF_rw_1b(RF_CONFIG, 0b00111011, true); // RX 모드	(스탠바이), 수신 인터럽트 활성화
	_delay_us(1600);		
	CE1; // rx 모드 시작
	flag = true;
	while(1) {
	}
	return 0;
}
ISR(INT0_vect) {
	cli();
	if(flag == true) {
		RF_recv_payload((char*) &recv_chr, 1);
		if( (recv_chr >> 2) == DEV_ADDR ) {
			if( (recv_chr & 0x01) == true) {
				OUTPUT_high;
			} else {
				OUTPUT_low;
			}
			CE0; // standby		
			_delay_us(800); // 상대 디바이스가 전송 준비할때까지 대기함
			if(is_read == true) {
				test[0] = DEV_ADDR;
				test[1] = 0x00;
				test[2] = 0x00;		
				test[3] = 0x00;
				test[4] = 0x00;
				test[5] = 0x00;
			} else if(is_read == false) {
				for (jj = 0; jj < 30; jj++)
				{
					recv_pay[jj+1] = ii++;
				}
				recv_pay[0] = DEV_ADDR;
				test[0] = DEV_ADDR;
				test[1] = 0x00;
				test[2] = 0x00;		
				test[3] = 0x00;
				test[4] = 0x00;
				//test[5] = ii++;
				//recv_pay[31] = ii++;
				is_read = false;
			}

			RF_rw_1b(RF_CONFIG, 0b01001000, true); // TX 모드 (off) , 송신, maxrt 인터럽트 생성
			_delay_us(160);
			RF_write_addr(RF_RX_ADDR_P0, 0xD728D728D7); // 마스터 어드레스를 수신 어드레스로 지정
			RF_rw_1b(RF_STATUS, 0b01110000, true); // rx_dr, tx_ds, max_rt 초기화	
			RF_cmd(RF_FLUSH_TX); // TX fifo 비우기
			RF_rw_1b(RF_CONFIG, 0b01001010, true);	 // TX 모드	, 송신, maxrt 인터럽트 생성
			//RF_tran_payload((char*) &test, 6);
			RF_tran_payload((char*) &recv_pay, 32);
			//Transmit_nb(6, test);

			if(cnt == 0xff)
				cnt = 0;

			CE1; //tx 시작
			_delay_us(130);
			CE0;
			flag = false;
		} else {
			CE0; // standby
			RF_rw_1b(RF_CONFIG, 0b00111001, true); // RX 모드 (off)
			RF_write_addr(RF_RX_ADDR_P0, 0);
			RF_rw_1b(RF_STATUS, 0b01110000, true); // rx_dr, tx_ds, max_rt 초기화
			RF_rw_1b(RF_CONFIG, 0b00111011, true); // RX 모드 (on)
			CE1;
			flag = true;
		}
	} else if(flag == false) {
		CE0; // standby
		RF_rw_1b(RF_CONFIG, 0b00111001, true); // RX 모드 (off)
		RF_write_addr(RF_RX_ADDR_P0, 0);
		RF_rw_1b(RF_STATUS, 0b01110000, true); // rx_dr, tx_ds, max_rt 초기화
		RF_rw_1b(RF_CONFIG, 0b00111011, true); // RX 모드 (on)
		CE1;
		flag = true;
	}
	sei();
}
