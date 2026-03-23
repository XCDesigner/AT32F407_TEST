/* Includes ------------------------------------------------------------------*/
#include "at32f403a_407_wk_config.h"
#include "wk_adc.h"
#include "wk_dac.h"
#include "wk_debug.h"
#include "wk_tmr.h"
#include "wk_usart.h"
#include "wk_dma.h"
#include "wk_gpio.h"
#include "wk_system.h"
#include "freertos_app.h"

/* private includes ----------------------------------------------------------*/
#include "my_flash.h"

/* private variables ---------------------------------------------------------*/
/**
 * @brief ADC1常规通道采集值缓冲区
 * @note  存储连续5次采集的12个通道的ADC数值，__IO修饰表示易失性，防止编译器优化
 */
__IO uint16_t adc1_ordinary_valuetab[DMA1_CHANNEL1_BUFFER_SIZE] = {0};    

/* private function prototypes --------------------------------------------*/
/**
 * @brief  延时函数专用微秒计数基准
 */
static __IO uint32_t fac_us;
/**
 * @brief  延时函数专用毫秒计数基准
 */
static __IO uint32_t fac_ms;
/**
 * @brief  毫秒延时分段步进值，避免单次延时过长导致精度问题
 */
#define STEP_DELAY_MS 50

/**
 * @brief  初始化延时函数（基于SysTick和DWT）
 * @param  none
 * @retval none
 * @note   同时启用DWT周期计数器和SysTick定时器，支持us/ms级延时，
 *         基准频率为系统核心时钟（AHB时钟，无分频）
 */
void delay_init();

/**
 * @brief  毫秒级延时函数
 * @param  nms: 要延时的毫秒数
 * @retval none
 * @note   分段延时，单次最大延时STEP_DELAY_MS毫秒，避免SysTick重载值溢出
 */
void delay_ms(uint16_t nms);

/* private user code ---------------------------------------------------------*/
void delay_init()
{
  // 使能DWT调试组件（用于精确计时）
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  // 重置DWT周期计数器
  DWT->CYCCNT = 0;
  // 启用DWT周期计数器
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  // 配置SysTick时钟源为AHB时钟（无分频）
  systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
  // 计算1微秒对应的时钟周期数
  fac_us = system_core_clock / (1000000U);
  // 计算1毫秒对应的时钟周期数
  fac_ms = fac_us * (1000U);
}

void delay_ms(uint16_t nms)
{
  uint32_t temp = 0;
  while (nms)
  {
    // 分段处理长延时，避免SysTick LOAD寄存器溢出
    if (nms > STEP_DELAY_MS)
    {
      SysTick->LOAD = (uint32_t)(STEP_DELAY_MS * fac_ms);
      nms -= STEP_DELAY_MS;
    }
    else
    {
      SysTick->LOAD = (uint32_t)(nms * fac_ms);
      nms = 0;
    }
    // 清空SysTick当前值寄存器
    SysTick->VAL = 0x00;
    // 启用SysTick定时器
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    // 等待延时完成（检测COUNTFLAG标志位）
    do
    {
      temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));
    // 关闭SysTick定时器
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    // 清空SysTick当前值寄存器
    SysTick->VAL = 0x00;
  }
}

/**
 * @brief  主函数：程序入口点
 * @param  none
 * @retval int 理论返回值（嵌入式系统中通常不返回）
 * @note   完成系统初始化、外设配置、FreeRTOS启动，进入死循环等待任务调度
 */
int main(void)
{
  // 系统核心时钟配置（如倍频、分频，设置系统主频）
  wk_system_clock_config();
  // 外设时钟使能配置（开启各模块对应的时钟）
  wk_periph_clock_config();
  // 调试接口配置（如SWD/JTAG，用于调试器连接）
  wk_debug_config();
  // NVIC中断优先级配置（设置各中断的抢占/响应优先级）
  wk_nvic_config();
  // 时基初始化（如SysTick作为系统时基，用于任务调度/延时）
  wk_timebase_init();
  // GPIO引脚配置（初始化各IO口的模式、速度、上下拉等）
  wk_gpio_config();
  
  // 初始化DMA1通道1（用于ADC数据的DMA传输）
  wk_dma1_channel1_init();
  // 配置DMA1通道1：数据源(ADC1数据寄存器)、数据目标(缓冲区)、传输长度
  wk_dma_channel_config(DMA1_CHANNEL1, (uint32_t)&ADC1->odt, DMA1_CHANNEL1_MEMORY_BASE_ADDR, DMA1_CHANNEL1_BUFFER_SIZE);
  // 使能DMA1通道1
  dma_channel_enable(DMA1_CHANNEL1, TRUE);
  
  // 初始化USART2（串口2，用于数据收发/调试）
  wk_usart2_init();
  // 注：ADC1初始化暂未启用（根据需求可开启）
  // wk_adc1_init();
  // 初始化定时器1（用于定时/脉冲生成等）
  wk_tmr1_init();
  // 初始化定时器8（用于定时/脉冲生成等）
  wk_tmr8_init();
  // 初始化DAC（数模转换器，用于输出模拟电压）
  wk_dac_init();
  
  // 初始化延时函数
  delay_init();
  // 200ms延时（用于外设上电稳定/初始化完成等待）
  delay_ms(200);
  // 初始化FreeRTOS（创建任务、启动调度器）
  wk_freertos_init();

  // 死循环（FreeRTOS启动后，调度器接管，此处仅作为保底）
  while(1);
}