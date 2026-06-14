#include "py32f0xx_hal.h"
#include "main.h"

/* Exported function prototypes -----------------------------------------------*/
void LORALIB_LORA_Init();
void LORALIB_LORA_SendPacket(uint8_t *data, uint8_t len);
uint8_t LORALIB_LORA_ReceivePacket(uint8_t *buffer);
void LORALIB_LORA_ChangePassword(uint8_t password);