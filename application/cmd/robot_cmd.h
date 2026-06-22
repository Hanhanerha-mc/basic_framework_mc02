#ifndef ROBOT_CMD_H
#define ROBOT_CMD_H


/**
 * @brief 机器人核心控制任务初始化,会被RobotInit()调用
 * 
 */
void RobotCMDInit();

/**
 * @brief 机器人核心控制任务,200Hz频率运行(必须高于视觉发送频率)
 * 
 */
void RobotCMDTask();

/**
 * @brief 获取云台控制数据指针,用于在DebugTask中发送调试命令
 * 
 * @return Gimbal_Ctrl_Cmd_s* 云台控制数据指针
 */
Gimbal_Ctrl_Cmd_s *GetGimbalCmdSendPtr();

#endif // !ROBOT_CMD_H