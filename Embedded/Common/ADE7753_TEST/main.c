#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>

#define F_CPU 8000000UL
#define ADE_CS1 PORTB |=  (1<<2);
#define ADE_CS0 PORTB &= ~(1<<2);
/*
PORTA(REMAP = 0x02) = //CE/SCK/SS/MOSI/MISO
*/

/* ADE7753 register mapping */
/* register adresses */
/* 24 bit */

#define ADE_Waveform  0x01    /* waveform sample register*/
#define ADE_AENERGY  0x02     /* active energy register */
#define ADE_RAENERGY 0x03
#define ADE_LAENERGY 0x04    /* line accumulation active energy register*/
#define ADE_VAENERGY 0x05    /* apparent energy register */
#define ADE_RVAENERGY 0x06
#define ADE_LVAENERGY 0x07
#define ADE_LVARENERGY 0x08
#define ADE_IRMS 0x16
#define ADE_VRMS 0x17
#define ADE_IPEAK 0x22
#define ADE_VPEAK 0x24

/* 16 bit*/
#define ADE_MODE 0x09         /* mode register*/
#define ADE_IRQEN 0x0A        /* interrupt enable register */
#define ADE_ADE_STATUS 0x0B      /* interrupt status register */
#define ADE_RSTSTATUS 0x0C      /* interrupt reset register */
#define ADE_PERIOD 0x27      /* Period of the channel 2*/
#define ADE_LINECYC 0x1C    /* line cycle mode register */

/* 12 bit */ 
#define ADE_ZXTOUT 0x1D    /* Zero-crossing timeout */
#define ADE_CFNUM  0x14
#define ADE_CFDEN  0x15

/* 8 bit */
#define ADE_GAIN 0x0F    /*PGA GAIN adjust for the input channels */
#define ADE_TEMP 0x26    /* Temperature register */
#define ADE_TMODE 0x3D


/* nRF24L01 register mapping */
/* Memory Map */
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

/* 컨트롤 명령 */
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

/* SPI 통신 */

uint8_t spi_WR(unsigned char data) {
    SPDR = data;
    while((SPSR & 0x80) == 0);
    return SPDR;
}

//ADE7753 전용
unsigned char data_WR_8b(unsigned char addr, unsigned char data, unsigned char WR) {
	unsigned char tmp = 0;
	ADE_CS0;
	if (WR) { // W = 1 / R = 0
		spi_WR(addr | 0x80);
		spi_WR(data);
		ADE_CS1;
		return 0;
	} else {
		spi_WR(addr);
		_delay_us(4);
		tmp = spi_WR(0x00);
		ADE_CS1;
		return tmp;
	}
}

unsigned int data_WR_16b(unsigned char addr, unsigned int data, unsigned char WR) {
	unsigned int tmp = 0;
	ADE_CS0;
	if (WR) { // W = 1 / R = 0
		spi_WR(addr | 0x80);
		spi_WR(data >> 8);
		spi_WR(data & 0xff);
		ADE_CS1;
		return 0;
	} else {
		spi_WR(addr);
		_delay_us(4);
		tmp = (unsigned int)spi_WR(0x00) << 8;
		tmp += (unsigned int)spi_WR(0x00);
		ADE_CS1;
		return tmp;
	}
}
unsigned long data_WR_24b(unsigned char addr, unsigned long data, unsigned char WR) {
	unsigned long tmp = 0;
	unsigned char dump;
	ADE_CS0;
	if (WR) { // W = 1 / R = 0
		spi_WR(addr | 0x80);
		spi_WR( (data >> 16) & 0xff );
		spi_WR( (data >> 8) & 0xff );
		spi_WR(data & 0xff);
		ADE_CS1;
		return 0;
	} else {
		spi_WR(addr);
		_delay_us(4);
		dump = spi_WR(0x00);
		tmp = (unsigned long)dump << 16;
		dump = spi_WR(0x00);
		tmp += (unsigned long)dump << 8;
		dump = spi_WR(0x00);
		tmp += (unsigned long)dump;
		ADE_CS1;
		return tmp;
	}
}


/* RF 통신 관련 함수 */


unsigned char Write_1b(unsigned char addr, unsigned char data) {
	CS0; // 쓰기 시작
	spi_WR(RF_W_REGISTER | addr);
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
	spi_WR(RF_W_REGISTER | addr);
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
	spi_WR(RF_R_REGISTER | addr);
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
	spi_WR(RF_W_TX_PAYLOAD);
	spi_WR(data);
	CS1;
}
void Transmit_nb(unsigned char n, unsigned char* datapointer) {
	char i;
	CS0;
	spi_WR(RF_W_TX_PAYLOAD);
	for(i = 0; i<n; i++)
		spi_WR(*(datapointer + i));
	CS1;
}
unsigned char Receive_1b( void ) {
	unsigned char tmp;
	CS0;
	spi_WR(RF_R_RX_PAYLOAD);
	tmp = spi_WR(0x00);
	CS1;
	return tmp;
}
void Receive_nb( unsigned char n, unsigned char* datapointer ) {
	char i;
	CS0;
	spi_WR(RF_R_RX_PAYLOAD);
	for(i = 0; i<n; i++)
		datapointer[i] = spi_WR(0x00);
	CS1;
}
/* ========================= 초기화 함수 ==================== */

void port_init(void)
{		/* SPI 설정보다 포트 입출력이 먼저 정의되어야 SPI 통신이 정상적으로 됨. */
	REMAP = 0x02;
	DDRA = 0b00011110; // CS, SCK, MOSI, CE(nRF24l01) 출력 포트로 사용
	DDRB = 0x05; // output (PB0, PB2)
}

void ADE_spi_init(void) {
	SPCR = 0b01010111; // spi 설정
}

void RF_spi_init(void) {
	SPCR = 0b01010000; // spi 설정
}

void int_init(void)
{
	/*	
	MCUCR = 0x02; // int0 (PB1) 하강엣지
	GIMSK = 0x40; // enable
	*/
	MCUCR = 0x02; // int0 (PB1) 하강엣지
	GIMSK = 0x50; // PCINT, INT0 활성화
	PCMSK0 = 0x80; // PCINT7 활성화

}

void spi_init(void)
{		/* SPI 설정보다 포트 입출력이 먼저 정의되어야 SPI 통신이 정상적으로 됨. */
	REMAP = 0x02;
	DDRA = 0b00011110; // CS, SCK, MOSI, CE 출력 포트로 사용
	SPCR = 0b01010000; // spi 설정
}

void RF_init(void)
{
	CE0;
	Write_1b(RF_RF_CH, 0x01); // RF 사용채널 지정 

	Write_1b(RF_RX_PW_P0, 0x01);
	Write_1b(RF_RX_PW_P1, 0x01);
	Write_1b(RF_RX_PW_P2, 0x00);
	Write_1b(RF_RX_PW_P3, 0x00);
	Write_1b(RF_RX_PW_P4, 0x00);
	Write_1b(RF_RX_PW_P5, 0x00);

	Write_1b(RF_RF_SETUP, 0x06); // 출력 최대, 속도 1Mbps
	Write_1b(RF_CONFIG, 0b01111001); // RX모드, CRC 사용, 모든 인터럽트 비활성화, 파워다운 모드
	Write_1b(RF_EN_AA, 0b00000011); // 데이터 파이프 0,1번 auto ack 사용
	Write_1b(RF_EN_RXADDR, 0b00000011); // 데이터 파이프라인 0,1번 사용
	Write_1b(RF_SETUP_RETR, 0x06); // 250us동안 재전송, 6번 재전송 시도
	Cmd(RF_FLUSH_RX); // rx fifo 비우기

	Write_1b(RF_STATUS, 0b01110000); // 인터럽트 플래그 초기화

}

void ADE_init(void) {
	data_WR_16b(ADE_MODE, 0b0000000010001100, 1);
	data_WR_16b(ADE_IRQEN, 0x0004, 1);
	data_WR_16b(ADE_LINECYC, 0x00F0, 1);
}

volatile unsigned char test[6] = {0,};
volatile unsigned char cnt = 0;
volatile unsigned char flag = false;
volatile unsigned char toggle = false;
volatile unsigned char is_read = false;
volatile unsigned char recv_chr = 0;
volatile unsigned int recv_PERIOD = 0x0000; // 2byte
volatile unsigned long recv_RAENERGY = 0x00000000; // 3byte
volatile unsigned int buf = 0x0000; // 2byte

int main(void) {
	port_init();
	ADE_CS1;
	CS1;

	_delay_ms(500);
	int_init();
	sei();
	ADE_spi_init();
	_delay_ms(11);
	ADE_init();
	RF_spi_init();
	_delay_ms(11);
	RF_init();
	CE0;
	write_addr(RF_TX_ADDR, 0xD7, 0x28, 0xD7, 0x28, 0xD7); // 마스터 디바이스의 고정주소
	write_addr(RF_RX_ADDR_P0, 0x00, 0x00, 0x00, 0x00, 0x00); // 충돌방지
	write_addr(RF_RX_ADDR_P1, DEV_ADDR, DEV_ADDR, DEV_ADDR, DEV_ADDR, DEV_ADDR); // 마스터 -> 슬레이브 전송시 슬레이브 주소
	Write_1b(RF_CONFIG, 0b00111011); // RX 모드	(스탠바이), 수신 인터럽트 활성화
	_delay_us(1600);		
	CE1; // rx 모드 시작
	flag = true;
	while(1) {
	}
	return 0;   /* never reached */
}
ISR(INT0_vect) {
	cli();
	if(flag == true) {
		recv_chr = Receive_1b();
		if( (recv_chr >> 2) == DEV_ADDR ) {
			if( (recv_chr & 0x01) == true) {
				OUTPUT_high;
			} else if( (recv_chr & 0x01) == false) {
				OUTPUT_low;
			}
			CE0; // standby		
			_delay_us(800); // 상대 디바이스가 전송 준비할때까지 대기함
	/*
			ADE_spi_init();
			recv_RAENERGY = data_WR_24b(ADE_RAENERGY, 0x00, 0);	
			RF_spi_init();
	*/
			if(is_read == true) {
				test[0] = DEV_ADDR;
				test[1] = 0x00;
				test[2] = 0x00;		
				test[3] = 0x00;
				test[4] = 0x00;
				test[5] = 0x00;
			} else if(is_read == false) {
				test[0] = DEV_ADDR;
				test[1] = (recv_RAENERGY >> 16) & 0xff;
				test[2] = (recv_RAENERGY >> 8) & 0xff;			
				test[3] = (recv_RAENERGY >> 0) & 0xff;
				test[4] = (recv_PERIOD >> 8) & 0xff;
				test[5] = (recv_PERIOD >> 0) & 0xff;
				is_read = true;
			}


			Write_1b(RF_CONFIG, 0b01001000); // TX 모드 (off) , 송신, maxrt 인터럽트 생성
			_delay_us(160);
			write_addr(RF_RX_ADDR_P0, 0xD7, 0x28, 0xD7, 0x28, 0xD7); // 마스터 어드레스를 수신 어드레스로 지정
			Write_1b(RF_STATUS, 0b01110000); // rx_dr, tx_ds, max_rt 초기화		
			Cmd(RF_FLUSH_TX); // TX fifo 비우기		
			Write_1b(RF_CONFIG, 0b01001010); // TX 모드	, 송신, maxrt 인터럽트 생성


			Transmit_nb(6, test);

			if(cnt == 0xff)
				cnt = 0;

			CE1; //tx 시작
			_delay_us(12);
			CE0;
			flag = false;
		} else {
			CE0; // standby
			Write_1b(RF_CONFIG, 0b00111001); // RX 모드 (off)
			write_addr(RF_RX_ADDR_P0, 0x00, 0x00, 0x00, 0x00, 0x00);
			Write_1b(RF_STATUS, 0b01110000); // rx_dr, tx_ds, max_rt 초기화
			Write_1b(RF_CONFIG, 0b00111011); // RX 모드 (on)
			CE1;
			flag = true;
		}
	} else if(flag == false) {
		CE0; // standby
		Write_1b(RF_CONFIG, 0b00111001); // RX 모드 (off)
		write_addr(RF_RX_ADDR_P0, 0x00, 0x00, 0x00, 0x00, 0x00);
		Write_1b(RF_STATUS, 0b01110000); // rx_dr, tx_ds, max_rt 초기화
		Write_1b(RF_CONFIG, 0b00111011); // RX 모드 (on)
		CE1;
		flag = true;
	}
	sei();
}
ISR(PCINT0_vect) { // 하강엣지
	if( (PINA & 0x80) == false) {
		ADE_spi_init();
		recv_RAENERGY = data_WR_24b(ADE_RAENERGY, 0x00, 0);
		recv_PERIOD = data_WR_16b(ADE_PERIOD, 0x00, 0);
		buf = data_WR_16b(ADE_RSTSTATUS, 0x00, 0);	
		RF_spi_init();
		is_read = false;
	}
}