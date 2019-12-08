#define uchar unsigned char
#define uint unsigned int

#define DEVICE_MARK 8

//��������������Ѽ��ػ���೤ʱ�����½�������
#define SLEEP_TIME_COUNT 2000

//�������ӵĶ˿�
#define KEY_PORT  PINB
#define KEY_PIN	   PB4
//�����
#define OUTPUT_PIN PB0
//ʹ�õ�PWM�˿ڶ�Ӧ�ļĴ���
#define  PWM_REG OCR0A

//����ɨ�������,��λ��ms
enum KeyIdleCount
{
	KEY_IDLE_FINISH 		= 20,	//����
	KEY_IDLE_ADJ 			= 250,	//���ȵ�����ֵ
	KEY_IDLE_MAX_COUNT 		= 10000,	//ֹͣ����
};
//����ɨ��ֵ
enum KeyValueData
{
	KEY_VALUE_NONE		= 0,	//�ް���
	KEY_VALUE_ON_OFF 	= 1,	//����
	KEY_VALUE_ADJ 		= 2,	//���ȵ���
	KEY_VALUE_ADJ_DONE	= 3,	//���ȵ������
//	KEY_VALUE_SETTING	= 4,	//����
};
//���ȿ���
enum BrightnessControl
{
//	DIRECTION_ADD = 0,			//���ȵ��ڷ�����
//	DIRECTION_SUB = 1,			//���ȵ��ڷ����,   ��ǰ������ʹ����/��
	BRIGHTNESS_ADJ_TIME_STEP = 12,//���ȵ����ٶ�(����)����λms ע��ӦС��255
	BRIGHTNESS_MIN	= 3,	//�������ֵ
	BRIGHTNESS_MAX	= 255,	//�������ֵ
	BRIGHTNESS_DEFAULT	= 180,	//Ĭ������
};

//��������eeprom�еĴ洢��ַ
enum DataSaveAddr
{
	SAVE_ADDR_DEVICE = 0x02,	//�豸���
	SAVE_ADDR_ON_OFF = 0x03,	//����״̬
	SAVE_ADDR_BRITHNESS = 0x04, //����ֵ
	SAVE_ADDR_CHECK_SUM = 0x05,	//У���,ռ��2�ֽڿռ�
};

uchar powerOnOff 	= 0;	//���ؿ���
uchar brightness 	= 0;	//����ֵ
uchar adjDirection	= 0;	//���ȵ��ڷ���

void PowerOn(void);
void PowerOff(void);
void LightAdj(void);
void SaveDataToEeprom(uchar *addr, uchar data);
void DoKeyAction(uchar keyValue);
uchar CheckKey(void);
uchar LoadConfig(void);
void SaveConfig(void);
void Init(void);
