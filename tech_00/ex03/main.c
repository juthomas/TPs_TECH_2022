
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#define F_CPU 16000000
#endif
#include <avr/io.h>
#include <avr/eeprom.h>
#define SERIAL_8N1 0x06

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

unsigned char EEPROM_read(unsigned int uiAddress)
{
	/* Wait for completion of previous write */
	while (EECR & (1 << EEPE))
		;
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1 << EERE);
	/* Return data from Data Register */
	return EEDR;
}

void eeprom_read(const uint16_t offset, uint8_t *const data, const uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		data[i] = EEPROM_read(offset + i);
	}
}

void eeprom_write(const uint16_t offset, const uint8_t *data, const uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		if (data[i] != EEPROM_read(offset + i))
		{
			// EEPROM_write(offset + i, data[i]);
			eeprom_write_byte((uint8_t *)(offset + i), data[i]);
		}
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

void print_memory(uint8_t *data, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		for (int8_t u = 7; u >= 0; u--)
		{
			uart_tx(((data[i] >> u) & 1) + '0');
		}
		uart_tx(' ');
	}
	uart_printstr("\r\n\r\n");
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

uint8_t get_data_at_addr(uint16_t addr)
{
	uint16_t *data = 0;
	custom_delay(100);
	eeprom_read(addr / 8, (uint8_t *)data, 2);
	custom_delay(100);
	switch (addr % 8)
	{
	case 0:
		return ((data[0] & 0b1110000000000000) >> 13);
		break;
	case 1:
		return ((data[0] & 0b0111000000000000) >> 12);
		break;
	case 2:
		return ((data[0] & 0b0011100000000000) >> 11);
		break;
	case 3:
		return ((data[0] & 0b0001110000000000) >> 10);
		break;
	case 4:
		return ((data[0] & 0b0000111000000000) >> 9);
		break;
	case 5:
		return ((data[0] & 0b0000011100000000) >> 8);
		break;
	case 6:
		return ((data[0] & 0b0000001110000000) >> 7);
		break;
	case 7:
		return ((data[0] & 0b0000000111000000) >> 6);
		break;
	}
	return (0);
}

void set_data_at_addr(uint8_t i, uint16_t addr)
{
	uint16_t *data = 0;

	uart_printstr("MEMORY I :");
	print_memory(&i, 1);
	uart_printstr("ADDR :");
	addr = addr % 8;
	print_memory((uint8_t *)&addr, 1);

	custom_delay(100);
	eeprom_read(addr / 8, (uint8_t *)data, 2);
	custom_delay(100);
	switch (addr % 8)
	{
	case 0:
		data[0] &= ~0b1110000000000000;
		data[0] |= ((uint16_t)i << 13) & 0b1110000000000000;
		break;
	case 1:
		data[0] &= ~0b0111000000000000;
		data[0] |= ((uint16_t)i << 12) & 0b0111000000000000;
		break;
	case 2:
		data[0] &= ~0b0011100000000000;
		data[0] |= ((uint16_t)i << 11) & 0b0011100000000000;
		break;
	case 3:
		data[0] &= ~0b0001110000000000;
		data[0] |= ((uint16_t)i << 10) & 0b0001110000000000;
		break;
	case 4:
		data[0] &= ~0b0000111000000000;
		data[0] |= ((uint16_t)i << 9) & 0b0000111000000000;
		break;
	case 5:
		data[0] &= ~0b0000011100000000;
		data[0] |= ((uint16_t)i << 8) & 0b0000011100000000;
		break;
	case 6:
		data[0] &= ~0b0000001110000000;
		data[0] |= ((uint16_t)i << 7) & 0b0000001110000000;
		break;
	case 7:
		data[0] &= ~0b0000000111000000;
		data[0] |= ((uint16_t)i << 6) & 0b0000000111000000;
		break;
	}
	if (data[0])
	{
		uart_printstr("DATA VOID\r\n\r\n");
	}
	else
	{
		uart_printstr("DATA OK\r\n\r\n");
	}

	// eeprom_read(HEADER_SIZE, data_tmp, 2);
	print_memory((uint8_t *)data, 2);

	custom_delay(100);
	eeprom_write(addr / 8, (uint8_t *)data, 2);
	custom_delay(100);
}

#define HEADER_SIZE 20
void write_ascii_uart(void)
{
	char c = '\0';
	char last_is_ret = 0;
	char palet[7] = {0};
	uint16_t current_index = 0;
	int i;

	while (c != '\n')
	{
		c = uart_rx();

		if (c == 13)
		{
			uart_printstr("\r\n");
			if (last_is_ret)
			{
				uart_printstr("\r\n");
				break;
			}
			// set_data_at_addr(7,  current_index * 3 + HEADER_SIZE * 8);

			uint8_t data_tmp;

			eeprom_read(HEADER_SIZE + (current_index / 2), (uint8_t *)&data_tmp, 1);

			if (current_index % 2 == 0)
			{
				data_tmp = (data_tmp & 0xF0) | 7;
			}
			else
			{
				data_tmp = (data_tmp & 0x0F) | (7 << 4);
			}
			// custom_delay(100);

			eeprom_write(HEADER_SIZE + (current_index / 2), (uint8_t *)&data_tmp, 1);

			// eeprom_write(HEADER_SIZE + current_index, (uint8_t*)&data_tmp, 1);

			current_index++;
			last_is_ret = 1;
		}
		else if ((c >= 32 && c < 126))
		{
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
				uart_printstr("ERROR\r\n\r\n");
			}
			uart_tx(c);
			// set_data_at_addr(i, current_index * 3 + HEADER_SIZE * 8);

			uint8_t data_tmp;
			eeprom_read(HEADER_SIZE + (current_index / 2), (uint8_t *)&data_tmp, 1);
			if (current_index % 2 == 0)
			{
				data_tmp = (data_tmp & 0xF0) | i;
			}
			else
			{
				data_tmp = (data_tmp & 0x0F) | (i << 4);
			}

			// custom_delay(100);
			eeprom_write(HEADER_SIZE + (current_index / 2), (uint8_t *)&data_tmp, 1);
			// custom_delay(100);

			// uint8_t data_tmp = i;
			// eeprom_write(HEADER_SIZE + current_index, (uint8_t*)&i, 1);
			current_index++;
			last_is_ret = 0;
		}
		else
		{
			uart_printstr("Unknown char\r\n");
		}
	}
	// current_index = 7;
	custom_delay(10);
	eeprom_write(0, (uint8_t *)palet, 7);
	custom_delay(10);
	eeprom_write(8, (uint8_t *)&current_index, 2);
	// uint8_t new_truc = 2;
	// eeprom_write(8, (uint8_t*)new_truc, 1);
	uart_printstr("End of write\r\n\r\n");
	uart_printstr("size :");
	uart_tx(current_index + '0');
	uart_printstr("palet :");
	uart_printstr(palet);
}

void read_ascii_uart()
{
	char palet[7] = {0};
	uint16_t max_size = 0;
	// uint8_t new_truc = 0;
	eeprom_read(0, (uint8_t *)palet, 7);
	eeprom_read(8, (uint8_t *)&max_size, 1);
	uint8_t c = 0;
	uart_printstr("size :");
	uart_tx(max_size + '0');
	uart_printstr("palet :");
	uart_printstr(palet);
	// uart_tx(max_size + '0');
	uart_printstr("\r\n");

	uint16_t data_tmp = 0;

	// eeprom_read(HEADER_SIZE, (uint8_t*)data_tmp, 16);
	// print_memory((uint8_t*)data_tmp, 16);

	for (uint16_t index = 0; index < max_size; index++)
	{
		// c = get_data_at_addr(HEADER_SIZE * 8 + index * 3);
		// eeprom_read(HEADER_SIZE + index, (uint8_t*)&c, 1);

		eeprom_read(HEADER_SIZE + (index / 2), (uint8_t *)&data_tmp, 1);
		if (index % 2 == 0)
		{
			c = data_tmp & 0x0F;
			// data_tmp = (data_tmp & 0xF0) | c;
		}
		else
		{
			c = (data_tmp & 0xF0) >> 4;
			// data_tmp = (data_tmp & 0x0F) | c << 4;
		}
		// eeprom_write(HEADER_SIZE + (index / 2), (uint8_t*)&data_tmp, 1);

		if (c == 7)
		{
			uart_printstr("\r\n");
		}
		else
		{
			uart_tx(palet[c]);
		}
	}
}

int main()
{
	uart_init(115200, SERIAL_8N1);
	char tmp_command[50];

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
			read_ascii_uart();
		}
		else if (str_comp(tmp_command, "write") == 0)
		{
			uart_printstr("\033[1;34mEntrez votre Ascii-Art\033[1;35m\r\n");
			write_ascii_uart();
		}
		else if (str_comp(tmp_command, "edit") == 0)
		{
			uart_printstr("\033[1;34mEdit \033[1;35m\r\n");
		}
		else if (str_comp(tmp_command, "eeprom w") == 0)
		{
			uart_printstr("\033[1;34mEeprom write \033[1;35m\r\n");
			uint16_t str[2] = {4, 0};
			eeprom_write(8, (uint8_t *)str, 2);
		}
		else if (str_comp(tmp_command, "eeprom r") == 0)
		{
			uart_printstr("\033[1;34mEeprom read \033[1;35m\r\n");
			uint16_t str[3] = {0};
			eeprom_read(8, (uint8_t *)str, 2);
			uart_tx(str[0] + '0');
			uart_printstr("\033[1;34m\r\n");
		}
		else if (str_comp(tmp_command, "test") == 0)
		{
			uint8_t i = 3;
			uint16_t data[] = {0};
			data[0] |= ((uint16_t)i << 12) & 0b0111000000000000;
			uart_printstr("\033[1;34mTest mem \033[1;35m\r\n");
			print_memory((uint8_t *)data, 2);
		}
		else
		{
			uart_printstr("\033[1;34mCommande inconnue \033[1;35m\r\n\r\n");
		}
		custom_delay(500);
	}
	return (0);
}