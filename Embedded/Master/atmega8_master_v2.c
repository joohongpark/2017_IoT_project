#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include <stdbool.h>

#define true 1
#define false 0
#define F_CPU 16000000UL

#define CS1 PORTB |=  (1<<2);
#define CS0 PORTB &= ~(1<<2);
#define CE1 PORTB |=  (1<<1);
#define CE0 PORTB &= ~(1<<1);

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

/* RF 관련 변수 */

volatile uint8_t flag = 1, status_now, rx_exist, rx_prev_exist;


/* 시리얼 통신 관련 함수 */

void tx_char(char tx_data) {
	while((UCSRA&0x20) == 0x00);
	UDR = tx_data; //시리얼 포트를 통하여 데이터 전송
}

void xprintf(char *str_data) {
	while(*str_data != 0x00){ //문자열의 끝부분이 아니라면
		tx_char(*str_data); //시리얼포트로 한개의 문자를 송신한다.
		str_data++;
		}
}
void xprintf_hex(uint8_t n, uint8_t *str_data) {
	int i;
	for(i = 0; i<n; i++){
		tx_hex(str_data[i]);
		tx_char(' ');
	}
}

void tx_hex(uint8_t data) {
	tx_char( (data / 16) < 0x0a ? ( (data / 16) + 0x30 ) : ( (data / 16) + 0x37) );
	tx_char( (data % 16) < 0x0a ? ( (data % 16) + 0x30 ) : ( (data % 16) + 0x37) );
}

void tx_dec(uint8_t data) {
	tx_char( (data / 10) == 0 ? ' ' : ( (data / 10) + 0x30 ) );
	tx_char( ( (data % 10) + 0x30 ) );
}

void print_4b_hex(unsigned long j) {
	xprintf("0x");	
	tx_hex( (j >> 24) & 0xff );
	tx_hex( (j >> 16) & 0xff );
	tx_hex( (j >> 8) & 0xff );
	tx_hex( (j >> 0) & 0xff );
}

void print_2b_hex(unsigned int j) {
	xprintf("0x");
	tx_hex( (j >> 8) & 0xff );
	tx_hex( (j >> 0) & 0xff );
}

void print_long_dec(unsigned long l) {
	char i, j, digit;
	unsigned long ten = 1;
	for(i = 9; i > -1 ;i--) {
		for(j = 0; j < i; j++)
			ten *= (unsigned long) 10;
		digit = (l / ten) % 10;
		tx_char(digit + 0x30);
		ten = 1;
	}
}

void print_long_dec_without_zero(unsigned long l) {
	char i, j, digit, appear_zero = 0, signed_bytes = 3;
	char buffer[10] = {0,};
	unsigned long ten = 1, tmp;
	if( ((l >> ((signed_bytes * 8) - 1 ) ) & 0x01) == 0x01) {
		tmp = (~l + 1) & (0xffffffff >> ((4 - signed_bytes) * 8));
		tx_char('-');
	} else {
		tmp = l;
	}
	for(i = 0; i < 10 ;i++) {
		for(j = 0; j < i; j++)
			ten *= (unsigned long) 10;
		buffer[i] = (tmp / ten) % 10;
		ten = 1;
	}
	for(i = 0; i < 10 ; i++) {
		if( buffer[9 - i] != 0 ) {
			tx_char(buffer[9 - i] + 0x30);
			appear_zero = 1;
		} else if ( (appear_zero == 1) | (i == 9) ) {
			tx_char(buffer[9 - i] + 0x30);
		}
	}
}
/* RF 통신 관련 함수 */

uint8_t spi_WR(uint8_t data) {
    SPDR = data;
    while((SPSR & 0x80) == 0);
    return SPDR;
}

uint8_t RF_rw_1b(uint8_t addr, uint8_t data, uint8_t RW) {
	static uint8_t byte;
	CS0;
	spi_WR(RW ? (W_REGISTER | addr) : (R_REGISTER | addr));
	byte = spi_WR(data);
	CS1;
	return byte;
}

void RF_write_addr(uint8_t addr, uint64_t fb_addr)
{
	CS0; // 쓰기 시작
	spi_WR(W_REGISTER | addr);
	spi_WR((char) (fb_addr & 0xff) );
	spi_WR((char) ( (fb_addr >> 8) & 0xff));
	spi_WR((char) ( (fb_addr >> 16) & 0xff));
	spi_WR((char) ( (fb_addr >> 24) & 0xff));
	spi_WR((char) ( (fb_addr >> 32) & 0xff));
	CS1;
}

void RF_cmd(uint8_t cmd) {
	CS0;
	spi_WR(cmd);
	CS1;
}

void RF_tran_payload(uint8_t* datap, uint8_t length) {
	char i;
	CS0;
	spi_WR(W_TX_PAYLOAD);
	for(i = 0; i < length; i++)
		spi_WR(*(datap + i));
	CS1;
}

void RF_recv_payload(uint8_t* datap, uint8_t length) {
	char i;
	CS0;
	spi_WR(R_RX_PAYLOAD);
	for(i = 0; i < length; i++)
		datap[i] = spi_WR(0x00);
	CS1;
}

/* ========================= 초기화 함수 ==================== */

void spi_init(void)
{		/* SPI 설정보다 포트 입출력이 먼저 정의되어야 SPI 통신이 정상적으로 됨. */
	DDRB = 0b00101110; // CS, SCK, MOSI, CE 출력 포트로 사용
	SPCR = 0b01010000; // spi 설정
}

void RF_init(void) {
	CE0;
	RF_rw_1b(RF_CH, 0x01, true); // RF 사용채널 지정 

	RF_rw_1b(RX_PW_P0, 0x01, true);
	RF_rw_1b(RX_PW_P1, 0x20, true); // 파이프라인 1번 수신, 32바이트
	RF_rw_1b(RX_PW_P2, 0x00, true);
	RF_rw_1b(RX_PW_P3, 0x00, true);
	RF_rw_1b(RX_PW_P4, 0x00, true);
	RF_rw_1b(RX_PW_P5, 0x00, true);

	RF_rw_1b(RF_SETUP, 0x06, true); // 출력 최대, 속도 1Mbps
	RF_rw_1b(CONFIG, 0b01001000, true); // CRC 사용, 수신인터럽트만 미사용(MAX_RT, TX 인터럽트 enable) PWR_UP = 0
	RF_rw_1b(EN_AA, 0b00000011, true); // 데이터 파이프 0,1번 auto ack 사용
	RF_rw_1b(EN_RXADDR, 0b00000011, true); // 데이터 파이프라인 0,1번 사용
	RF_rw_1b(SETUP_RETR, 0x0A, true); // 250us동안 재전송, 10번 재전송 시도

	RF_cmd(FLUSH_TX); // tx fifo 비우기
	RF_cmd(FLUSH_RX); // rx fifo 비우기

	RF_rw_1b(STATUS, 0b01110000, true); // rx_dr, tx_ds, max_rt 초기화

}

void Serial_init() {
	UCSRB = 0b10011000;
	UCSRC = 0b10000110;
	UBRRL = 8;
}

volatile uint8_t rx[32] = {0, };
volatile unsigned int recv_PERIOD = 0x0000; // 2byte
volatile unsigned long recv_RAENERGY = 0x00000000; // 3byte
volatile uint8_t i = 1, k;
volatile uint8_t eeprom_pointer = 0x00;
volatile uint8_t transmit_buf = 0x00;
volatile uint8_t uart_rx_buf = 0x00;
volatile uint8_t is_exist = false;
volatile uint8_t tx_dump[4] = {0x00, };

volatile uint8_t rx_mode = false;
volatile uint8_t buf_pointer = 0x00;
volatile uint8_t ctrl_addr = 0x00;
volatile uint8_t ctrl_value = 0x00;

int main(void) {
	uint64_t addr = 0;
	spi_init();
	Serial_init();
	_delay_ms(2000); // Power on reset (10.3ms)
	RF_init();
	CE0;

	//int0 하강엣지
	MCUCR = 0b00000010;
	GICR = 0b01000000;
	sei();

	RF_write_addr(RX_ADDR_P1, 0x5050050555);


	i = 0x01; // start addr
	while(1) {
		addr = ((uint64_t)i << 32) | ((uint64_t) (~i & 0xff) << 24) | ((uint64_t)i << 16) | ((uint64_t) (~i & 0xff) << 8) | i;
		RF_write_addr(TX_ADDR, addr);
		RF_write_addr(RX_ADDR_P0, addr);	

		RF_cmd(FLUSH_TX); // tx fifo 비우기				
		RF_rw_1b(CONFIG, 0b01001010, true); // TX 모드 & power on	
		_delay_us(1600); // start up delay
		rx_exist = false; // 인터럽트 루틴 플래그 초기화
		transmit_buf = eeprom_read_byte(i);
		tx_dump[0] = i << 4 | (~i & 0x0f);
		tx_dump[1] = transmit_buf;
		tx_dump[2] = 0x0f;
		tx_dump[3] = 0xf0;
		RF_tran_payload((char*) &tx_dump, 4); // 데이터 전송
		CE1;
		_delay_us(12);
		CE0;		
		flag = true;
		while(flag);
		RF_rw_1b(CONFIG, 0b01111000, true); // TX 모드 power off
		if(rx_exist){
			recv_RAENERGY = ((unsigned long)rx[1] << 16) | ((unsigned long)rx[2] << 8) | ((unsigned long)rx[3] << 0);
			recv_PERIOD = ((unsigned long)rx[30] << 8) | ((unsigned long)rx[31] << 0);
			if(is_exist) {
				xprintf(",");
			}
			xprintf("\"");
			tx_dec(i);
			xprintf("\":{");
			xprintf("\"onoff\":");
			if(transmit_buf == true)
				xprintf("1");
			else
				xprintf("0");
			xprintf(",\"raw_data\":");
			for (k = 0; k < 32; k++)
			{
				tx_hex(rx[k]);
			}
			xprintf("}");
			is_exist = true;
		}		
		i++;
		if(i == 0x20) {
			i = 0x01;
			xprintf("}\r\n{");
			is_exist = false;
			RF_init();
			}
	}
	return 0;   /* never reached */
}

ISR(INT0_vect) { //데이터 송신이 완료된다면? 혹은 데이터 전송을 실패 한다면???????s
	status_now = RF_rw_1b(STATUS, 0x00, false);
	if ((status_now & 0x20) == 0x20) { // 데이터 전송을 성공한다면	
		rx_exist = true;
		RF_rw_1b(STATUS, 0b01110000, true); // 인터럽트 플래그 초기화
		RF_rw_1b(CONFIG, 0b01111011, true); // RX 모드	(모든 인터럽트 비활성화)
		RF_cmd(FLUSH_RX); // rx fifo 비우기			
		CE1; // rx 모드 시작
		_delay_us(130); // rx 모드 보증
		TCCR0 = 0b00000101; // timer active		
		while((TCNT0 <= 0xfe) & ( (RF_rw_1b(STATUS, 0x00, false) & 0x40) != 0x40 ) ); // 16.384ms동안 대기하거나 (timeout) 데이터가 수신되면
		if(( (RF_rw_1b(STATUS, 0x00, false) & 0x40) != 0x40 ))
			rx_exist = false;
		CE0; // standby
		TCCR0 = 0x00; // timer disable		
		TCNT0 = 0x00; // counter init
		RF_rw_1b(STATUS, 0b01110000, true); // rx_dr, tx_ds, max_rt 초기화
		RF_recv_payload(rx, 32); // 데이터 받기
		RF_cmd(FLUSH_RX); // rx fifo 비우기
	} else if ((status_now & 0x10) == 0x10) { // 데이터 전송을 실패한다면
		RF_rw_1b(STATUS, 0b01110000, true); // rx_dr, tx_ds, max_rt 초기화
		RF_cmd(FLUSH_RX); // rx fifo 비우기
		rx_exist = false;
	} else {
		RF_rw_1b(STATUS, 0b01110000, true); // rx_dr, tx_ds, max_rt 초기화
	}
	flag = false; // rx 루틴 플래그
}

ISR(USART_RXC_vect) { // 시리얼 수신시
	uart_rx_buf = UDR;
	if(uart_rx_buf == 0x63)
		rx_mode = true;
	if(rx_mode == true) {
		buf_pointer++;
		if( (2 == buf_pointer) | (3 == buf_pointer)) {
			if( (0x30 <= uart_rx_buf) | (0x39 >= uart_rx_buf) ) {
				if(buf_pointer == 2) {
					ctrl_addr = (uart_rx_buf - 0x30) * 10;
				} else if(buf_pointer == 3) {
					ctrl_addr += (uart_rx_buf - 0x30);
				}
			} else {
				rx_mode = false;
				buf_pointer = 0;
			}
		}
		if(buf_pointer == 4) {
			if( (0x30 <= uart_rx_buf) | (0x39 >= uart_rx_buf) ) {
				ctrl_value = uart_rx_buf - 0x30;
			} else {
				rx_mode = false;
				buf_pointer = 0;
			}
		}
		if(buf_pointer == 5) {
			if( uart_rx_buf == 0x0D ) {
				eeprom_write_byte(ctrl_addr,ctrl_value);
				rx_mode = false;
				buf_pointer = 0;
			} else {
				rx_mode = false;
				buf_pointer = 0;
			}
		}
		if(buf_pointer > 5) {
			rx_mode = false;
			buf_pointer = 0;
		}
	}
}