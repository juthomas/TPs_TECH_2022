
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#define F_CPU 16000000
#endif
#include <avr/io.h>
#include <avr/eeprom.h>
#define SERIAL_8N1 0x06
#define NULL 0


void eeprom_read(const uint16_t offset, uint8_t* const data, const uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		data[i] = eeprom_read_byte(offset + i);
		custom_delay(100);
	}
}

void eeprom_write(const uint16_t offset, const uint8_t* data, const uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		if (data[i] != eeprom_read_byte(offset + i))
		{
			eeprom_write_byte(offset + i, data[i]);
			// EEPROM_write(offset + i, data[i]);
		}
		custom_delay(100);
	}
}

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


void set_data_at_addr(uint8_t i, uint16_t addr)
{
	uint16_t *data;
	eeprom_read(addr/8, (uint8_t*)data, 2);
	switch (addr % 8)
	{
		case 0:
			data[0] &= ~0b1110000000000000;
			data[0] |= i & 0b1110000000000000;
			break;
		case 1:
			data[0] &= ~0b0111000000000000;
			data[0] |= i & 0b0111000000000000;
		case 2:
			data[0] &= ~0b0011100000000000;
			data[0] |= i & 0b0011100000000000;
		case 3:
			data[0] &= ~0b0001110000000000;
			data[0] |= i & 0b0001110000000000;
		case 4:
			data[0] &= ~0b0000111000000000;
			data[0] |= i & 0b0000111000000000;
		case 5:
			data[0] &= ~0b0000011100000000;
			data[0] |= i & 0b0000011100000000;
		case 6:
			data[0] &= ~0b0000001110000000;
			data[0] |= i & 0b0000001110000000;
		case 7:
			data[0] &= ~0b0000000111000000;
			data[0] |= i & 0b0000000111000000;
	}
	eeprom_write(addr/8, (uint8_t*)data, 2);
}


#define HEADER_SIZE 20
void get_ascii_uart(void)
{
	char c = '\0';
	int i = 0;
	char last_is_ret = 0;
	char palet[7] = {0};
	uint16_t current_index = 0;
	uint8_t buffer[3] = {};

	while (c != '\n')
	{
		c = uart_rx();

		if (c == 13)
		{
			if (last_is_ret)
			{
				uart_printstr("\r\n");
				break;
			}
			set_data_at_addr(7 + HEADER_SIZE * 8, current_index);
			current_index++;
			last_is_ret = 1;
		}
		else if ((c >= 32 && c < 126))
		{
			int i;
			for (i = 0; i < 7; i++)
			{
				if (palet[i] == c)
				{
					break;
				}
				if (palet[i] == 0)
				{
					palet[i] = c;
					break;
				}
			}
			if (i == 7)
			{
				//error
			}
			set_data_at_addr(i + HEADER_SIZE * 8, current_index);
			current_index++;
			last_is_ret = 0;
		}
	}
	eeprom_write(0, (uint8_t*)palet, 7);
	eeprom_write(0, (uint8_t*)current_index, 2);


}

void print_ascii_uart()
{

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

	uint8_t current_state = 0;
	for (;;)
	{
		// uart_printstr("\033[1;36mVeuillez entrer votre ascii-art\r\n");
		
		uart_printstr("\033[1;36mBonjour ! Commandes possibles : \r\n"
					  "\033[1;35mread\033[1;36m / \033[1;35mwrite\033[1;36m / \033[1;35medit\033[1;36m\r\n\r\n");
		uart_printstr("\033[1;34m$> \033[1;35m");
		get_string_uart(1, tmp_command, 50);
		custom_delay(500);

		uart_printstr("\033[1;34mcommand: \033[1;35m");
		uart_printstr(tmp_command);
		// get_string_uart(0, tmp_password);
		uart_printstr("\033[1;36m\r\n");
		if (str_comp(tmp_command, "read") == 0)
		{
			uart_printstr("\033[1;34mRead \033[1;35m\r\n");
		}
		else if (str_comp(tmp_command, "write") == 0)
		{
			uart_printstr("\033[1;34mEntrez votre Ascii-Art\033[1;35m\r\n");
			get_ascii_uart();
		}
		else if (str_comp(tmp_command, "edit") == 0)
		{
			uart_printstr("\033[1;34mEdit \033[1;35m\r\n");
		}
		else 
		{
			uart_printstr("\033[1;34mCommande inconnue \033[1;35m\r\n\r\n");
		}




	}
}