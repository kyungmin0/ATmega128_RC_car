#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile unsigned char flag;	// 입력 받을 변수 선언

ISR(USART1_RX_vect)		// UART 인터럽트 
{
	flag = UDR1;		// 변수 안에 data 저장
}

void init();
void DCmotor();
void servo();
void psd_sensor();
void flag_check();

int adc;

int main(void)
{
	init();
	DCmotor();
	servo();
	psd_sensor();
	sei();
	
	while(1)
	{
		ADCSRA |= (1 << ADSC);	// ADSC -> 6번 핀, 1일 때 ADC 변환 시작 (0b 1100 0111)

		while(!(ADCSRA & (1<<ADIF)));	//ADIF  -> 4번 핀, ADC 변환 후 무한루프 탈출
		adc = ADC;						// 변환된 디지털 값을 ADC Register에 입력
		
		flag_check();		// RC카 상태
	}
}

void init()
{
	DDRA = 0xFF;	// DDRA = 0b 1111 1111
	DDRB = 0xFF;	// DDRB = 0b 1111 1111
	DDRD = 0x08;	// DDRD = 0b 0000 1000
	DDRE = 0x1F;	// DDRE = 0b 0001 1111
	DDRF = 0x00;	// DDRF = 0b 0000 0000
	
	UCSR1A = 0x00;	// UCSR1A = 0b 0000 0000
	UCSR1B = 0x98;	// UCSR1B = 0b 1001 1000
	UCSR1C = 0x06;	// UCSR1C = 0b 0000 0110
	UBRR1H = 0;
	UBRR1L = 103;
}

void DCmotor()
{
	TCCR1A = (1<<COM1B1)|(0<<COM1B0)|(1<<WGM11);
	TCCR1B = (1<<WGM13)|(1<<WGM12)|(0<<CS02)|(0<<CS01)|(1<<CS00); //prescaler 1
	
	ICR1 = 799;		// Timer/Counter 1, top
}

void servo()
{
		TCCR3A |= (1<<COM3B1) | (0<<COM3B0) | (1<<WGM31) | (0<<WGM30);				// TCCR3A = 0b 0010 0010 , 0x22	/ B channel
		TCCR3B |= (1<<WGM33) | (1<<WGM32) | (0<<CS32) | (1<<CS31) | (1<<CS30);		// TCCR3B = 0b 0001 1011 , 0x1b / 64분주
		ICR3 = 4999; // Fast PWM TOP = ICR3 / 주기 : 20ms
}

void psd_sensor()
{
	ADMUX = 0x40;	// 0b 0100 0000
	ADCSRA = 0x87;	// 0b 1000 0111
}

void flag_check()
{
	if(flag =='G') // 전진
	{
		if(adc > 300)	// 장애물과 가까워질 때
		{
			PORTA = 0x3C;	// PORTA = 0b 0011 1100
			OCR1B = 0;
		}
		else
		{
			PORTA = 0xFF;	// PORTA = 0b 1111 1111
			OCR1B = 719;	// Timer/Counter 1, Duty Ratio : 90%
			PORTE = 0x0A;	// PORTE = 0b 0000 1010
		}
	}
	else if(flag =='B')	 // 후진
	{
		PORTA = 0xFF;	// PORTA = 0b 1111 1111
		OCR1B = 719;	// Timer/Counter 1, Duty Ratio : 90%
		PORTE = 0x15;	// PORTE = 0b 0001 0101
	}
	else if(flag =='R') // 우로 앞바퀴 조향
	{
		PORTA = 0x3F;		// PORTA = 0b 0011 1111
		PORTE = 0x10;		// PORTE = 0b 0001 0000
		if (OCR3B >= 350)
		{
			OCR3B = 250;	// Timer/Counter 3, 서보모터 역회전
		}
	}
	else if(flag =='L')	// 좌로 앞바퀴 조향
	{
		PORTA = 0xFC;		// PORTA = 0b 1111 1100
		PORTE = 0x10;		// PORTE = 0b 0001 0000
		if (OCR3B <= 400)
		{
			OCR3B = 500;	// Timer/Counter 3, 서보모터 정회전
		}
	}
	else if (flag == 'M')	// 가운데로 앞바퀴 조향
	{
		PORTA = 0xC3;		// PORTA = 0b 1100 0011 
		PORTE = 0x10;		// PORTE = 0b 0001 0000
		if(OCR3B != 380)
		{
			OCR3B = 380;	// Timer/Countere 3, 서보모터 고정
		}
	}
	else if(flag == 0) //정지
	{
		PORTA = 0x00;	// PORTA = 0b 0000 0000
		OCR1B = 0;
	}
}




