
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#define F_CPU 16000000
#endif
#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdlib.h>
// #include <avr/iom328p.h>
#define CPU_CLOCK 2000000 // 16Mhz -> / 8 2Mhz
#define SERIAL_8N1 0x06
#define NULL 0

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



void	wait_x_cpu_clocks(int32_t cpu_clocks)
{
	while (cpu_clocks > 0)
	{
		cpu_clocks-=3;
	}
}

void	custom_delay(uint32_t milli)
{
	//milli = 0,001s
	milli = milli *	2000;
	wait_x_cpu_clocks(milli - 5);
}

int		main()
{
	DDRB |= (1 << 3);
	DDRB |= (1 << 2);
	DDRB |= (1 << 1);
	DDRB |= (1 << 0);

	DDRD = 0b00000000;
	eeprom_read(0, &PORTB, 1);

	for(;;)
	{
		if (!(PIND & (1 << 2)))
		{
			while (!(PIND & (1 << 2)));
			PORTB = (PORTB & 0b00001111) == 0b00001111 ? PORTB & (0b11110000) : PORTB + 1;
			eeprom_write(0, &PORTB, 1);
			custom_delay(200);
		}
	}
	return (0);
}