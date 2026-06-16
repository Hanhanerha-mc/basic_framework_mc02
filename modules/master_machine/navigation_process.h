#ifndef NAVIGATION_PROCESS_H
#define NAVIGATION_PROCESS_H

#include "bsp_usart.h"

#pragma pack(1)     // 压缩结构体
typedef enum
{
    VELOCITY_MODE = 1,
    FOLLOW_GIMBAL_MODE = 2,
    SWING_MODE = 3,
    SPIN_MODE = 4,
} control_mode_e;


typedef struct
{
    float vx;
    float vy;
    float wz;
    control_mode_e mode;
} Navigation_Recv_s;

typedef struct
{
    float health;
    float game_mode;
} Navigation_Send_s;
#pragma pack()

/**
 * @brief 导航进程初始化函数
 *
 * @return Navigation_Recv_s* 返回接收数据的指针,供其他模块获取
 */
Navigation_Recv_s* NavigationProcessInit(UART_HandleTypeDef *_handle);

#endif // !NAVIGATION_PROCESS_H