#include "debug.h"
#include "bsp_log.h"
#include "bsp_dwt.h"
#include "message_center.h"
#include "user_lib.h"
#include <stdint.h>

uint8_t *debug_cmd; // 调试命令指针,在DebugCmdInit中初始化
// Gimbal_Ctrl_Cmd_s *gimbal_cmd_send_ptr; // 云台控制数据指针,在GetGimbalCmdSendPtr中初始化

void Debug_Init(UART_HandleTypeDef *_handle)
{
    debug_cmd = DebugCmdGet();

    // Publisher_t *gimbal_cmd_pub = PubRegister("gimbal_cmd", 1);         // 获取云台控制消息发布者,用于在DebugTask中发送调试命令
}

/**
 * @brief 小范围yaw轴正弦扫描,使用DWT高精度时间轴
 *
 * @param center_position 中心位置
 * @param amplitude 振幅
 * @param period 周期,单位:秒
 * @return float 计算出的目标yaw角度
 */
static float SmallYawScan(float center_position, float amplitude, float period)
{
    static float start_time = 0;
    if (start_time == 0)
        start_time = DWT_GetTimeline_s();
    float time = DWT_GetTimeline_s() - start_time;
    return center_position + amplitude * msin(2 * PI * time / period);
}

void DebugTask()
{
    if (debug_cmd[1] == 0x01)       // 如果接收到调试命令0x01,执行小范围扫描
    {
        
    }
}
