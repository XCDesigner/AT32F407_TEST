/* add user code begin Header */
/**
  ******************************************************************************
  * File Name          : freertos_app.c
  * Description        : Code for freertos applications
  */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "freertos_app.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "my_flash.h"
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */


/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */
   extern __IO uint16_t adc1_ordinary_valuetab[5];

   static char *cli_argv[CLI_MAX_ARGS];
   static int   cli_argc;
   	
   char line_buf[CLI_LINE_MAX];
   
    uint16_t CH1_Value  = 0;
	uint16_t CH2_Value  = 0;
	uint16_t CH3_Value  = 0;
	uint16_t CH4_Value  = 0;
	uint16_t CH5_Value  = 0;
	uint16_t CH6_Value  = 0;
	uint16_t CH7_Value  = 0;
	uint16_t CH8_Value  = 0;
	uint16_t CH9_Value  = 0;
	uint16_t CH10_Value = 0;
	uint16_t CH11_Value = 0;
	uint16_t CH12_Value = 0;
/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */
  void CLI_ProcessLine(char *line);

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */


/* add user code end 0 */

/* task handler */
TaskHandle_t temperature_task_handle;
TaskHandle_t current_task_handle;
TaskHandle_t process_task_handle;
TaskHandle_t check_task_handle;
TaskHandle_t update_task_handle;
TaskHandle_t key_task_handle;

/* queue handler */
QueueHandle_t usart_queue_handle;
QueueHandle_t key_queue_handle;

/* binary semaphore handler */
SemaphoreHandle_t dma_binary_sem_handle;

/* Idle task control block and stack */
static StackType_t idle_task_stack[configMINIMAL_STACK_SIZE];
static StackType_t timer_task_stack[configTIMER_TASK_STACK_DEPTH];

static StaticTask_t idle_task_tcb;
static StaticTask_t timer_task_tcb;

/* External Idle and Timer task static memory allocation functions */
extern void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );
extern void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t * pulTimerTaskStackSize );
/*
  vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &idle_task_tcb;
  *ppxIdleTaskStackBuffer = &idle_task_stack[0];
  *pulIdleTaskStackSize = (uint32_t)configMINIMAL_STACK_SIZE;
}
/*
  vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t * pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &timer_task_tcb;
  *ppxTimerTaskStackBuffer = &timer_task_stack[0];
  *pulTimerTaskStackSize = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}

/* add user code begin 1 */

/* add user code end 1 */

/**
  * @brief  initializes all task.
  * @param  none
  * @retval none
  */
void freertos_task_create()
{
  /* create temperature_task task */
  // xTaskCreate(temperature_task_func,
  //             "temperature_task",
  //             128,
  //             NULL,
  //             9,
  //             &temperature_task_handle);

  /* create current_task task */
  xTaskCreate(current_task_func,
              "current_task",
              128,
              NULL,
              0,
              &current_task_handle);

  /* create process_task task */
  xTaskCreate(process_task_func,
              "process_task",
              128,
              NULL,
              8,
              &process_task_handle);

  /* create check_task task */
  xTaskCreate(check_task_func,
              "check_task",
              128,
              NULL,
              6,
              &check_task_handle);

  /* create update_task task */
  xTaskCreate(update_task_func,
              "update_task",
              128,
              NULL,
              7,
              &update_task_handle);

  /* create key_task task */
  xTaskCreate(key_task_func,
              "key_task",
              128,
              NULL,
              7,
              &key_task_handle);
}

/**
  * @brief  initializes all queue.
  * @param  none
  * @retval none
  */
void freertos_queue_create(void)
{
  /* Create the usart_queue, storing the returned handle in the xQueue variable. */
  usart_queue_handle = xQueueCreate(128, sizeof(char));

  /* Create the key_queue, storing the returned handle in the xQueue variable. */
  key_queue_handle = xQueueCreate(16, sizeof(int));
}

/**
  * @brief  initializes all semaphore.
  * @param  none
  * @retval none
  */
void freertos_semaphore_create(void)
{
  /* Create the dma_binary_sem */
  dma_binary_sem_handle = xSemaphoreCreateBinary();
}



/**
  * @brief  freertos init and begin run.
  * @param  none
  * @retval none
  */
void wk_freertos_init(void)
{
  /* enter critical */
  taskENTER_CRITICAL();

  freertos_semaphore_create();
  freertos_queue_create();

  freertos_task_create();
	
  /* exit critical */
  taskEXIT_CRITICAL();

  /* start scheduler */
  vTaskStartScheduler();
}

/**
  * @brief temperature_task function.
  * @param  none
  * @retval none
  */
void temperature_task_func(void *pvParameters)
{
	int i;
	adc_ordinary_software_trigger_enable(ADC1, TRUE);
  while(1)
  {
  /* add user code begin temperature_task_func 1 */
	 if (xSemaphoreTake(dma_binary_sem_handle, pdMS_TO_TICKS(10)) == pdTRUE)
    {   
		CH1_Value  = 0;
		CH2_Value  = 0;
		CH3_Value  = 0;
		CH4_Value  = 0;
		CH5_Value  = 0;
		CH6_Value  = 0;
		CH7_Value  = 0;
		CH8_Value  = 0;
		CH9_Value  = 0;
		CH10_Value = 0;
		CH11_Value = 0;
		CH12_Value = 0;
		for(i=0; i<5; i++)
		{   
			int base = i * ADC_CHANNEL_NUM;
			CH1_Value  += adc1_ordinary_valuetab[base + 0];
			CH2_Value  += adc1_ordinary_valuetab[base + 1];
			CH3_Value  += adc1_ordinary_valuetab[base + 2];
			CH4_Value  += adc1_ordinary_valuetab[base + 3];
			CH5_Value  += adc1_ordinary_valuetab[base + 4];
			CH6_Value  += adc1_ordinary_valuetab[base + 5];
			CH7_Value  += adc1_ordinary_valuetab[base + 6];
			CH8_Value  += adc1_ordinary_valuetab[base + 7];
			CH9_Value  += adc1_ordinary_valuetab[base + 8];
			CH10_Value += adc1_ordinary_valuetab[base + 9];
			CH11_Value += adc1_ordinary_valuetab[base + 10];
			CH12_Value += adc1_ordinary_valuetab[base + 11];
		}	
	 }
	adc_ordinary_software_trigger_enable(ADC1, TRUE);
    vTaskDelay(100);
  }
}


/**
  * @brief current_task function.
  * @param  none
  * @retval none
  */
void current_task_func(void *pvParameters)
{
  runConfig.pwm_timer_id = (uint16_t)8;
  runConfig.pwm_channel_id = 4;
  PWM_Update_Duty(&runConfig);
  xTaskNotifyGive(update_task_handle);
  while(1)
  {
    vTaskDelay(1);
  }
}

/**
  * @brief process_task function.
  * @param  none
  * @retval none
  */
void process_task_func(void *pvParameters)
{
  uint16_t line_pos = 0;
  char     recv;
  while(1)
  {
    if (xQueueReceive(usart_queue_handle,
                          &recv,
                          pdMS_TO_TICKS(10)) == pdPASS)
    {    
      if(recv == '\r')
      {
      
      }
      else if (recv == '\n' || line_pos >= CLI_LINE_MAX-1)
      {   
        line_buf[line_pos] = '\0';
        line_pos = 0; 
        CLI_ProcessLine(line_buf);
      }
      else 
      {
        line_buf[line_pos++] = recv;     /*   通 址  奂  */
      }	
    }
	  vTaskDelay(10);
  }
}

/**
  * @brief check_task function.
  * @param  none
  * @retval none
  */
void check_task_func(void *pvParameters)
{
  while(1)
  {
	  gpio_bits_set(GPIOC, GPIO_PINS_14);	  
    vTaskDelay(100);      
    gpio_bits_reset(GPIOC, GPIO_PINS_14);	  
    vTaskDelay(100);
  }
}


/**
  * @brief update_task function.
  * @param  none
  * @retval none
  */
void update_task_func(void *pvParameters)
{
  while(1)
  {
	  ulTaskNotifyTake(pdTRUE,portMAX_DELAY);

	  DAC_Update(&runConfig);
	  KEY_Update();
	  Flash_WriteConfig(&runConfig);
//	  Flash_DumpAllSlots();
    vTaskDelay(1);
  }
}


/**
  * @brief key_task function.
  * @param  none
  * @retval none
  */
void key_task_func(void *pvParameters)
{
  /* add user code begin key_task_func 0 */
	#define NUM_KEYS        1
	
	uint8_t i;
	int key = 0;
	static uint8_t lastState = 0;
  uint8_t curState = 0;
  uint8_t count = 0;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	   
	static const uint16_t keyPins[NUM_KEYS] = {
    GPIO_PINS_3,    // Key 1
  };

  /* Infinite loop */
  while(1)
  {
    curState = 0;
	  for (i = 0; i < NUM_KEYS; i++) 
    {
      if (gpio_input_data_bit_read(GPIOC, keyPins[i]) == RESET)
        curState |= (1 << i);
    }

    if (curState == lastState) {
      if (count < 200)
        count++;
    }
    else {
      if (curState != 0) {
        // 触发单击事件
        if ((count > 3) && (count < 50)) {
          switch(curState) {
            case 0x01:
              if (gpio_output_data_bit_read(GPIOB, GPIO_PINS_6) == TRUE)
                gpio_bits_write(GPIOB, GPIO_PINS_6, FALSE);
              else
                gpio_bits_write(GPIOB, GPIO_PINS_6, TRUE);
            break;
          }
        }
      }
      count = 0;
    }
    lastState = curState;
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
/* add user code begin 2 */

static void CLI_Tokenize(char *line)
{
    char *p = line;
    cli_argc = 0;
 
    while (*p && isspace((uint8_t)*p)) p++;  //       头 瞻 ,     值  为0   瞻 

    while (*p && cli_argc < CLI_MAX_ARGS) {
      cli_argv[cli_argc++] = p;      //   录      
      while (*p && !isspace((uint8_t)*p)) p++;   //  也   尾
      if (*p == '\0') break;
      *p++ = '\0';                    //  卸 
      while (*p && isspace((uint8_t)*p)) p++;
    }
}

//   畲�        指      
typedef int (*CLI_Handler_t)(int argc, char *argv[]);

typedef struct {
    const char   *name;        //      址   
    CLI_Handler_t handler;     //          暮   
    const char   *help;        //     说  
} CLI_Command_t;

//     畲�     原  
static int CLI_Help(int, char **);
static int CLI_Cfg (int, char **);
static int CLI_Set (int, char **);
static int CLI_Print (int, char **);
static int CLI_Restore (int, char **);
static int CLI_Clear (int, char **);

//  始           
static const CLI_Command_t cli_cmds[] = {
  {"help", CLI_Help, "帮助"},
  {"cfg",  CLI_Cfg,  ""},
  {"set",  CLI_Set,  ""},
  {"print",CLI_Print,""},
  {"restore", CLI_Restore, "保存PWM duty"},
  {"clear", CLI_Clear, "PWM输出0"},
};

//        
static const int cli_cmd_count = sizeof(cli_cmds) / sizeof(cli_cmds[0]);

void CLI_ProcessLine(char *line)
{
	uint16_t i;
  CLI_Tokenize(line);
  if (cli_argc == 0) return;                  //     

  for (i = 0; i < cli_cmd_count; i++) 
  {           //         
    if (strcmp(cli_argv[0], cli_cmds[i].name) == 0) 
    {
      int ret = cli_cmds[i].handler(cli_argc, cli_argv);
      if (ret != 0) {
        printf("Error: %s\r\n", cli_argv[0]);
      }
      return;
    }
  }
  printf("Unknown cmd: %s\r\n", cli_argv[0]);
}

static int CLI_Help(int argc, char *argv[])
{
	uint16_t i;
  printf("Commands:\r\n");
  for (i = 0; i < cli_cmd_count; i++) 
  {
    printf("  %-6s - %s\r\n",
              cli_cmds[i].name,
              cli_cmds[i].help);
  }
	xTaskNotifyGive(update_task_handle);		// 通知 update_task
  return 0;
}

static int CLI_Cfg(int argc, char *argv[])
{
//    printf("ADC channel:  %u\r\n", runConfig.adc_channel_id);
//    printf("ADC sample:   %u\r\n", runConfig.adc_sampling_time);
  printf("PWM timer:    %u\r\n", runConfig.pwm_timer_id);
  printf("PWM channel:  %u\r\n", runConfig.pwm_channel_id);
  printf("PWM freq:     %u Hz\r\n", runConfig.pwm_frequency);
  printf("PWM duty:     %u %%\r\n", runConfig.pwm_duty);
  printf("DAC :  %u\r\n", runConfig.dac_id);
  printf("DAC value:   %u\r\n", runConfig.dac_otput_value);
	
	xTaskNotifyGive(update_task_handle);		// 通知 update_task
  return 0;
}

static int CLI_Set(int argc, char *argv[])
{
	int duty,tmr,date,out;
	long ch,fre;
//	//set adc ch 1 41
//    if (argc>=5 && strcmp(argv[1],"adc")==0 && strcmp(argv[2],"ch")==0) {
//        ch = strtol(argv[3], NULL, 0);
//        if (ch<1 && ch>16) {
//				printf("Error: channel must be 1~16\r\n");
//				return -1;
//			}
//		
//			cyc = strtol(argv[4], NULL, 10);
//			if(cyc<0){
//				printf("Error: sample time must be positive\r\n");
//				return -1;
//			}
//            runConfig.adc_channel_id = ch ;     
//			runConfig.adc_sampling_time = cyc;
//			printf("OK: ADC channel = %ld, sample time = %d cycles\r\n", ch, cyc);
//			return 0;				
//    }
	
	//set pwm 8 ch 2 fre 10000
	if (argc>=7 
	  && strcmp(argv[1],"pwm")==0
    && strcmp(argv[3],"ch")==0
    && strcmp(argv[5],"fre")==0) 
  {
		tmr = strtol(argv[2], NULL, 10);
    if (tmr < 1 || tmr > 8) 
    {
      printf("Error: timer_id must be 1~8\r\n");
      return -1;
    }
		
		ch = strtol(argv[4], NULL, 10);
    if (ch < 1 || ch > 4) 
    {
      printf("Error: channel must be 1~4\r\n");
      return -1;
    }

		fre = strtol(argv[6], NULL, 10);
    if (fre<0 || fre>240000000) 
    {
      printf("Error:fre 0~240000000\r\n");
      return -1;
    }
			
    runConfig.pwm_timer_id = (uint16_t)tmr;
    runConfig.pwm_channel_id = (uint32_t)ch;
    runConfig.pwm_frequency = (uint32_t)fre;
			
		PWM_Update_Fre(&runConfig);	
		printf("OK: PWM TIM%d CH%ld -> %ldHz\r\n",tmr, ch, fre);
		
    xTaskNotifyGive(update_task_handle);		// 通知 update_task
    return 0;	
  }
	 	
	//set pwm 3 ch 1 duty 50
  if (argc>=7 
	  && strcmp(argv[1],"pwm")==0
    && strcmp(argv[3],"ch")==0
    && strcmp(argv[5],"duty")==0) 
  {
		tmr = strtol(argv[2], NULL, 10);
    if (tmr < 1 || tmr > 8) {
      printf("Error: timer_id must be 1~8\r\n");
      return -1;
    }
		ch = strtol(argv[4], NULL, 10);
    if (ch < 1 || ch > 4) 
    {
        printf("Error: channel must be 1~4\r\n");
        return -1;
    } 
		duty = strtol(argv[6], NULL, 0);
		if (duty < 0 || duty > 100) 
    {
			printf("Error:duty 0~100\r\n");
			return -1;
		}
		runConfig.pwm_timer_id = (uint16_t)tmr;
    runConfig.pwm_channel_id = (uint16_t)ch;
    runConfig.pwm_duty = (uint16_t)duty;
		PWM_Update_Duty(&runConfig);		
    printf("OK: PWM TIM%d CH%ld -> %d%%\r\n",tmr, ch, duty);
		xTaskNotifyGive(update_task_handle);		// 通知 update_task
		return 0; 
  }

	//set dac 1 500
  if (argc>=4 
	  && strcmp(argv[1],"dac")==0)
  {
    out = strtol(argv[2], NULL, 10);
    date = strtol(argv[3], NULL, 10);
    if (out < 0 || out > 3) 
    {
      printf("Error:DAC id will be only 1 or 2\r\n");
      return -1;
    }
		if (date < 0 || date > 4095) 
    {
			printf("Error:DAC Value 0~4095\r\n");
			return -1;
		}
		
		runConfig.dac_id = out;
		runConfig.dac_otput_value = date;
		printf("OK: DAC%d -> %d\r\n",out, date);
		
		xTaskNotifyGive(update_task_handle);		// 通知 update_task
		return 0;
	}
	return 0;
}

//restore pwm 8        (默  100%)
static int CLI_Restore(int argc, char *argv[])
{      
  uint16_t i;
  if (argc >= 3 &&
    strcmp(argv[1], "pwm") == 0)
  {
    tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_4, lastDuty[7]*48);     //  Duty转  为CCR
    tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_3, lastDuty[6]*48);
    tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_2, lastDuty[5]*48);
    tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_1, lastDuty[4]*48);
    for ( i = 4; i <= 7; i++)
      printf("OK: restore successfully\r\n");
    xTaskNotifyGive(update_task_handle);       // 通知 update_task
  }
  return 0;
}

//clear
static int CLI_Clear(int argc, char *argv[])
{
  if (argc >= 1 &&
    strcmp(argv[0], "clear") == 0)
  {
    tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_4, 0);    
    tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_3, 0);
    tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_2, 0);
    tmr_channel_value_set(TMR8, TMR_SELECT_CHANNEL_1, 0);
    printf("OK: clear successfully\r\n");
    xTaskNotifyGive(update_task_handle);       // 通知 update_task
  }
	return 0;
}


static int CLI_Print(int argc, char *argv[])
{
	int ch,adc;
	 //print adc 1 ch 1
	if (argc>=5 
	  && strcmp(argv[1],"adc")==0
    && strcmp(argv[3],"ch")==0)
	{
		adc = strtol(argv[2], NULL, 10); 
		ch = strtol(argv[4], NULL, 10);
		switch(ch)
    {
			case 1  :   printf("CH1_Value = %.2f  \r\n",Read_Temperature_C(CH1_Value));      break;
			case 2  :   printf("CH2_Value = %.2f  \r\n",Read_Temperature_C(CH2_Value));      break;
			case 3  :   printf("CH3_Value = %.2f  \r\n",Read_Temperature_C(CH3_Value));      break;
			case 4  :   printf("CH4_Value = %.2f  \r\n",Read_Temperature_C(CH4_Value));      break;
			case 5  :   printf("CH5_Value = %.2f  \r\n",Read_Temperature_C(CH5_Value));      break;
			case 6  :   printf("CH6_Value = %.2f  \r\n",Read_Temperature_C(CH6_Value));      break;
			case 7  :   printf("CH7_Value = %.2f  \r\n",Read_Temperature_C(CH7_Value));      break;
			case 8  :   printf("CH8_Value = %.2f  \r\n",Read_Temperature_C(CH8_Value));      break;
			case 9  :   printf("CH9_Value = %.2f  \r\n",Read_Temperature_C(CH9_Value));      break;
			case 10 :   printf("CH10_Value = %.2f  \r\n",Read_Temperature_C(CH10_Value));    break;
			case 11 :   printf("CH11_Value = %.2f  \r\n",Read_Temperature_C(CH11_Value));    break;
			case 12 :   printf("CH12_Value = %.2f  \r\n",Read_Temperature_C(CH12_Value));    break;
			default :   printf("Channel not exist");
		}		
	}
	return 0;
}

static float ADC_RawToVoltage_mV(uint16_t raw)
{
  return raw * VREF_MV / ADC_MAX_VALUE;
}

static float NTC_Resistance_Ohm(float vout_mV)
{
	float Vout,Vcc;

  Vout = vout_mV / 1000.0f;       //mV    为V
  Vcc  = VREF_MV / 1000.0f;
  return R_REF_OHM * Vout / (Vcc - Vout);
}

static float NTC_Temperature_C(float r_ntc)
{
	float T_K,inv_T;
  // 1/T = 1/T0 + (1/  )  ln(R/R0)
  inv_T = (1.0f / T0_K) + (1.0f / BETA_K) * logf(r_ntc / R0_OHM);
  T_K   = 1.0f / inv_T;
  return T_K - 273.15f;
}

float Read_Temperature_C(uint16_t CH_Value)
{
  uint16_t raw     = CH_Value / 5;                  	// 1.     
  float    vout_mV = ADC_RawToVoltage_mV(raw);       // 2.   压
  float    r_ntc   = NTC_Resistance_Ohm(vout_mV);    // 3.   值
  float    tempC   = NTC_Temperature_C(r_ntc);       // 4.  露 
  return tempC;
}
/* add user code end 2 */

