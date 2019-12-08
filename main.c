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
//	TCCR0A = 0x33;		//使用OC0B端口则开启这个并设置OCR0B
	TCCR0B = 0x01;		//0x01 匹配时清零 top：255(快速PWM模式) 频率：9.6M/256=37.5K 4.8M/256=18.75K
	//开灯时渐亮
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

	//关灯时渐暗
	while(pwmRegtemp > BRIGHTNESS_MIN)
	{
		pwmRegtemp--;
		PWM_REG = pwmRegtemp;
		wdt_reset();
		_delay_ms(2);
	}

	TCCR0A = 0x00;
	TCCR0B = 0x00;	//定时器停止
	PORTB |= _BV(OUTPUT_PIN);	//置为高电平
}

void Init(void)
{
	//初始化端口
	PORTB = 0xff;
	//输出口
	DDRB |= _BV(OUTPUT_PIN);
	//按键口
	DDRB &= ~_BV(KEY_PIN);
	//初始化pcint中断
	GIMSK |= _BV(PCIE);
//	PCMSK |= _BV(PCINT4);
	PCMSK = _BV(PCINT4);//PCMSK默认值为所有端口中断全开，现在设置为只开pcint4
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
		keyValueCopy = KEY_VALUE_ON_OFF;		//关机状态强转为开机键值
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

//每1ms调用一次
uchar CheckKey(void)
{
	uchar keyValue = KEY_VALUE_NONE;
	static uint idleCount;
	static uchar adjCount = BRIGHTNESS_ADJ_TIME_STEP;//赋初值是为了第一次直接发送

	if(!CheckKeyPress)
	{
		if(idleCount < KEY_IDLE_MAX_COUNT)
		{
			idleCount++;
		}
		//这里做特殊处理，如果没有开机，优先发送开机信号
		// if(idleCount == KEY_IDLE_FINISH)
		// {
			// if(!powerOnOff)
			// {
				// idleCount = KEY_IDLE_MAX_COUNT;	//这里有点无奈，
				// keyValue = KEY_VALUE_ON_OFF;
			// }
		//}
		if (idleCount >= KEY_IDLE_ADJ)
		{
			if(adjCount < BRIGHTNESS_ADJ_TIME_STEP)	//间隔BRIGHTNESS_ADJ_TIME_STEP时间返回一次adj信号
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

//为减少写eeprom次数，在休眠前才调用这个函数，保存校验数据，用于检查存储的数据是否正常
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
