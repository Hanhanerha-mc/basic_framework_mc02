#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "debug_cmd.h"

// extern Gimbal_Ctrl_Cmd_s gimbal_cmd_send; // 来自robot_cmd.c的云台控制数据

void Debug_Init(UART_HandleTypeDef *_handle);
void DebugTask();

#endif // __DEBUG_H__