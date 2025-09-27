#ifndef __MY_FLASH_H
#define __MY_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f403a_407_wk_config.h"
#include "at32f403a_407_adc.h"

// FLASH 区段地址及扇区
#define CONFIG_FLASH_SECTOR FLASH_Bank2_Sector_1
#define FLASH_SECTOR_ADDR  ((uint32_t)0x08080000) // 起始扇区地址
#define FLASH_SECTOR_SIZE  2*1024 				  // 单扇区大小, 单位字节
#define FLASH_SECTOR_COUNT  10                    // 准备轮询写入的扇区数
#define FLASH_SLOT_SIZE     16
#define FLASH_SLOT_COUNT    128
#define ADC_CHANNEL_NUM     12                    //ADC通道数

typedef struct __attribute__((packed)){
//	uint16_t adc_channel_id;        // ADC 通道号，例：ADC_CHANNEL_1
//    uint16_t adc_sampling_time;  // ADC 采样时间，例：15=ADC_SAMPLETIME_15CYCLES
	uint8_t pwm_timer_id;     	  	// 定时器编号
	uint8_t pwm_channel_id;      	// PWM 通道号，例：TMR_SELECT_CHANNEL_1
	uint8_t dac_id;            		// DAC 号，例：DAC1_SELECT
	uint8_t pwm_duty;           	// PWM 占空比，0~100 对应 0%~100%
    uint32_t pwm_frequency;      	// PWM 频率，单位 Hz
	uint16_t dac_otput_value;    	//DAC 输出大小 例：0-4095
	uint16_t flag;                	//用于判断下次FLASH写入的地址
	uint32_t crc;                 	// CRC32 校验码
	
} DeviceConfig_t;

extern volatile uint16_t lastDuty[8];
 //最新一次各通道PWM脉宽  lastDuty[0..3] 对应 TMR3 CH1..4  lastDuty[4..7] 对应 TMR8 CH1..4


extern  DeviceConfig_t runConfig;

void Flash_WriteConfig(const DeviceConfig_t *cfg);

void PWM_Update(const DeviceConfig_t *cfg);

void PWM_Update_Duty(const DeviceConfig_t *cfg);

void PWM_Update_Fre(const DeviceConfig_t *cfg);

void DAC_Update(const DeviceConfig_t *cfg);

void KEY_Update(void);

void Flash_Dump(uint32_t address, uint32_t length);

void Flash_DumpAllSlots(void);

void CRC32_Init(void);

uint32_t CRC32_Calc(const void *buf, uint32_t len);



#ifdef __cplusplus
}
#endif

#endif
