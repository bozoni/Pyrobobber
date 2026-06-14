#include "LoRa_lib.h"

SPI_HandleTypeDef Spi1Handle;

/* Private function prototypes -----------------------------------------------*/
static void LORALIB_GPIO_Init(void);
static void LORALIB_SPI_Init(void);
static uint8_t LORALIB_SPI_Data(uint8_t d);
static void LORALIB_LORA_Select(void);
static void LORALIB_LORA_Deselect(void);
static void LORALIB_LORA_Reset(void);
static void LORALIB_LORA_WriteReg(uint8_t addr, uint8_t data);
static uint8_t LORALIB_LORA_ReadReg(uint8_t addr); 
static void LORALIB_LORA_WriteFifo(uint8_t *data, uint8_t len);
static void LORALIB_PERF_Init(void);

/* Exported function prototypes -----------------------------------------------*/
void LORALIB_LORA_Init();
void LORALIB_LORA_SendPacket(uint8_t *data, uint8_t len);


/**
  * @brief  GPIO initialization
  */
static void LORALIB_GPIO_Init(void)
{
  // ????????????? ?????????, ????????, ?????????? ?? PA4
	// 2. ????????? ???? ?????? ??? ?????
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitTypeDef gpio_init = {0};
	gpio_init.Pin = GPIO_PIN_0;          // ???????????, ?????? ?? PA0
	gpio_init.Mode = GPIO_MODE_INPUT;    // ????? ?????
	gpio_init.Pull = GPIO_PULLUP;        // ???????? ?????????? ???????? ? ???????
	gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &gpio_init);

	// 3. ????????? ???? ?????????? ??? ?????? (??? ?????????)
	gpio_init.Pin = GPIO_PIN_5 | GPIO_PIN_6;
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio_init);	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
}


/**
  * @brief  SPI initialization
  */
static void LORALIB_SPI_Init()
{
  /* De-initialize SPI configuration */
  Spi1Handle.Instance                       = SPI1;                       /* SPI1 */
  //Spi1Handle.Init.BaudRatePrescaler			  = SPI_BAUDRATEPRESCALER_2;    //SPI_BAUDRATEPRESCALER_;  /* 256 frequency division */
  Spi1Handle.Init.Direction   		      		= SPI_DIRECTION_2LINES;       /* full duplex */
	Spi1Handle.Init.CLKPolarity 							= SPI_POLARITY_HIGH;          /* High clock polarity */
	Spi1Handle.Init.CLKPhase    							= SPI_PHASE_2EDGE;            /* Data sampling on second clock edge */
  Spi1Handle.Init.DataSize    		  	 	    = SPI_DATASIZE_8BIT;          /* SPI data length is 8 bits */
  Spi1Handle.Init.FirstBit      		     		= SPI_FIRSTBIT_MSB;           /* Send MSB first */
  Spi1Handle.Init.NSS              				  = SPI_NSS_SOFT;      				  /* NSS software mode (hardware mode) */
  Spi1Handle.Init.Mode   								 	  = SPI_MODE_MASTER;         		/* Configure as master */
	
  if (HAL_SPI_DeInit(&Spi1Handle) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* SPI initialization */
  if (HAL_SPI_Init(&Spi1Handle) != HAL_OK)
  {
    APP_ErrorHandler();
  }  
	  __HAL_SPI_ENABLE(&Spi1Handle);
}
/**
  * @brief  Send/receive data via SPI
  */
static uint8_t LORALIB_SPI_Data(uint8_t d)
{
	uint8_t rec;
	// ????, ???? SPI ????? ????????
	while (!__HAL_SPI_GET_FLAG(&Spi1Handle, SPI_FLAG_TXE));
	Spi1Handle.Instance->DR = d;
	while (!__HAL_SPI_GET_FLAG(&Spi1Handle, SPI_FLAG_RXNE));
	rec = Spi1Handle.Instance->DR; // ??????, ????? ????????
	return rec;
}

static void LORALIB_PERF_Init(void)
{
	LORALIB_SPI_Init();
	LORALIB_GPIO_Init();
}

static void LORALIB_LORA_Select(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

static void LORALIB_LORA_Deselect(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

static void LORALIB_LORA_Reset(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_Delay(10);
}

static void LORALIB_LORA_WriteReg(uint8_t addr, uint8_t data)
{
	LORALIB_LORA_Select();
	LORALIB_SPI_Data(addr|0x80);
	LORALIB_SPI_Data(data);
	LORALIB_LORA_Deselect();
}

static uint8_t LORALIB_LORA_ReadReg(uint8_t addr)
{
	uint8_t val;
	LORALIB_LORA_Select();
	LORALIB_SPI_Data(addr&0x7F);
	val = LORALIB_SPI_Data(0x00);
	LORALIB_LORA_Deselect();
	return val;
}

static void LORALIB_LORA_WriteFifo(uint8_t *data, uint8_t len)
{
	LORALIB_LORA_Select();
	LORALIB_SPI_Data(0x80);
	for(uint8_t i = 0; i < len; i++)
	{
		LORALIB_SPI_Data(data[i]);
	}
	LORALIB_LORA_Deselect();
}

void LORALIB_LORA_Init()
{
	LORALIB_PERF_Init();
	LORALIB_LORA_Reset();

	uint8_t version = LORALIB_LORA_ReadReg(0x42);
	if (version != 0x12)
	{
		APP_ErrorHandler();
	}
	LORALIB_LORA_WriteReg(0x01, 0x00);

	LORALIB_LORA_WriteReg(0x01, 0x80);
	HAL_Delay(10);
	LORALIB_LORA_WriteReg(0x39, 0x67);
	uint64_t freq = 434000000; 
	uint64_t frf = (freq << 19) / 32000000; 
	LORALIB_LORA_WriteReg(0x06, (frf >> 16) & 0xFF);
	LORALIB_LORA_WriteReg(0x07, (frf >> 8) & 0xFF);
	LORALIB_LORA_WriteReg(0x08, frf & 0xFF);         

	LORALIB_LORA_WriteReg(0x09, 0xFF);
	LORALIB_LORA_WriteReg(0x4D, 0x87);

	LORALIB_LORA_WriteReg(0x1E, 0x72);
	LORALIB_LORA_WriteReg(0x1F, 0xC4);
	LORALIB_LORA_WriteReg(0x26, 0x04);

	LORALIB_LORA_WriteReg(0x01, 0x81); // RegOpMode -> LoRa + Standby
	HAL_Delay(100);	
}

void LORALIB_LORA_SendPacket(uint8_t *data, uint8_t len)
{
	LORALIB_LORA_WriteReg(0x01, 0x81);
	HAL_Delay(10);

	LORALIB_LORA_WriteReg(0x0E, 0x00);
	LORALIB_LORA_WriteReg(0x0D, 0x00);

	// ?????????? ?????? ? FIFO
	LORALIB_LORA_WriteFifo(data, len);

	// ????????????? ????? ??????
	LORALIB_LORA_WriteReg(0x22, len);

	LORALIB_LORA_WriteReg(0x01, 0x83);

	while ((LORALIB_LORA_ReadReg(0x12) & 0x08) == 0) {
			HAL_Delay(1);
	}

	// ?????????? ????? ??????????
	LORALIB_LORA_WriteReg(0x12, 0xFF); // RegIrqFlags -> ????? ???? ??????

	// ?????????? ? ????? Standby
	LORALIB_LORA_WriteReg(0x01, 0x81); // RegOpMode -> LoRa + Standby	
}

void LORALIB_LORA_ReadFifo(uint8_t *buffer, uint8_t len) {
    LORALIB_LORA_Select();
	
    LORALIB_SPI_Data(0x00);

    for (uint8_t i = 0; i < len; i++) {
        buffer[i] = LORALIB_SPI_Data(0x00);
    }
		
    LORALIB_LORA_Deselect();
}

uint8_t LORALIB_LORA_ReceivePacket(uint8_t *buffer) {
    LORALIB_LORA_WriteReg(0x01, 0x85);
    
    // ???????? RxDone
    while ((LORALIB_LORA_ReadReg(0x12) & 0x40) == 0);
    
    // ???????? CRC
    if (LORALIB_LORA_ReadReg(0x12) & 0x20) {
        // CRC ?????? – ???????? ????? ? ??????? 0
        LORALIB_LORA_WriteReg(0x12, 0xFF);
        return 0;
    }
    
    uint8_t len = LORALIB_LORA_ReadReg(0x13);
    uint8_t fifo_addr = LORALIB_LORA_ReadReg(0x10);
    LORALIB_LORA_WriteReg(0x0D, fifo_addr);
    LORALIB_LORA_ReadFifo(buffer, len);
    LORALIB_LORA_WriteReg(0x12, 0xFF);
    return len;
}

void LORALIB_LORA_ChangePassword(uint8_t password){
	LORALIB_LORA_Select();
	LORALIB_LORA_WriteReg(0x39, password);
	LORALIB_LORA_Deselect();
}