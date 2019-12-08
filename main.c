#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "DataValue.h"

#define CheckKeyPress (KEY_PORT & _BV(KEY_PIN))

void PowerOn(void)
{
//	uchar pwmRegtemp = BRIGHTNESS_MIN;
	if(brightness == BRIGHTNESS_MAX)
	{
		adjDirection = 1;
	}
	else
	{
		adjDirection = 0;
	}
	DDRB |= _BV(OUTPUT_PIN);
//	PORTB |= _BV(OUTPUT_PIN);

	PWM_REG = brightness;
	// PWM_REG = BRIGHTNESS_MIN;
	TCCR0A = 0xc3;
//	TCCR0A = 0x33;		//ʹ��OC0B�˿��������������OCR0B
	TCCR0B = 0x01;		//0x01 ƥ��ʱ���� top��255(����PWMģʽ) Ƶ�ʣ�9.6M/256=37.5K 4.8M/256=18.75K
	//����ʱ����
	// while(pwmRegtemp < brightness)
	// {
		// pwmRegtemp++;
		// PWM_REG = pwmRegtemp;
		// wdt_reset();
		// _delay_ms(5);
	// }
}

void PowerOff(void)
{
	 uchar pwmRegtemp = brightness;

	//�ص�ʱ����
	while(pwmRegtemp > BRIGHTNESS_MIN)
	{
		pwmRegtemp--;
		PWM_REG = pwmRegtemp;
		wdt_reset();
		_delay_ms(2);
	}

	TCCR0A = 0x00;
	TCCR0B = 0x00;	//��ʱ��ֹͣ
	PORTB |= _BV(OUTPUT_PIN);	//��Ϊ�ߵ�ƽ
}

void Init(void)
{
	//��ʼ���˿�
	PORTB = 0xff;
	//�����
	DDRB |= _BV(OUTPUT_PIN);
	//������
	DDRB &= ~_BV(KEY_PIN);
	//��ʼ��pcint�ж�
	GIMSK |= _BV(PCIE);
//	PCMSK |= _BV(PCINT4);
	PCMSK = _BV(PCINT4);//PCMSKĬ��ֵΪ���ж˿��ж�ȫ������������Ϊֻ��pcint4
}

void LightAdj(void)
{
	if(!adjDirection)
	{
		if(brightness < BRIGHTNESS_MAX)
		{
			brightness++;
		}
	}
	else
	{
		if(brightness > BRIGHTNESS_MIN)
		{
			brightness--;
		}
	}
	PWM_REG = brightness;
}

void SaveDataToEeprom(uchar *addr, uchar data)
{
	eeprom_busy_wait();
	eeprom_write_byte(addr, data);
}

void DoKeyAction(uchar keyValue)
{
	uchar keyValueCopy = keyValue;

	if(!powerOnOff && keyValueCopy != KEY_VALUE_ON_OFF)
	{
		keyValueCopy = KEY_VALUE_ON_OFF;		//�ػ�״̬ǿתΪ������ֵ
	}
	switch(keyValueCopy)
	{
		case KEY_VALUE_ON_OFF:
			powerOnOff = !powerOnOff;
			if(powerOnOff)
			{
				PowerOn();
			}
			else
			{
				PowerOff();
			}
			SaveDataToEeprom((uchar *)SAVE_ADDR_ON_OFF, powerOnOff);
			SaveConfig();
			break;
		case KEY_VALUE_ADJ:
			if(powerOnOff)
			{
				LightAdj();
			}
			break;
		case KEY_VALUE_ADJ_DONE:
			if(powerOnOff)
			{
				adjDirection = !adjDirection;
				SaveDataToEeprom((uchar *)SAVE_ADDR_BRITHNESS, brightness);
				SaveConfig();
			}
			break;
	}
}

//ÿ1ms����һ��
uchar CheckKey(void)
{
	uchar keyValue = KEY_VALUE_NONE;
	static uint idleCount;
	static uchar adjCount = BRIGHTNESS_ADJ_TIME_STEP;//����ֵ��Ϊ�˵�һ��ֱ�ӷ���

	if(!CheckKeyPress)
	{
		if(idleCount < KEY_IDLE_MAX_COUNT)
		{
			idleCount++;
		}
		//���������⴦�����û�п��������ȷ��Ϳ����ź�
		// if(idleCount == KEY_IDLE_FINISH)
		// {
			// if(!powerOnOff)
			// {
				// idleCount = KEY_IDLE_MAX_COUNT;	//�����е����Σ�
				// keyValue = KEY_VALUE_ON_OFF;
			// }
		//}
		if (idleCount >= KEY_IDLE_ADJ)
		{
			if(adjCount < BRIGHTNESS_ADJ_TIME_STEP)	//���BRIGHTNESS_ADJ_TIME_STEPʱ�䷵��һ��adj�ź�
			{
				adjCount++;
			}
			else
			{
				adjCount = 0;
				keyValue = KEY_VALUE_ADJ;
			}
		}
	}
	else
	{
		if(idleCount >= KEY_IDLE_FINISH && idleCount < KEY_IDLE_ADJ)
		{
			keyValue = KEY_VALUE_ON_OFF;
		}
		else if(idleCount >= KEY_IDLE_ADJ && idleCount <= KEY_IDLE_MAX_COUNT)
		{
			keyValue = KEY_VALUE_ADJ_DONE;
		}
		idleCount = 0;
	}

	return keyValue;
}

uchar LoadConfig(void)
{
	uchar deviceMarkValue = 0;
	uint checkSum = 0;

	eeprom_busy_wait();
	deviceMarkValue = eeprom_read_byte((uchar *)SAVE_ADDR_DEVICE);
	eeprom_busy_wait();
	powerOnOff = eeprom_read_byte((uchar *)SAVE_ADDR_ON_OFF);
	eeprom_busy_wait();
	brightness = eeprom_read_byte((uchar *)SAVE_ADDR_BRITHNESS);
	eeprom_busy_wait();
	checkSum = eeprom_read_word((uint *)SAVE_ADDR_CHECK_SUM);
	if(checkSum != deviceMarkValue + powerOnOff + brightness)return 0;
	return 1;
}

//Ϊ����дeeprom������������ǰ�ŵ����������������У�����ݣ����ڼ��洢�������Ƿ�����
void SaveConfig(void)
{
	eeprom_busy_wait();
	eeprom_write_word((uint *)SAVE_ADDR_CHECK_SUM, (DEVICE_MARK + powerOnOff + brightness));
}

void GetSafeConfig(void)
{
	brightness = BRIGHTNESS_DEFAULT;
	SaveDataToEeprom((uchar *)SAVE_ADDR_BRITHNESS,brightness);
	powerOnOff = 0;
	SaveDataToEeprom((uchar *)SAVE_ADDR_ON_OFF,powerOnOff);
	SaveDataToEeprom((uchar *)SAVE_ADDR_DEVICE,DEVICE_MARK);
}

int main(void)
{
	uchar keyValue = 0;
	uchar configDataisOk = 0;
	uint sleepCount = 0;

//	_delay_ms(20);
	wdt_disable();
	Init();
	configDataisOk = LoadConfig();
	if(!configDataisOk)GetSafeConfig();
	else if(powerOnOff)
	{
		PowerOn();
	}
	wdt_enable(WDTO_30MS);
	while(1)
	{
		keyValue = CheckKey();
		if (keyValue)
		{
			DoKeyAction(keyValue);
		}
		else if(!powerOnOff)
		{
			sleepCount++;
			if(sleepCount == SLEEP_TIME_COUNT)
			{
				wdt_disable();
				sei();
				set_sleep_mode(SLEEP_MODE_PWR_DOWN);
				sleep_mode();
				cli();
				wdt_enable(WDTO_30MS);
			}
		}
		else
		{
			sleepCount = 0;
		}
		if(powerOnOff + adjDirection <= 2 && brightness >= BRIGHTNESS_MIN)wdt_reset();
		else
		{
			while(1);	//wait to die here
		}
		_delay_ms(1);
	}
	return 0;
}

ISR(PCINT0_vect)
{
	sleep_disable();
	//donothing here,just wake up
}
