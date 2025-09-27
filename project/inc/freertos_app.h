/* add user code begin Header */
/**
  ******************************************************************************
  * File Name          : freertos_app.h
  * Description        : Code for freertos applications
  */
/* add user code end Header */
  
#ifndef FREERTOS_APP_H
#define FREERTOS_APP_H

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"
#include "wk_system.h"

/* private includes -------------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* exported types -------------------------------------------------------------*/
/* add user code begin exported types */

/* add user code end exported types */

/* exported constants --------------------------------------------------------*/
/* add user code begin exported constants */

/* add user code end exported constants */

/* exported macro ------------------------------------------------------------*/
/* add user code begin exported macro */

/* add user code end exported macro */

/* task handler */
extern TaskHandle_t temperature_task_handle;
extern TaskHandle_t current_task_handle;
extern TaskHandle_t process_task_handle;
extern TaskHandle_t check_task_handle;
extern TaskHandle_t update_task_handle;
extern TaskHandle_t key_task_handle;
/* declaration for task function */
void temperature_task_func(void *pvParameters);
void current_task_func(void *pvParameters);
void process_task_func(void *pvParameters);
void check_task_func(void *pvParameters);
void update_task_func(void *pvParameters);
void key_task_func(void *pvParameters);

/* queue handler */
extern QueueHandle_t usart_queue_handle;
extern QueueHandle_t key_queue_handle;

/* binary semaphore handler */
extern SemaphoreHandle_t dma_binary_sem_handle;

/* add user code begin 0 */
#define CLI_LINE_MAX   128
#define CLI_MAX_ARGS     8    // 最多支持 8 个参数

#define ADC_MAX_VALUE         4095.0f
#define VREF_MV               3300.0f      /* Vdda 电压，单位 mV */
#define R_REF_OHM             10000.0f      /* 分压固定电阻 Ω */
/* NTC 参数（R25=10kΩ, β=3950 K） */
#define BETA_K                3950.0f
#define R0_OHM                10000.0f
#define T0_K                  298.15f      /* 25°C = 298.15 K 开氏温度*/

/* add user code end 0 */

void freertos_task_create(void);
void freertos_queue_create(void);
void freertos_semaphore_create(void);
void wk_freertos_init(void);

/* add user code begin 1 */
//static const float R_table[TABLE_SIZE] = {
//    327019.5f, 254242.8f, 199200.7f, 15722.9f, 136882.5f, 124973.4f, 109241.7f, 100000.0f, 91633.3f,
//	84050.5f,  77170.7f,  70922.2f,  65241.1f, 60070.7f,  55360.4f,  51065.1f,  47144.3f,  43562.1f
//};
//static const float T_table[TABLE_SIZE] = {
//    0.0f,    5.0f,   10.0f,   15.0f,	18.0f,	20.0f,   23.0f,   25.0f,   27.0f,
//	29.0f,   31.0f,  33.0f,   35.0f,    37.0f,  39.0f,   41.0f,   43.0f,   45.0f
//};
float Read_Temperature_C(uint16_t CH_Value);
/* add user code end 1 */

#endif /* FREERTOS_APP_H */
