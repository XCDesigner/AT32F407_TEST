#include "my_flash.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#include "at32f403a_407_wk_config.h"
#include "freertos_app.h"

static const uint32_t TIMER_ARR = 4799;
volatile uint16_t lastDuty[8] = {0,0,0,0,100,100,100,100};

// 默认配置，放到 FLASH 区段
 DeviceConfig_t runConfig = {
	8,                      // 定时器编号
	4,                      // PWM 通道号，例：TMR_SELECT_CHANNEL_1
    1,                      // DAC 号，例：DAC1_SELECT
    100,                     // PWM 占空比，0~100 对应 0%~100%
	50000,                  // PWM 频率，单位 Hz
	500,                    //DAC 输出大小 例：0-4095
	0,                      //用于判断下次FLASH写入的地址
	0	                    // CRC32 校验码
};

// 上电或第一次写前，扫描扇区找最大 flag
static uint8_t Flash_DetectLastFlag(void)
{
    uint8_t  maxFlag = 0xFF;
    DeviceConfig_t tmp;
    uint8_t i;
	int bestIdx = -1;
	uint32_t crcCalc;
	
    for (i = 0; i < FLASH_SLOT_COUNT; i++) {
        uint32_t addr = FLASH_SECTOR_ADDR + i * FLASH_SLOT_SIZE;
        memcpy(&tmp, (void *)addr, sizeof(tmp));
		
//		crcCalc = CRC32_Calc(&tmp, offsetof(DeviceConfig_t, crc));
//        if (crcCalc != tmp.crc) {
//            continue;  // CRC 不通过，跳过
//        }
		
        // 判断这个扇区里有没有写过合法的 flag
        if (tmp.flag < FLASH_SLOT_COUNT &&
			(bestIdx < 0 || tmp.flag > maxFlag)) {
			{     
                maxFlag = tmp.flag;
				bestIdx = i;
            }
        }
    }
    return bestIdx;
}

void Flash_WriteConfig(const DeviceConfig_t *cfg) {
    DeviceConfig_t tmp = *cfg;   	//在栈上拷贝一个本地副本
	
	uint32_t i,baseAddr;
    uint32_t wordCnt = (sizeof(DeviceConfig_t) + 3) / 4;
    const uint32_t *p = (const uint32_t *)&tmp;
	static uint8_t  sectorFlag = 0;       //静态变量用来保存下一次写入的扇区索引
	static bool     firstInit  = true;   
	
	
	// 第一次调用前，恢复上次写到的扇区索引
    if (firstInit) {
        int idx = Flash_DetectLastFlag();
        sectorFlag = (idx < 0) ? 0 : ((idx + 1) % FLASH_SLOT_COUNT);
        firstInit  = false;
    }
	
	tmp.flag = sectorFlag;
	tmp.crc  = CRC32_Calc(&tmp, offsetof(DeviceConfig_t, crc));
	baseAddr = FLASH_SECTOR_ADDR + sectorFlag * FLASH_SLOT_SIZE;	   //本次要写入的扇区起始地址

	flash_bank2_unlock();
	if (sectorFlag == 0){
    flash_sector_erase(baseAddr);
	}
		for (i = 0; i < wordCnt; i++) {                 //按 32-bit 写入整个配置结构
      flash_word_program(baseAddr + i*4, p[i]);
	}
	
    flash_bank2_lock();
  
	sectorFlag = (sectorFlag + 1) % FLASH_SLOT_COUNT;
}

/**
 * @brief  打印 Flash 指定区域的十六进制内容
 * @param  address   起始地址
 * @param  length    打印长度（字节）
 */
void Flash_Dump(uint32_t address, uint32_t length)
{  
	uint32_t i;
	DeviceConfig_t tmp;
	volatile uint8_t *p = (volatile uint8_t *)address;
	memcpy(&tmp, (void *)address, sizeof(tmp));
	
//	crcCalc = CRC32_Calc(&tmp, offsetof(DeviceConfig_t, crc));	

    for (i = 0; i < length; i++) {// 每行 16 字节，打印地址
        if ((i & 0x0F) == 0) {
            printf("\r\n%08lX: ", (unsigned long)(address + i));
        }
        printf("%02X ", p[i]);
    }
}

/**
 * @brief  打印 Flash 指定扇区内所有slot内容
 * @param  NULL
 */
void Flash_DumpAllSlots(void)
{  
	uint32_t i,crcCalc;
	uint8_t slot;
	DeviceConfig_t tmp;
	uint8_t  *flashBase = (uint8_t *)FLASH_SECTOR_ADDR;
	
	for (slot = 0; slot < FLASH_SLOT_COUNT; slot++) {
		
    uint32_t slotAddr = FLASH_SECTOR_ADDR + slot * FLASH_SLOT_SIZE;
		
	memcpy(&tmp, (void *)slotAddr, sizeof(tmp));
	
	crcCalc = CRC32_Calc(&tmp, offsetof(DeviceConfig_t, crc));	

	for (i = 0; i < FLASH_SLOT_SIZE; i++) {
        if ((i & 0x0F) == 0) {
            printf("\r\n%08lX: ", (unsigned long)(slotAddr + i));
        }
		printf("%02X ", flashBase[slot * FLASH_SLOT_SIZE + i]);
    }	
		
	printf("%s\r\n",(crcCalc == tmp.crc) ? "CRC OK" : "CRC FAIL");	
		
  }
}

//映射表方式，一次性拿到定时器指针
static const tmr_type* TMR_Instances[8] = {
    TMR1, TMR2, TMR3, TMR4,
    TMR5, TMR6, TMR7, TMR8
};

//通道宏静态映射
static const uint16_t TMR_ChannelMap[4] = {
    TMR_SELECT_CHANNEL_1,
    TMR_SELECT_CHANNEL_2,
    TMR_SELECT_CHANNEL_3,
    TMR_SELECT_CHANNEL_4
};

/**
 * @brief  更新 PWM 占空比（0~100，对应 0~100%）。
 * @param  TMRx        定时器实例
 * @param  channel     通道编号（TMR_CH1 ~ TMR_CH4）
 * @param  duty_PerMille   占空比百分比（0~100）
 */
void PWM_Update_Duty(const DeviceConfig_t *cfg){ 
	tmr_type *TMRx = (tmr_type *)TMR_Instances[cfg->pwm_timer_id - 1];
    uint16_t  chMx = TMR_ChannelMap[cfg->pwm_channel_id - 1];
	uint8_t index,i;
	uint32_t timerClk = SystemCoreClock;    
	// 计算 ARR（自动重装载值）和 CCR（比较值）
														 
    uint32_t ccr = (uint32_t)(( cfg->pwm_duty) * (TIMER_ARR+1)) / 100;    //CCR
	
	/*暂停计数，更新周期并写入比较值，再重启 */
    tmr_counter_enable(TMRx, FALSE);
	
    /* 更新通道比较值 */
    tmr_channel_value_set(TMRx, chMx, ccr);
	
	// 存储最新PWM脉宽到 lastDuty[]
	if(cfg->pwm_timer_id == 3){
		 index = (cfg->pwm_channel_id - 1);            // CH1→idx=0, CH4→idx=3 
	}
	else{
		 index = 4 + (cfg->pwm_channel_id - 1);
	}
	lastDuty[index] = cfg->pwm_duty;
	
    /* 重启定时器，新的 PWM 立即生效 */
    tmr_counter_enable(TMRx, TRUE);
//	for ( i = 4; i <= 7; i++)
//    printf("restore lastDuty[%d] = %d\n", i, lastDuty[i]);

}
/**
 * @brief  更新 PWM 频率（Hz）。
 * @param  TMRx        定时器实例
 * @param  channel     通道编号（此函数不改 CCR）
 * @param  freq_Hz     目标频率，单位 Hz
 */
void PWM_Update_Fre(const DeviceConfig_t *cfg){ 
	tmr_type *TMRx = (tmr_type *)TMR_Instances[cfg->pwm_timer_id - 1];
    uint16_t  chMx = TMR_ChannelMap[cfg->pwm_channel_id - 1];
	
	uint32_t timerClk = SystemCoreClock;    
	// 计算 ARR（自动重装载值）和 CCR（比较值）
	uint64_t tmp  =  timerClk / ((cfg->pwm_frequency)* (TIMER_ARR + 1));													 
	uint32_t psc  = (tmp > 0) ? (tmp - 1) : 0;    // PSC
	
	/*暂停计数，更新周期并写入比较值，再重启 */
    tmr_counter_enable(TMRx, FALSE);
	
	/* 重载周期 (ARR) 和预分频 (PSC)，然后立即产生更新事件*/
    tmr_div_value_set(TMRx, psc);
	tmr_event_sw_trigger(TMRx,TMR_TRIGGER_SWTRIG);
	

    /* 重启定时器，新的 PWM 立即生效 */
    tmr_counter_enable(TMRx, TRUE);
}

void DAC_Update(const DeviceConfig_t *cfg){
	if(cfg->dac_id == 1)
	{
		dac_1_data_set(DAC1_12BIT_RIGHT,cfg->dac_otput_value);
		dac_software_trigger_generate(DAC1_SELECT);
	}
	else if(cfg->dac_id == 2)
	{
		dac_2_data_set(DAC2_12BIT_RIGHT,cfg->dac_otput_value);
		dac_software_trigger_generate(DAC1_SELECT);
	}
}

void KEY_Update(void){
	int recvKey;
	if (xQueueReceive(key_queue_handle, &recvKey, pdMS_TO_TICKS(10)) == pdTRUE)     // 成功接收，可以处理 recvKey
		{ 
			
		   if(recvKey == 1){         //Key1按下  使能
//			   tmr_counter_enable(TMR1, FALSE);              //PWM恢复
//			   tmr_counter_enable(TMR8, FALSE);
			   
//			   tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_1, lastDuty[0]);
//			   tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_2, lastDuty[1]);
//			   tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_3, lastDuty[2]);
//             tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_4, lastDuty[3]);
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_1, lastDuty[4]);
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_2, lastDuty[5]);
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_3, lastDuty[6]);
////			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_4, lastDuty[7]);
//			   tmr_event_sw_trigger(TMR8,TMR_TRIGGER_SWTRIG);
//			   tmr_counter_enable(TMR1, TRUE);
//			   tmr_counter_enable(TMR8, TRUE);
			   
//			   gpio_bits_set(GPIOB, GPIO_PINS_3);//IO使能
//			   gpio_bits_set(GPIOB, GPIO_PINS_4);
//			   gpio_bits_set(GPIOB, GPIO_PINS_5);
			   gpio_bits_set(GPIOB, GPIO_PINS_6);
			   gpio_bits_set(GPIOD, GPIO_PINS_6);
		   }
		   else if(recvKey == -1){         //Key1弹起
//			   tmr_counter_enable(TMR1, FALSE);          //PWM，脉宽为0
//			   tmr_counter_enable(TMR8, FALSE);
			   
//			   tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_1, 0);
//			   tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_2, 0);
//			   tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_3, 0);
//			   tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_4, 0);
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_1, 0);
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_2, 0);
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_3, 0);
////			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_4, 0);
//		       tmr_event_sw_trigger(TMR1,TMR_TRIGGER_SWTRIG);
//			   tmr_event_sw_trigger(TMR8,TMR_TRIGGER_SWTRIG);
			   
//			   tmr_counter_enable(TMR1, TRUE);
//			   tmr_counter_enable(TMR8, TRUE);     
			   
			   gpio_bits_reset(GPIOB, GPIO_PINS_6);//IO失能
			   gpio_bits_reset(GPIOD, GPIO_PINS_6);
		   }
		   else if(recvKey == 2)
		   {
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_3, lastDuty[6]);
//			   tmr_event_sw_trigger(TMR8,TMR_TRIGGER_SWTRIG);
//			   tmr_counter_enable(TMR8, TRUE);
			   gpio_bits_set(GPIOB, GPIO_PINS_5);
			   gpio_bits_set(GPIOD, GPIO_PINS_5);
		   }
		   else if(recvKey == -2)
		   {			  
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_3, 0);
//			   tmr_event_sw_trigger(TMR8,TMR_TRIGGER_SWTRIG);
//			   tmr_counter_enable(TMR8, TRUE);
			   gpio_bits_reset(GPIOB, GPIO_PINS_5);
			   gpio_bits_reset(GPIOD, GPIO_PINS_5);
		   }
		   else if(recvKey == 3)
		   {
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_2, lastDuty[5]);
//			   tmr_event_sw_trigger(TMR8,TMR_TRIGGER_SWTRIG);
//			   tmr_counter_enable(TMR8, TRUE);
			   gpio_bits_set(GPIOB, GPIO_PINS_4);
			   gpio_bits_set(GPIOD, GPIO_PINS_4);
		   } 
		   else if(recvKey == -3)
		   {
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_2, 0);
//			   tmr_event_sw_trigger(TMR8,TMR_TRIGGER_SWTRIG);
//			   tmr_counter_enable(TMR8, TRUE);
			   gpio_bits_reset(GPIOB, GPIO_PINS_4);
			   gpio_bits_reset(GPIOD, GPIO_PINS_4);
		   }
		   else if(recvKey == 4)
		   {
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_1, lastDuty[4]);
//			   tmr_event_sw_trigger(TMR8,TMR_TRIGGER_SWTRIG);
//			   tmr_counter_enable(TMR8, TRUE);
			   gpio_bits_set(GPIOB, GPIO_PINS_3);
			   gpio_bits_set(GPIOD, GPIO_PINS_3);
		   } 
		   else if(recvKey == -4)
		   {
//			   tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_1, 0);
//			   tmr_event_sw_trigger(TMR8,TMR_TRIGGER_SWTRIG);
//			   tmr_counter_enable(TMR8, TRUE);
			   gpio_bits_reset(GPIOB, GPIO_PINS_3);
			   gpio_bits_reset(GPIOD, GPIO_PINS_3);
		   }
		}


}

// 简易 CRC32 (多项式 0x04C11DB7)
static uint32_t CRC32_Table[256];

// 在系统启动时调用，一次性填表
void CRC32_Init(void) {
	uint32_t i,crc;
	uint8_t j;
    uint32_t poly = 0x04C11DB7;
    for (i = 0; i < 256; i++) {
        crc = i << 24;
        for (j = 0; j < 8; j++) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ poly;
            } else {
                crc <<= 1;
            }
        }
        CRC32_Table[i] = crc;
    }
}

// 基于查表的 CRC32 计算：buf 长度为 len 字节(crc偏移字节数)
uint32_t CRC32_Calc(const void *buf, uint32_t len) {
    const uint8_t *ptr = buf;
    uint32_t crc = 0xFFFFFFFF;
    while (len--) {
        uint8_t idx = (crc >> 24) ^ *ptr++;
        crc = (crc << 8) ^ CRC32_Table[idx];
    }
    return ~crc;
}
