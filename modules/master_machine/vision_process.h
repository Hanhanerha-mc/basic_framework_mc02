#ifndef MASTER_PROCESS_H
#define MASTER_PROCESS_H

#include "bsp_usart.h"
#include "seasky_protocol.h"
#include "general_def.h"

#pragma pack(1)
typedef enum
{
	NO_FIRE = 0,
	AUTO_FIRE = 1,
	AUTO_AIM = 2
} Fire_Mode_e;

typedef enum
{
	NO_TARGET = 0,
	TARGET_CONVERGING = 1,
	READY_TO_FIRE = 2
} Target_State_e;

typedef enum
{
	NO_TARGET_NUM = 0,
	HERO1 = 1,
	ENGINEER2 = 2,
	INFANTRY3 = 3,
	INFANTRY4 = 4,
	INFANTRY5 = 5,
	OUTPOST = 6,
	SENTRY = 7,
	BASE = 8
} Target_Type_e;

typedef struct
{
	uint8_t head[2];
	Target_State_e target_state;  // 0: 不控制(无目标), 1: 控制云台但不开火(跟踪)，2: 控制云台且开火(开火)
	float target_yaw;
	float target_yaw_vel;
	float target_yaw_acc;
	float target_pitch;
	float target_pitch_vel;
	float target_pitch_acc;
	uint16_t crc16;

} Vision_Recv_s;

typedef enum
{
	COLOR_NONE = 0,
	COLOR_BLUE = 1,
	COLOR_RED = 2,
} Enemy_Color_e;

typedef enum
{
	VISION_MODE_AIM = 0,
	VISION_MODE_SMALL_BUFF = 1,
	VISION_MODE_BIG_BUFF = 2
} Work_Mode_e;

typedef enum
{
	BULLET_SPEED_NONE = 0,
	BIG_AMU_10 = 10,
	SMALL_AMU_15 = 15,
	BIG_AMU_16 = 16,
	SMALL_AMU_18 = 18,
	SMALL_AMU_30 = 30,
} Bullet_Speed_e;

typedef struct
{
	uint8_t head[2];
	Work_Mode_e work_mode;		// 0: 空闲, 1: 自瞄, 2: 小符, 3: 大符
	float q[4];					// wxyz顺序
	float current_yaw;
	float current_yaw_vel;
	float current_pitch;
	float current_pitch_vel;
	float bullet_speed;		// 当前弹速,用于统计
	uint16_t bullet_count;		// 子弹累计发送次数
	uint16_t crc16;
} Vision_Send_s;
#pragma pack()

/**
 * @brief 调用此函数初始化和视觉的串口通信
 *
 * @param handle 用于和视觉通信的串口handle(C板上一般为USART1,丝印为USART2,4pin)
 */
Vision_Recv_s *VisionInit(UART_HandleTypeDef *_handle);

/**
 * @brief 发送视觉数据
 *
 */
void VisionSend();

/**
 * @brief 设置视觉发送标志位
 *
 * @param work_mode
 * @param bullet_speed
 * @param bullet_count
 */
void VisionSetFlag(Work_Mode_e work_mode, Bullet_Speed_e bullet_speed, uint16_t bullet_count);

/**
 * @brief 设置发送数据的姿态部分
 *
 * @param yaw
 * @param pitch
 * @param yaw_vel
 * @param pitch_vel
 */
void VisionSetAltitude(float yaw, float pitch, float yaw_vel, float pitch_vel);

/**
 * @brief 设置发送数据的四元数部分
 *
 * @param w
 * @param x
 * @param y
 * @param z
 */
void VisionSetQuaternion(float w, float x, float y, float z);

#endif // !MASTER_PROCESS_H