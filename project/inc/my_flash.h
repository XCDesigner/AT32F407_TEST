#ifndef __MY_FLASH_H
#define __MY_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f403a_407_wk_config.h"
#include "at32f403a_407_adc.h"

// FLASH ���ε�ַ������
#define CONFIG_FLASH_SECTOR FLASH_Bank2_Sector_1
#define FLASH_SECTOR_ADDR  ((uint32_t)0x08080000) // ��ʼ������ַ
#define FLASH_SECTOR_SIZE  2*1024 				  // ��������С, ��λ�ֽ�
#define FLASH_SECTOR_COUNT  10                    // ׼����ѯд���������
#define FLASH_SLOT_SIZE     16
#define FLASH_SLOT_COUNT    128
#define ADC_CHANNEL_NUM     12                    //ADCͨ����

typedef struct __attribute__((packed)){
//	uint16_t adc_channel_id;        // ADC ͨ���ţ�����ADC_CHANNEL_1
//    uint16_t adc_sampling_time;  // ADC ����ʱ�䣬����15=ADC_SAMPLETIME_15CYCLES
	uint8_t pwm_timer_id;     	  	// ��ʱ�����
	uint8_t pwm_channel_id;      	// PWM ͨ���ţ�����TMR_SELECT_CHANNEL_1
	uint8_t dac_id;            		// DAC �ţ�����DAC1_SELECT
	uint8_t pwm_duty;           	// PWM ռ�ձȣ�0~100 ��Ӧ 0%~100%
    uint32_t pwm_frequency;      	// PWM Ƶ�ʣ���λ Hz
	uint16_t dac_otput_value;    	//DAC �����С ����0-4095
	uint16_t flag;                	//�����ж��´�FLASHд��ĵ�ַ
	uint32_t crc;                 	// CRC32 У����
	
} DeviceConfig_t;

extern volatile uint16_t lastDuty[8];
 //����һ�θ�ͨ��PWM����  lastDuty[0..3] ��Ӧ TMR3 CH1..4  lastDuty[4..7] ��Ӧ TMR8 CH1..4


extern  DeviceConfig_t runConfig;

void Flash_WriteConfig(const DeviceConfig_t *cfg);

void PWM_Update(const DeviceConfig_t *cfg);

void PWM_Update_Duty(const DeviceConfig_t *cfg);

void PWM_Update_Fre(const DeviceConfig_t *cfg);

void DAC_Update(const DeviceConfig_t *cfg);

void KEY_Update(void);

void Flash_Dump(uint32_t address, uint32_t length);

void Flash_DumpAllSlots(void);


#ifdef __cplusplus
}
#endif

#endif
