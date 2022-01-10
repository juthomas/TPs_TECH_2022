
#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#define F_CPU 16000000
#endif
#include <avr/io.h>

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

int main()
{
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
		if (!(PIND & 0b00000100))
		{
			while (!(PIND & 0b00000100))
				;
			custom_delay(50);
			switch (current_state)
			{
			case 0:
				OCR0A = 0xFF - 0xFF; //R
				OCR0B = 0xFF - 0x00; //G
				OCR2B = 0xFF - 0x00; //B
				break;
			case 1:
				OCR0A = 0xFF - 0x00; //R
				OCR0B = 0xFF - 0xFF; //G
				OCR2B = 0xFF - 0x00; //B
				break;
			case 2:
				OCR0A = 0xFF - 0x00; //R
				OCR0B = 0xFF - 0x00; //G
				OCR2B = 0xFF - 0xFF; //B
				break;
			case 3:
				OCR0A = 0xFF - 0xFF; //R
				OCR0B = 0xFF - 0xFF; //G
				OCR2B = 0xFF - 0x00; //B
				break;
			case 4:
				OCR0A = 0xFF - 0x00; //R
				OCR0B = 0xFF - 0xFF; //G
				OCR2B = 0xFF - 0xFF; //B
				break;
			case 5:
				OCR0A = 0xFF - 0xFF; //R
				OCR0B = 0xFF - 0x00; //G
				OCR2B = 0xFF - 0xFF; //B
				break;
			case 6:
				OCR0A = 0xFF - 0xFF; //R
				OCR0B = 0xFF - 0xFF; //G
				OCR2B = 0xFF - 0xFF; //B
				break;
			}
			current_state = current_state < 6 ? current_state + 1 : 0;
		}
	}
}