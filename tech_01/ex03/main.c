
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

uint8_t parse(char *command, uint8_t *output)
{
	*output = 0;
	if (command[0] >= '0' && command[0] <= '9')
	{
		*output += (command[0] - '0') << 4;
	}
	else if (command[0] >= 'a' && command[0] <= 'f')
	{
		*output += (command[0] - 'a' + 10) << 4;
	}
	else if (command[0] >= 'F' && command[0] <= 'F')
	{
		*output += (command[0] - 'F' + 10) << 4;
	}
	else
	{
		return (1);
	}
	if (command[1] >= '0' && command[1] <= '9')
	{
		*output += (command[1] - '0') << 4;
	}
	else if (command[1] >= 'a' && command[1] <= 'f')
	{
		*output += (command[1] - 'a' + 10) << 4;
	}
	else if (command[1] >= 'F' && command[1] <= 'F')
	{
		*output += (command[1] - 'F' + 10) << 4;
	}
	else
	{
		return (1);
	}
	return (0);
}

uint8_t parse_color(char *command)
{
	if (command[0] != '#')
		return (1);
	uint8_t r;
	uint8_t g;
	uint8_t b;
	if (parse(command + 1, &r))
	{
		return (1);
	}
	if (parse(command + 3, &g))
	{
		return (1);
	}
	if (parse(command + 5, &b))
	{
		return (1);
	}
	OCR0A = 0xFF - r; //R
	OCR0B = 0xFF - g; //G
	OCR2B = 0xFF - b; //B
	return (0);
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


	uint8_t current_state = 0;
	for (;;)
	{
		uart_printstr("\033[1;36mVeuillez entrer une couleur en hexadecimal "
					  "(\033[1;35m#XXXXXX\033[1;36m) : \r\n");
		uart_printstr("\033[1;34m$> \033[1;35m");
		get_string_uart(1, tmp_command, 50);
		custom_delay(500);

		uart_printstr("\033[1;34mcommand: \033[1;35m");
		uart_printstr(tmp_command);
		// get_string_uart(0, tmp_password);
		uart_printstr("\033[1;36m\r\n");

		if (parse_color(tmp_command))
		{
			uart_printstr("\033[1;34mError \033[1;35m\r\n");
		}
	}
}