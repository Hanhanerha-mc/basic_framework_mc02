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

/**
 * @brief 初始化调试命令模块
 * 
 * @param _handle 串口句柄
 * @return uint8_t* 指向调试命令的指针
 */
uint8_t *DebugCmdInit(UART_HandleTypeDef *_handle);

/**
 * @brief 获取调试命令
 * 
 * @return uint8_t* 指向调试命令的指针
 */
uint8_t *DebugCmdGet();

#endif // __DEBUG_CMD_H__