/*该文件定义键盘控制板相关的硬件*/

#include "nrf_gpio.h"

//将GPIO映射至板子上的FPC座子脚位
//板子上的30PIN FPC母座从左往右数第二个开始定义为第0个，
#define PIN00 NRF_GPIO_PIN_MAP(0, 18) // Config as reset pin by macro CONFIG_GPIO_AS_PINRESET
#define PIN01 NRF_GPIO_PIN_MAP(0, 10)
#define PIN02 NRF_GPIO_PIN_MAP(0, 9)
#define PIN03 NRF_GPIO_PIN_MAP(0, 24)
#define PIN04 NRF_GPIO_PIN_MAP(0, 13)
#define PIN05 NRF_GPIO_PIN_MAP(1, 6)
#define PIN06 NRF_GPIO_PIN_MAP(1, 4)
#define PIN07 NRF_GPIO_PIN_MAP(1, 2)
#define PIN08 NRF_GPIO_PIN_MAP(1, 0)
#define PIN09 NRF_GPIO_PIN_MAP(0, 22)
#define PIN10 NRF_GPIO_PIN_MAP(0, 20)
#define PIN11 NRF_GPIO_PIN_MAP(0, 17)
#define PIN12 NRF_GPIO_PIN_MAP(0, 15)
#define PIN13 NRF_GPIO_PIN_MAP(1, 9)
#define PIN14 NRF_GPIO_PIN_MAP(0, 5)
#define PIN15 NRF_GPIO_PIN_MAP(0, 7)
#define PIN16 NRF_GPIO_PIN_MAP(0, 12)
#define PIN17 NRF_GPIO_PIN_MAP(0, 4)
#define PIN18 NRF_GPIO_PIN_MAP(0, 8)
#define PIN19 NRF_GPIO_PIN_MAP(0, 6)
#define PIN20 NRF_GPIO_PIN_MAP(0, 26)
#define PIN21 NRF_GPIO_PIN_MAP(0, 3)
#define PIN22 NRF_GPIO_PIN_MAP(0, 28)
#define PIN23 NRF_GPIO_PIN_MAP(1, 5)
#define PIN24 NRF_GPIO_PIN_MAP(0, 2)
#define PIN25 NRF_GPIO_PIN_MAP(0, 29)
#define PIN26 NRF_GPIO_PIN_MAP(0, 31)
#define PIN27 NRF_GPIO_PIN_MAP(0, 30)

/*大小写灯*/
#define CAPS_LED PIN01 // pin to control Capslock light
#define CAPS_LED_HIGH_ACT // 高电平点亮

//两脚双色LED灯
//蓝灯引脚（用于指示蓝牙状态）
#define BNR_LED_B PIN02
//红灯引脚（用于指示USB状态）
#define BNR_LED_R PIN03

//输出PWM信号控制单色背光灯引脚
#define BACKLIGHT_PIN NRF_GPIO_PIN_MAP(0, 25)
//背光使用的PWM设备
#define BACKLIGHT_PWM_INSTANCE 0

//用哪个Analog通道读电池电压
#define VOLTAGE_SOURCE NRF_SAADC_INPUT_VDDHDIV5 //使用VDDH脚读取