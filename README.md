# LovelyLamp

市售小黄灯用自制控制板, ATTINY13, 程序已使用3年,螺丝孔未适配

TINY13   9.6MHz不不分频;
OCR0A口输出PWM，休眠掉电模式，工作电流3.多毫安，休眠电流<2uA，按键中断唤醒;
短按开/关, 长按调光, 亮度值保存, 意外断电重新通电时恢复断电前开关状态。


工程文件是TKStudio(内含WinAVR), 也可以用vscode+winavr

(V1.0版的Gerber线路有所不同，按键引脚接在2脚(PB3))