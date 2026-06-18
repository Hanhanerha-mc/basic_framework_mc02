#ifndef __DEBUG_CMD_H__
#define __DEBUG_CMD_H__

#include "bsp_usart.h"

#pragma pack(1)
typedef struct
{
    uint8_t header;
    uint8_t cmd[2];
    uint8_t checksum;
} Debug_Cmd_s;
#pragma pack()

uint8_t *DebugCmdInit(UART_HandleTypeDef *_handle);

#endif // __DEBUG_CMD_H__