#include <string.h>
#include "navigation_process.h"
#include "bsp_log.h"
#include "daemon.h"
#include "crc8.h"

#define HEADER 0xFF
#define FOOTER 0x0D

static USARTInstance *navigation_usart_instance;
static DaemonInstance *navigation_daemon_instance;
static Navigation_Recv_s recv_data; // 接收数据
static Navigation_Send_s send_data; // 发送数据

#define RECV_PACKET_SIZE sizeof(Navigation_Recv_s) + 3
#define SEND_PACKET_SIZE sizeof(Navigation_Send_s) + 3
#define RECV_DATA_SIZE   sizeof(Navigation_Recv_s)
#define SEND_DATA_SIZE   sizeof(Navigation_Send_s)

static void DecodeNavigation()
{
    DaemonReload(navigation_daemon_instance); // 喂狗

    uint8_t *recv_buff = navigation_usart_instance->recv_buff;
    if (recv_buff[0] != HEADER || recv_buff[RECV_PACKET_SIZE - 1] != FOOTER) // 帧头或帧尾错误
    {
        LOGERROR("[navigation] protocol header or footer error: %02X %02X", recv_buff[0], recv_buff[RECV_PACKET_SIZE - 1]);
        return;
    }
    if (crc_8(recv_buff + 1, RECV_PACKET_SIZE - 3) != recv_buff[RECV_PACKET_SIZE - 2]) // 校验和错误
    {
        LOGERROR("[navigation] protocol checksum error: %02X", recv_buff[RECV_PACKET_SIZE - 2]);
        return;
    }
    // 数据解析
    memcpy(&recv_data, recv_buff + 1, RECV_DATA_SIZE);
}

static void NavigationOfflineCallback(void *id)
{
    USARTServiceInit(navigation_usart_instance);
    LOGWARNING("[navigation] navigation offline, restart communication.");
}

static void NavigationSend()
{
    // buff和txlen必须为static,才能保证在函数退出后不被释放,使得DMA正确完成发送,否则会出现析构后的陷阱
    static uint8_t send_buff[SEND_PACKET_SIZE];

    send_buff[0] = HEADER;
    memcpy(send_buff + 1, &send_data, SEND_DATA_SIZE);   
    send_buff[SEND_PACKET_SIZE - 2] = crc_8(send_buff + 1, SEND_PACKET_SIZE - 3);
    send_buff[SEND_PACKET_SIZE - 1] = FOOTER;

    USARTSend(navigation_usart_instance, send_buff, SEND_PACKET_SIZE, USART_TRANSFER_DMA);
}

Navigation_Recv_s* NavigationProcessInit(UART_HandleTypeDef *_handle)
{
    // 先注册daemon,否则USARTRegister启动DMA接收后,若RX引脚有噪声,
    // DecodeNavigation回调会访问尚未初始化的navigation_daemon_instance导致空指针崩溃
    Daemon_Init_Config_s daemon_conf = {
        .reload_count = 10,
        .callback = NavigationOfflineCallback, // 离线时调用的回调函数,会重启串口接收
        .owner_id = NULL,   // 只有一个导航通信,不需要owner_id
    };
    navigation_daemon_instance = DaemonRegister(&daemon_conf);

    USART_Init_Config_s conf = {
        .recv_buff_size = RECV_PACKET_SIZE,
        .usart_handle = _handle,
        .module_callback = DecodeNavigation,
    };
    navigation_usart_instance = USARTRegister(&conf);

    return &recv_data;
}