#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#define DEV_ADDR 0x03


#define F_CPU 8000000UL
#define CS1 PORTA |=  (1<<2);
#define CS0 PORTA &= ~(1<<2);
#define CE1 PORTA |=  (1<<6);
#define CE0 PORTA &= ~(1<<6);
#define ADE_CS1 PORTB |=  (1<<2);
#define ADE_CS0 PORTB &= ~(1<<2);
#define OUTPUT_high PORTB |=  (1<<0);
#define OUTPUT_low PORTB &= ~(1<<0);
#define true 1
#define false 0


#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_COMMAND_RFCONFIGURATION       (0x32)
#define PN532_COMMAND_SETPARAMETERS         (0x12)
#define PN532_COMMAND_WRITEREGISTER         (0x08)
#define PN532_COMMAND_INSELECT              (0x54)
#define PN532_COMMAND_INDATAEXCHANGE        (0x40)

#define STATUS_NONE							(0x00)
#define STATUS_ACK							(0x01)
#define STATUS_NACK							(0x02)
#define STATUS_PACKET_RECV					(0x03)
#define STATUS_PACKET_RECV_FAIL				(0x04)

const uint8_t ACK_F[6] = {0x00, 0x00, 0xff, 0x00, 0xff, 0x00};
const uint8_t SAMConfig[2] = { PN532_COMMAND_SAMCONFIGURATION, 0x01}; // normal mode
const uint8_t readPassiveTargetID[3] = { PN532_COMMAND_INLISTPASSIVETARGET, 0x02, 0x00}; // normal mode
const uint8_t RFConfiguration[3] = { PN532_COMMAND_RFCONFIGURATION, 0x01, 0x02};
const uint8_t SetParameters[2] = { PN532_COMMAND_SETPARAMETERS, 0x04};
const uint8_t InSelect[2] = { PN532_COMMAND_INSELECT, 0x01};
volatile uint8_t InDataExchange[4] = {PN532_COMMAND_INDATAEXCHANGE , 0x01, 0x30, 0x00};
const uint8_t WriteRegister[4] = {PN532_COMMAND_WRITEREGISTER , 0x01, 0x24, 0x00};

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

#define DEV_SET_STATUS		0x00
#define DEV_SET_DATA		0x01
#define DEV_GET_NFC			0x02
#define DEV_GET_POW			0x03

//시리얼 통신
void tx_char(uint8_t tx_data) {
	while((UCSR1A&0x20) == 0x00);
	UDR1 = tx_data; //시리얼 포트를 통하여 데이터 전송
}

void tx_packet(uint8_t *s, int len) {
	int i;
 	for (i = 0; i < len; i++)
    	tx_char(s[i]);
}

void wakeup(uint8_t* frame) {
    frame[0] = 0x55;
    frame[1] = 0x55;
    frame[2] = 0x00;
    frame[3] = 0x00;
    frame[4] = 0x00;
    tx_packet(frame, 5);
    return frame;
}

void packet(uint8_t* frame, uint8_t* data, int len) {
	int i;
    char sum = 0xD4;
    frame[0] = 0x00;
    frame[1] = 0x00;
    frame[2] = 0xFF;
    frame[3] = len + 1;
    frame[4] = ~(len+1) + 1;
    frame[5] = 0xD4;
    for(i=0; i<len; i++){
        frame[6+i] = data[i];
        sum += data[i];
    }
    frame[len+6] = ~sum+1;
    frame[len+7] = 0x00;
    tx_packet(frame, len + 8);
}

void xprintf(uint8_t *str_data) {
	while(*str_data != 0x00){ //문자열의 끝부분이 아니라면
		tx_char(*str_data); //시리얼포트로 한개의 문자를 송신한다.
		str_data++;
	}
}


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
	DDRA = 0b01101110; // CS, SCK, MOSI, CE 출력 포트로 사용, serial output 출력
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
	RF_rw_1b(RF_RX_PW_P1, 0x02, true); // 파이프라인 1번 수신
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

volatile unsigned char recv_pay[32] = {0,};
volatile unsigned char cnt = 0;
volatile unsigned char flag = false;
volatile unsigned char toggle = false;
//volatile unsigned char is_read = false;
volatile unsigned char recv_chr[2];
volatile unsigned int recv_PERIOD = 0x0000, jj = 0; // 2byte
volatile unsigned long recv_RAENERGY = 0x00000000; // 3byte
volatile unsigned int buf = 0x0000; // 2byte

volatile uint8_t tx_buf[30];
volatile uint8_t rx_buf[30];
volatile uint8_t dump[3][16] = {0x00, };
volatile uint8_t status = 0, checksum = 0, rx_len = 0, len;

volatile uint8_t test = 0x00;


int main(void) {
	int i, j;
	port_init();

	//(UART 1)
	UCSR1A = 0b00100010;
	UCSR1B = 0b10011000;
	UCSR1C = 0b00000110;
	UBRR1L = 8;

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

	status = STATUS_NONE;
	wakeup(tx_buf);
	packet(tx_buf, SAMConfig, 2);

	while(status != STATUS_PACKET_RECV);
	wakeup(tx_buf);
	packet(tx_buf, RFConfiguration, 3);
	status = STATUS_NONE;
	while(status != STATUS_PACKET_RECV);
	wakeup(tx_buf);
	packet(tx_buf, SetParameters, 2);
	status = STATUS_NONE;
	while(status != STATUS_PACKET_RECV);
	wakeup(tx_buf);
	packet(tx_buf, readPassiveTargetID, 3);
	status = STATUS_NONE;
	while(status != STATUS_PACKET_RECV);
	wakeup(tx_buf);
	packet(tx_buf, InSelect, 2);
	status = STATUS_NONE;
	for (i = 0; i < 3; i++) {
		InDataExchange[3] = i;
		while(status != STATUS_PACKET_RECV);
		wakeup(tx_buf);
		packet(tx_buf, InDataExchange, 4);
		status = STATUS_NONE;
		while(status != STATUS_PACKET_RECV);
		memcpy(dump[i], rx_buf + 8,16);
		wakeup(tx_buf);
		packet(tx_buf, WriteRegister, 4);
		status = STATUS_NONE;
	}

	while(1) {
	}
	return 0;
}
ISR(INT0_vect) {
	sei();
	if(flag == true) {
		RF_recv_payload((char*) &recv_chr, 2);
		if( (recv_chr[0] >> 2) == DEV_ADDR ) {
			if( (recv_chr[0] & 0x01) == true) {
				OUTPUT_high;
			} else {
				OUTPUT_low;
			}
			CE0; // standby		
			_delay_us(800); // 상대 디바이스가 전송 준비할때까지 대기함

			recv_pay[0] = recv_chr[0];
			recv_pay[1] = recv_chr[1];
			for (jj = 0; jj < 16; jj++)
			{
				recv_pay[jj+2] = dump[0][jj];
			}
			/*
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

			*/
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
}
ISR(USART1_RX_vect) {
	rx_buf[rx_len++] = UDR1;
	if(rx_len > 29)
		rx_len = 0;
	if(rx_len == 6) {
		if(memcmp(rx_buf, ACK_F, 6) == 0) {
			OUTPUT_high;
			status = STATUS_ACK;
			rx_len = 0;
		}
	}
	if(status == STATUS_ACK) {
		if(rx_len == 4)
			len = rx_buf[rx_len - 1] + 1;
		if(rx_len >= 6) {
			if(len) {
				checksum += rx_buf[rx_len - 1];
				len--;
			} else {
				if ( !( 0xff & checksum ) ) {
					status = STATUS_PACKET_RECV;
					rx_len = 0;
				}
				else {
					status = STATUS_PACKET_RECV_FAIL;
					rx_len = 0;
				}
			}
		}
	}
}