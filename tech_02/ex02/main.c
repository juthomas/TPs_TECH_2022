
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#define F_CPU 16000000
#endif
#include <avr/io.h>

#define SERIAL_8N1 0x06
#define NULL 0

void uart_init(uint32_t baud, uint8_t config)
{
	//(20.5)
	uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;

	UCSR0A = 1 << U2X0;

	//Setting baudrate
	UBRR0H = baud_setting >> 8;
	UBRR0L = baud_setting;

	//Setting frame format config
	UCSR0C = config;
	// UCSR0C |= (1<<UCSZ00 | (1 << UCSZ01));
	//Enable Transmition and Reception and Interruption on Reception
	UCSR0B = (1 << TXEN0) | (1 << RXEN0); // | (1 << RXCIE0);
}

void uart_tx(char c)
{
	//Wait for empty buffer (20.6.1)
	while (!(UCSR0A & (1 << UDRE0)))
		;
	//Put char into register buffer
	UDR0 = c;
}

char uart_rx(void)
{
	//(20.7)
	/* Wait for data to be received */
	while (!(UCSR0A & (1 << RXC0)))
		;
	/* Get and return received data from buffer */
	return UDR0;
}

int str_comp(char *s1, char *s2)
{
	while (*s1 == *s2 && *s1 && *s2)
	{
		s1++;
		s2++;
	}
	return (*s1 - *s2);
}


void uart_printstr(const char *str)
{
	while (*str)
	{
		uart_tx(*str++);
	}
}

void get_string_uart(int print_char, char str[50], uint16_t max_len)
{
	char c = '\0';
	int i = 0;
	while (c != '\n')
	{
		c = uart_rx();

		if (c == 127)
		{
			if (i > 0)
			{
				uart_printstr("\033[1D\033[K");
				i--;
			}
		}
		else if (c == 13)
		{
			uart_printstr("\r\n");
			break;
		}
		else if ((c >= 32 && c < 126))
		{
			if (print_char == 1)
			{
				uart_tx(c);
			}
			else
			{
				uart_tx('*');
			}
			str[i] = c;
			i++;
			if (i > max_len)
				break;
		}
	}
	str[i] = '\0';
}


void wait_x_cpu_clocks(int32_t cpu_clocks)
{
	while (cpu_clocks > 0)
	{
		cpu_clocks -= 3;
	}
}

void custom_delay(uint32_t milli)
{
	//milli = 0,001s
	milli = milli * 2000;
	wait_x_cpu_clocks(milli - 5);
}

uint8_t analog_read_adc4()
{
	//  REF1  REF0 :
	//  0     0      = AREF, Internal Vref turned off 
	//  0     1      = AVcc with external capacitor at AREF pin 
	//  1     0      = Reserved
	//  1     1      = Internal 2.56 Voltage Reference with external capacitor at AREF Pin 

	//  ADLAR:
	//  0 = Right adjust the result 
	//  1 = Left adjust the result 

	//  MUX4  MUX3  MUX2  MUX1  MUX0 :
	//  0     0     0     0     0      = ADC0
	//  0     0     0     0     1      = ADC1
	//  0     0     0     1     0      = ADC2
	//  0     0     0     1     1      = ADC3
	//  0     0     1     0     0      = ADC4
	//  0     0     1     0     1      = ADC5
	//  0     0     1     1     0      = ADC6
	//  0     0     1     1     1      = ADC7

	//**                     Setup ADC                     **//
	//        ADLAR
	//          |
    //     REFS1|
	//        | |
	ADMUX = 0b01000100;
	//		   | |___|
	//         |     |
	//		   |  MUX4-MUX0
	//	     REFS0
	
	//**                Start the convertion               **//
	ADCSRA |= (1<<ADSC);

	//**         Wait until the convertion is completed    **//
	while (!(ADCSRA & (1<<ADIF)));

	//**                 Stop the convertion               **//
	ADCSRA |= (1<<ADIF);

	//** Get the ADC value and moving from 10bits to 8bits **//
	return (ADC >> 2);
}

void write_exa_number(uint8_t nu)
{
	uint8_t tmp_a;
	uint8_t tmp_b;

	tmp_a = nu & 0xF;
	tmp_b = (nu & 0xF0) >> 4;

	if (tmp_b < 0xA)
		uart_tx(tmp_b + '0');
	else
		uart_tx(tmp_b + 'A' - 10);
	
	if (tmp_a < 0xA)
		uart_tx(tmp_a + '0');
	else
		uart_tx(tmp_a + 'A' - 10);
}

void setup_adc()
{
	ADMUX=(1<<REFS0);      // Selecting internal reference voltage
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);     // Enable ADC also set Prescaler as 128
}


void draw_cursor(uint8_t input, uint8_t len)
{
	uart_tx('[');
	for (uint8_t i = 0; i < len; i++)
	{
		if ((((uint16_t)input * len) / 0xFF) <= i)
			uart_tx(' ');
		else
			uart_tx('#');

	}
	uart_tx(']');

	// uart_tx(tmp_a + 'A' - 10);
}

void remove_cursor(uint8_t len)
{
	uart_printstr("\033[1D\033[K\033[1D\033[K");
	while(len--)
	{
		uart_printstr("\033[1D\033[K");
	}
}

int main()
{
	uart_init(115200, SERIAL_8N1);
	char tmp_command[50];
	DDRD |= 0b01101000;

	TCCR0A |= (1 << COM0A1) | (1 << WGM00) | (1 << WGM01);
	TCCR0B |= (1 << CS10);

	TCCR0A |= (1 << COM0B1) | (1 << WGM00) | (1 << WGM01);
	TCCR0B |= (1 << CS10);

	TCCR2A |= (1 << COM2B1) | (1 << WGM00) | (1 << WGM01);
	TCCR2B |= (1 << CS10);

	OCR0A = 0xFF; //R
	OCR0B = 0xFF; //G
	OCR2B = 0xFF; //B

	setup_adc();
	custom_delay(1000);

	uint8_t current_state = 0;
	uint8_t tmp_last_adc = 0;
	uint8_t tmp_current_adc = 0;
	uart_printstr("\033[1;36mSalut a toi jeune developpeur !\r\n\r\n");
	uart_printstr("\033[1;36mValeur de ADC4 (PD4) : \033[1;35m0x00  ");
	draw_cursor(0, 20);

	for (;;)
	{

		tmp_current_adc = analog_read_adc4();
		if (tmp_last_adc != tmp_current_adc)
		{
			remove_cursor(20);
			uart_printstr("\033[1D\033[K\033[1D\033[K\033[1D\033[K\033[1D\033[K");
			write_exa_number(analog_read_adc4());
			uart_printstr("  ");
			draw_cursor(analog_read_adc4(), 20);
			tmp_last_adc = tmp_current_adc;
		}
		custom_delay(50);
	}
}