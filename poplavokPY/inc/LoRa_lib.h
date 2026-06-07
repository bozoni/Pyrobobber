#include "py32f0xx_hal.h"
#include "main.h"

/* Exported function prototypes -----------------------------------------------*/
void LORALIB_LORA_Init();
void LORALIB_LORA_SendPacket(uint8_t *data, uint8_t len);