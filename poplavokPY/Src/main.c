/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
extern char cnt;
extern _Bool EXTI_Flag;
uint8_t mes[255] = {0xAB, 0xBA};
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
void APP_ErrorHandler(void);
static void APP_ConfigureExti(void); 
static void APP_GPIO_Init();
static void APP_Robbober_cicl();
void APP_GPIO_Blink();

#define id 3
//#define scnd  //ACHTUNG!!! Comment if id != 2


#ifdef scnd 
static void APP_Robbober_cicl()       											//if id == 2 Send enc value every 500 ms
{
	mes[0] = cnt;
	LORALIB_LORA_SendPacket(mes, 1);
	APP_GPIO_Blink();
	HAL_Delay(1000);
}

#else
static void APP_Robbober_cicl()															//else:
{
	while(!LORALIB_LORA_ReceivePacket(mes)) HAL_Delay(100);		//wait for message from previos modules
	LORALIB_LORA_ChangePassword(id);													//change synchword for transmission
	mes[id-2] = cnt;																					//
	LORALIB_LORA_SendPacket(mes, id-1);												//send recieved values and self cnt value
	LORALIB_LORA_ChangePassword(id - 1);											//change synchword for reseption
}
#endif

/*
transmit synchword {id} 
recieve synchword {id - 1}
*/

/**
  * @brief  Main program.
  * @retval int
  */
int main(void)
{

	
  HAL_Init();
  APP_SystemClockConfig();
	LORALIB_LORA_Init();
	APP_GPIO_Init();
	for(int i = 0; i < 5; i++)
	{
		APP_GPIO_Blink();
		HAL_Delay(500);
	}
	LORALIB_LORA_ChangePassword(id);
	LORALIB_LORA_SendPacket(mes, 1);
	#ifdef scnd
		LORALIB_LORA_ChangePassword(id);
	#else
		LORALIB_LORA_ChangePassword(id-1);
	#endif
	
  while (1)
  {
		APP_Robbober_cicl();
  }
}

/**
  * @brief  System clock configuration
  */
static void APP_SystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Oscillator configuration */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;
  
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* Clock source configuration */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSISYS;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  Configure EXTI
  * @param  None
  * @retval None
  */
static void APP_ConfigureExti(void)
{
  /* Configuration pins */
  GPIO_InitTypeDef  GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();                  /* Enable GPIOA clock */

  GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;  /* GPIO mode is a falling edge interrupt */
  GPIO_InitStruct.Pull  = GPIO_PULLUP;           /* pull up */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  /* The speed is high */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Enable EXTI interrupt */
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
  /* Configure interrupt priority */
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
}

static void APP_GPIO_Init()
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitTypeDef gpio_init = {0};
	gpio_init.Pin = GPIO_PIN_0;          // ???????????, ?????? ?? PA0
	gpio_init.Mode = GPIO_MODE_INPUT;    // ????? ?????
	gpio_init.Pull = GPIO_PULLUP;        // ???????? ?????????? ???????? ? ???????
	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &gpio_init);
	gpio_init.Pin = GPIO_PIN_3;          // ???????????, ?????? ?? PA0
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   // ????? ?????
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &gpio_init);
	
}

void APP_GPIO_Blink()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
}

void APP_ErrorHandler(void)
{
	while(1);
}