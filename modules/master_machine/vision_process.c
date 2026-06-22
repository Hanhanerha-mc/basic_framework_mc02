/**
 * @file vision_process.c
 * @author neozng
 * @brief  module for recv&send vision data
 * @version beta
 * @date 2022-11-03
 * @todo 增加对串口调试助手协议的支持,包括vofa和serial debug
 * @copyright Copyright (c) 2022
 *
 */
#include "vision_process.h"
#include "seasky_protocol.h"
#include "daemon.h"
#include "bsp_log.h"
#include "robot_def.h"
#include "crc16.h"

#define HEADER0 'S'
#define HEADER1 'P'
#define RECV_PACKET_SIZE sizeof(Vision_Recv_s)
#define SEND_PACKET_SIZE sizeof(Vision_Send_s)

static USARTInstance *vision_usart_instance;

static Vision_Recv_s recv_data;
static Vision_Send_s send_data;
static DaemonInstance *vision_daemon_instance;

void VisionSetFlag(Work_Mode_e work_mode, Bullet_Speed_e bullet_speed, uint16_t bullet_count)
{
    // send_data.enemy_color = enemy_color;
    send_data.work_mode = work_mode;
    send_data.bullet_speed = bullet_speed;
    send_data.bullet_count = bullet_count;          // *
}

void VisionSetAltitude(float yaw, float pitch, float yaw_vel, float pitch_vel)
{
    send_data.current_yaw = yaw;
    send_data.current_pitch = pitch;
    send_data.current_yaw_vel = yaw_vel; 
    send_data.current_pitch_vel = pitch_vel; 
}

void VisionSetQuaternion(float w, float x, float y, float z)
{
    send_data.q[0] = w;
    send_data.q[1] = x;
    send_data.q[2] = y;
    send_data.q[3] = z;
}

/**
 * @brief 离线回调函数,将在daemon.c中被daemon task调用
 * @attention 由于HAL库的设计问题,串口开启DMA接收之后同时发送有概率出现__HAL_LOCK()导致的死锁,使得无法
 *            进入接收中断.通过daemon判断数据更新,重新调用服务启动函数以解决此问题.
 *
 * @param id vision_usart_instance的地址,此处没用.
 */
static void VisionOfflineCallback(void *id)
{
#ifdef VISION_USE_UART
    USARTServiceInit(vision_usart_instance);
#endif // !VISION_USE_UART
    LOGWARNING("[vision] vision offline, restart communication.");
}

#ifdef VISION_USE_UART

#include "bsp_usart.h"



/**
 * @brief 接收解包回调函数,将在bsp_usart.c中被usart rx callback调用
 * @todo  1.提高可读性,将get_protocol_info的第四个参数增加一个float类型buffer
 *        2.添加标志位解码
 */
static void DecodeVision()
{
    // uint16_t flag_register;  // TODO: 解析标志位
    DaemonReload(vision_daemon_instance); // 喂狗

    uint8_t *recv_buff = vision_usart_instance->recv_buff;
    static uint16_t crc = 0, recv_crc = 0;

    if (recv_buff[0] != HEADER0 || recv_buff[1] != HEADER1) // 帧头错误
    {
        LOGERROR("[vision] protocol header error: %02X", recv_buff[0]);
        return;
    }

    crc = crc_16(recv_buff + 2, RECV_PACKET_SIZE - 4); // 计算CRC16,不包括帧头和CRC本身
    recv_crc = (uint16_t)recv_buff[RECV_PACKET_SIZE - 2] << 8 | recv_buff[RECV_PACKET_SIZE - 1]; // 从接收数据中提取CRC16,不包括帧头
    if (crc != recv_crc) // 校验和错误
    {
        LOGERROR("[vision] protocol checksum error: need %04X but get %04X", crc, recv_crc);
        return;
    }
    // 解析数据
    memcpy(&recv_data, recv_buff, RECV_PACKET_SIZE); // 包括帧头和CRC,但不影响数据解析,且可以减少一次memcpy

    // TODO: code to resolve flag_register;
}

Vision_Recv_s *VisionInit(UART_HandleTypeDef *_handle)
{
    USART_Init_Config_s conf;
    conf.module_callback = DecodeVision;
    conf.recv_buff_size = RECV_PACKET_SIZE;
    conf.usart_handle = _handle;
    vision_usart_instance = USARTRegister(&conf);

    // 为master process注册daemon,用于判断视觉通信是否离线
    Daemon_Init_Config_s daemon_conf = {
        .callback = VisionOfflineCallback, // 离线时调用的回调函数,会重启串口接收
        .owner_id = vision_usart_instance,
        .reload_count = 10,
    };
    vision_daemon_instance = DaemonRegister(&daemon_conf);

    return &recv_data;
}

/**
 * @brief 发送函数
 *
 * @param send 待发送数据
 *
 */
void VisionSend()
{
    // buff和txlen必须为static,才能保证在函数退出后不被释放,使得DMA正确完成发送
    // 析构后的陷阱需要特别注意!
    // static uint16_t flag_register;
    static uint8_t send_buff[SEND_PACKET_SIZE];
    static uint16_t crc = 0;

    memcpy(&send_buff, &send_data, SEND_PACKET_SIZE - 2); // 数据部分,不包括帧头和CRC    q
    send_buff[0] = HEADER0;
    send_buff[1] = HEADER1;
    send_data.work_mode = 1; // TODO: code to set flag_register

    crc = crc_16(send_buff, SEND_PACKET_SIZE - 2); // 计算CRC16,不包括帧头和CRC本身
    send_buff[SEND_PACKET_SIZE - 2] = (uint8_t)(crc >> 8); // CRC高8位
    send_buff[SEND_PACKET_SIZE - 1] = (uint8_t)(crc & 0x00FF); // CRC低8位
    
    USARTSend(vision_usart_instance, send_buff, SEND_PACKET_SIZE, USART_TRANSFER_DMA);
}

#endif // VISION_USE_UART

#ifdef VISION_USE_VCP

#include "bsp_usb.h"
static uint8_t *vis_recv_buff;

static void DecodeVision(uint16_t recv_len)
{
    uint16_t flag_register;
    get_protocol_info(vis_recv_buff, &flag_register, (uint8_t *)&recv_data.pitch);
    // TODO: code to resolve flag_register;
}

/* 视觉通信初始化 */
Vision_Recv_s *VisionInit(UART_HandleTypeDef *_handle)
{
    UNUSED(_handle); // 仅为了消除警告
    USB_Init_Config_s conf = {.rx_cbk = DecodeVision};
    vis_recv_buff = USBInit(conf);

    // 为master process注册daemon,用于判断视觉通信是否离线
    Daemon_Init_Config_s daemon_conf = {
        .callback = VisionOfflineCallback, // 离线时调用的回调函数,会重启串口接收
        .owner_id = NULL,
        .reload_count = 5, // 50ms
    };
    vision_daemon_instance = DaemonRegister(&daemon_conf);

    return &recv_data;
}

void VisionSend()
{
    static uint16_t flag_register;
    static uint8_t send_buff[VISION_SEND_SIZE];
    static uint16_t tx_len;
    // TODO: code to set flag_register
    flag_register = 30 << 8 | 0b00000001;
    // 将数据转化为seasky协议的数据包
    get_protocol_send_data(0x02, flag_register, &send_data.yaw, 3, send_buff, &tx_len);
    USBTransmit(send_buff, tx_len);
}

#endif // VISION_USE_VCP
