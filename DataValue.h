#define uchar unsigned char
#define uint unsigned int

#define DEVICE_MARK 8

//这个决定按键唤醒及关机后多长时间重新进入休眠
#define SLEEP_TIME_COUNT 2000

//按键连接的端口
#define KEY_PORT  PINB
#define KEY_PIN	   PB4
//输出端
#define OUTPUT_PIN PB0
//使用的PWM端口对应的寄存器
#define  PWM_REG OCR0A

//按键扫描控制量,单位是ms
enum KeyIdleCount
{
	KEY_IDLE_FINISH 		= 20,	//消抖
	KEY_IDLE_ADJ 			= 250,	//亮度调节阈值
	KEY_IDLE_MAX_COUNT 		= 10000,	//停止计数
};
//按键扫描值
enum KeyValueData
{
	KEY_VALUE_NONE		= 0,	//无按键
	KEY_VALUE_ON_OFF 	= 1,	//开关
	KEY_VALUE_ADJ 		= 2,	//亮度调节
	KEY_VALUE_ADJ_DONE	= 3,	//亮度调节完毕
//	KEY_VALUE_SETTING	= 4,	//设置
};
//亮度控制
enum BrightnessControl
{
//	DIRECTION_ADD = 0,			//亮度调节方向增
//	DIRECTION_SUB = 1,			//亮度调节方向减,   当前程序中使用真/假
	BRIGHTNESS_ADJ_TIME_STEP = 12,//亮度调节速度(近似)，单位ms 注：应小于255
	BRIGHTNESS_MIN	= 3,	//最低亮度值
	BRIGHTNESS_MAX	= 255,	//最高亮度值
	BRIGHTNESS_DEFAULT	= 180,	//默认亮度
};

//各参数在eeprom中的存储地址
enum DataSaveAddr
{
	SAVE_ADDR_DEVICE = 0x02,	//设备标记
	SAVE_ADDR_ON_OFF = 0x03,	//开关状态
	SAVE_ADDR_BRITHNESS = 0x04, //亮度值
	SAVE_ADDR_CHECK_SUM = 0x05,	//校验和,占用2字节空间
};

uchar powerOnOff 	= 0;	//开关控制
uchar brightness 	= 0;	//亮度值
uchar adjDirection	= 0;	//亮度调节方向

void PowerOn(void);
void PowerOff(void);
void LightAdj(void);
void SaveDataToEeprom(uchar *addr, uchar data);
void DoKeyAction(uchar keyValue);
uchar CheckKey(void);
uchar LoadConfig(void);
void SaveConfig(void);
void Init(void);
