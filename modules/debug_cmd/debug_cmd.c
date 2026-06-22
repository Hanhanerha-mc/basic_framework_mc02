#include <string.h>
#include "debug_cmd.h"
#include "bsp_log.h"

#define HEADER 0xAA

static USARTInstance *usart_instance;
static Debug_Cmd_s *recv_data;
static uint8_t debug_cmd[sizeof(Debug_Cmd_s) - 2]; // 不包括帧头和checksum,用于存储解析后的数据,减少memcpy的开销

/**
 * @brief 解析调试命令
 * 
 * 从usart_instance->recv_buff中解析调试命令,并存储到debug_cmd中,供外部调用
 * 协议格式: [HEADER][CMD1][CMD2][CHECKSUM] 
 * 其中CMD1和CMD2为调试命令,校验和为CMD1和CMD2的和
*/
static void DebugCmdDecode()
{
    recv_data = (Debug_Cmd_s *)usart_instance->recv_buff;
    uint8_t checksum = 0;

    if (recv_data->header != HEADER) // 帧头错误
    {
        LOGERROR("[debug_cmd] protocol header error: %02X", recv_data->header);
        return;
    }
    
    checksum = recv_data->cmd[0] + recv_data->cmd[1]; // 简单的校验和,不包括帧头
    if (checksum != recv_data->checksum) // 校验和错误
    {
        LOGERROR("[debug_cmd] protocol checksum error: need %02X but get %02X", checksum, recv_data->checksum);
        return;
    }

    // 解析数据
    memcpy(debug_cmd, recv_data->cmd, sizeof(Debug_Cmd_s) - 2); // 不包括帧头和checksum,用于存储解析后的数据,减少memcpy的开销
}

uint8_t *DebugCmdInit(UART_HandleTypeDef *_handle)
{
    USART_Init_Config_s conf;
    conf.recv_buff_size = sizeof(Debug_Cmd_s);
    conf.usart_handle = _handle;
    conf.module_callback = DebugCmdDecode;

    usart_instance = USARTRegister(&conf);
    // USARTServiceInit(usart_instance); // 启动串口服务,开始接收数据
    return debug_cmd;
}

uint8_t *DebugCmdGet()
{
    return debug_cmd;
}
