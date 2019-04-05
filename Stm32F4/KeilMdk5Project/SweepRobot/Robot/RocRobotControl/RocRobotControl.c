/********************************************************************************
 * This code is used for robot control
*********************************************************************************
 * Author        Data            Version
 * Liren         2018/12/16      1.0
********************************************************************************/
#include <string.h>

#include "tim.h"
#include "usart.h"

#include "RocLog.h"
#include "RocLed.h"
#include "RocServo.h"
#include "RocMotor.h"
#include "RocBeeper.h"
#include "RocBattery.h"
#include "RocPca9685.h"
#include "RocZmod4410.h"
#include "RocBluetooth.h"
#include "RocRemoteControl.h"
#include "RocRobotControl.h"
#include "RocRobotDhAlgorithm.h"


#ifdef ROC_ROBOT_CLOSED_LOOP_CONTROL
static double           g_ExpectedAngle = 0;
#endif


static uint32_t g_RobotWalkModeStatus = ROC_ROBOT_WALK_MODE_HEXAPOD;
static ROC_ROBOT_CONTROL_s *g_RocRobotCtrl = NULL;
static ROC_REMOTE_CTRL_INPUT_s g_RocRobotRemoteCtrlInput = {0};

/*********************************************************************************
 *  Description:
 *              Set the robot walk mode
 *
 *  Parameter:
 *              WalkMode: the expected robot walk mode
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static void RocRobotWalkModeSet(uint32_t WalkMode)
{
    g_RobotWalkModeStatus = WalkMode;
}

/*********************************************************************************
 *  Description:
 *              Get the robot walk mode
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              The current robot walk mode
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static uint32_t RocRobotWalkModeGet(void)
{
    return g_RobotWalkModeStatus;
}

/*********************************************************************************
 *  Description:
 *              Robot move core
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2019.03.31)
**********************************************************************************/
static void RocRobotMoveCtrlCore(ROC_ROBOT_CONTROL_s *RobotCtrl)
{
    RocRobotCtrlDeltaMoveCoorInput(g_RocRobotRemoteCtrlInput.X, g_RocRobotRemoteCtrlInput.Y,
                                   g_RocRobotRemoteCtrlInput.Z, g_RocRobotRemoteCtrlInput.A,
                                   g_RocRobotRemoteCtrlInput.H);

    RocRobotGaitSeqUpdate();

    if(ROC_ROBOT_GAIT_CIRCLE_6 != RobotCtrl->CurState.GaitType)
    {
        RocRobotOpenLoopWalkCalculate(&RobotCtrl->CurServo);
    }
    else
    {
        RocRobotOpenLoopCircleCalculate(&RobotCtrl->CurServo);
    }

    //RocServoSpeedSet(g_RocRobotCtrl->CurGait.NomGaitSpeed);
}

/*********************************************************************************
 *  Description:
 *              Robot remote control function
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static void RocRobotRemoteControl(void)
{
    uint8_t RobotCtrlCmd;

    RobotCtrlCmd = RocBluetoothCtrlCmd_Get();

    switch(RobotCtrlCmd)
    {
        case ROC_ROBOT_CTRL_CMD_MOSTAND:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotRemoteCtrlInput.X = 0;
                g_RocRobotRemoteCtrlInput.Y = 0;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = 0;
                g_RocRobotRemoteCtrlInput.H = 0;
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_FORWARD:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotCtrl->CurState.GaitType = ROC_ROBOT_GAIT_TRIPOD_6;

                g_RocRobotRemoteCtrlInput.X = 0;
                g_RocRobotRemoteCtrlInput.Y = ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = 0;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_BAKWARD:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotCtrl->CurState.GaitType = ROC_ROBOT_GAIT_TRIPOD_6;

                g_RocRobotRemoteCtrlInput.X = 0;
                g_RocRobotRemoteCtrlInput.Y = -ROC_ROBOT_DEFAULT_LEG_STEP;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = 0;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;
            }
            break;
        }

        case ROC_ROBOT_CTRL_CMD_LFCLOCK:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotCtrl->CurState.GaitType = ROC_ROBOT_GAIT_CIRCLE_6;

                g_RocRobotRemoteCtrlInput.X = 0;
                g_RocRobotRemoteCtrlInput.Y = 0;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = ROC_ROBOT_TURN_ANGLE_DEFAULT;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;
            }

            break;
        }

        case ROC_ROBOT_CTRL_CMD_RGCLOCK:
        {
            if(ROC_ROBOT_WALK_MODE_HEXAPOD == RocRobotWalkModeGet())
            {
                g_RocRobotCtrl->CurState.GaitType = ROC_ROBOT_GAIT_CIRCLE_6;

                g_RocRobotRemoteCtrlInput.X = 0;
                g_RocRobotRemoteCtrlInput.Y = 0;
                g_RocRobotRemoteCtrlInput.Z = 0;
                g_RocRobotRemoteCtrlInput.A = -ROC_ROBOT_TURN_ANGLE_DEFAULT;
                g_RocRobotRemoteCtrlInput.H = ROC_ROBOT_DEFAULT_FEET_LIFT;
            }

            break;
        }

#ifdef ROC_ROBOT_GAIT_FUNCTION_DEBUG
        case ROC_ROBOT_CTRL_CMD_PARAMET:    RocRobotForwardWalkCtrl();
                                            RocBluetoothCtrlCmd_Set(ROC_NONE);
                                            break;
#endif

        case ROC_ROBOT_CTRL_CMD_CARFORD:    RocMotorRotateDirectionSet(ROC_MOTOR_FORWARD_ROTATE);
                                            break;

        case ROC_ROBOT_CTRL_CMD_CARBAKD:    RocMotorRotateDirectionSet(ROC_MOTOR_REVERSE_ROTATE);
                                            break;

        case ROC_ROBOT_CTRL_CMD_CARMODE:    RocRobotWalkModeSet(ROC_ROBOT_WALK_MODE_CAR);
                                            //RocRobotWalkModeChangeCtrl();
                                            break;

        case ROC_ROBOT_CTRL_CMD_ROTMODE:    RocMotorRotateDirectionSet(ROC_MOTOR_STOPPED_ROTATE);
                                            RocRobotWalkModeSet(ROC_ROBOT_WALK_MODE_HEXAPOD);
                                            //RocRobotWalkModeChangeCtrl();
                                            break;

        case ROC_ROBOT_CTRL_CMD_TURNLDR:    RocMotorServoTurnAngleSet(ROC_MOTOR_SERVO_DEFAULT_ANGLE - 30);
                                            break;

        case ROC_ROBOT_CTRL_CMD_TURNRDR:    RocMotorServoTurnAngleSet(ROC_MOTOR_SERVO_DEFAULT_ANGLE + 30);
                                            break;

        default:                            RocMotorRotateDirectionSet(ROC_MOTOR_STOPPED_ROTATE);
                                            break;
    }
}

/*********************************************************************************
 *  Description:
 *              Robot control init
 *
 *  Parameter:
 *              None
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static void RocRobotControlInit(void)
{
    g_RocRobotCtrl = RocRobotCtrlInfoGet();

    g_RocRobotCtrl->CurGait.NomGaitSpeed = ROC_ROBOT_RUN_SPEED_DEFAULT;

    RocRobotOpenLoopWalkCalculate(&g_RocRobotCtrl->CurServo);

    RocServoControl((int16_t *)(&g_RocRobotCtrl->CurServo));
}

/*********************************************************************************
 *  Description:
 *              Robot start running
 *
 *  Parameter:
 *              None
 *
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static ROC_RESULT RocRobotStartRun(void)
{
    ROC_RESULT Ret = RET_OK;

    RocRobotControlInit();

    //RocBluetoothCtrlCmd_Set(ROC_ROBOT_CTRL_CMD_LFCLOCK);

    RocServoSpeedSet(g_RocRobotCtrl->CurGait.NomGaitSpeed);

    Ret = RocServoTimerStart();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot start servo motor in error, and robot will stop running!");

        while(1);
    }

    return Ret;
}

/*********************************************************************************
 *  Description:
 *              Robot start running
 *
 *  Parameter:
 *              None
 *
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
static ROC_RESULT RocRobotStopRun(void)
{
    ROC_RESULT Ret = RET_OK;

    RocServoSpeedSet(0);

    Ret = RocServoTimerStop();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot stop servo motor in error!");
        return Ret;
    }

    while(1)
    {
        RocBeeperBlink(4, 800);
    }
}

/*********************************************************************************
 *  Description:
 *              Robot control control init
 *
 *  Parameter:
 *              None
 *
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.15)
**********************************************************************************/
void RocRobotInit(void)
{
    ROC_RESULT Ret = RET_OK;

    ROC_LOGW("############# Robot hardware version is 0.5! #############");

    Ret = RocLedInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocBatteryInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocBluetoothInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocPca9685Init();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocServoInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    //Ret = RocZmod4410Init();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocMotorInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    //Ret = RocRemoteControlInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");

        while(1);
    }

    Ret = RocBeeperInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");
    
        while(1);
    }

    ROC_LOGI("Robot hardware init is in success, and the system start running.");

    Ret = RocRobotAlgoCtrlInit();
    if(RET_OK != Ret)
    {
        ROC_LOGE("Robot hardware is in error, the system will not run!");
    
        while(1);
    }

    Ret = RocRobotStartRun();
    if(RET_OK == Ret)
    {
        ROC_LOGW("############# Robot is running! Be careful! #############");
    }
}

/*********************************************************************************
 *  Description:
 *              The interrupt service handle for timer
 *
 *  Parameter:
 *              *htim: the point of the interrupt timer
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.16)
**********************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(TIM6 == htim->Instance)
    {
        RocLedToggle();

        RocRobotRemoteControl();

        if(ROC_TRUE == RocServoTurnIsFinshed())
        {
            RocRobotMoveCtrlCore(g_RocRobotCtrl);
        }

        RocServoControl((int16_t *)(&g_RocRobotCtrl->CurServo));
    }
    else if(TIM7 == htim->Instance)
    {
        RocBatteryVoltageAdcSample();
    }
}

/*********************************************************************************
 *  Description:
 *              Robot control control while main
 *
 *  Parameter:
 *              None
 *
 *
 *  Return:
 *              None
 *
 *  Author:
 *              ROC LiRen(2018.12.15)
**********************************************************************************/
void RocRobotMain(void)
{
    if(g_BtRecvEnd == ROC_TRUE)
    {
        ROC_LOGI("Bluetooth receive (%d) data(%s).", g_BtRxDatLen, g_BtRxBuffer);

        RocBluetoothData_Send(g_BtRxBuffer, g_BtRxDatLen);

        memset(g_BtRxBuffer, 0, g_BtRxDatLen);

        g_BtRecvEnd = ROC_FALSE;
    }

    if(ROC_ROBOT_BATTERY_LIMITED_VOLTATE > RocBatteryVoltageGet())
    {
        //RocRobotStopRun();
    }

    if(ROC_ROBOT_CTRL_MEASURE_START == RocBluetoothCtrlCmd_Get())
    {
        RocZmode4410MeasureStart();

        if(ROC_TRUE == RocZmod4410SensorStatusIsChange())
        {
            RocBeeperBlink(4, 800);
        }
    }
    else if(ROC_ROBOT_CTRL_MEASURE_STOP == RocBluetoothCtrlCmd_Get())
    {
        RocZmode4410MeasureStop();
    }
}
