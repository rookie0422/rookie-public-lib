#include "app.h"
#include "vmsgexec.h"
#include <string.h>
#define LOG_TAG "APP"
#include "elog.h"
#include "bsp.h"
#include "vkey.h"
#include "dht11.h"
#include "main.h"
#include "cmd_queue.h"
#include "uart_interface.h"
#include "cmd_process.h"

const key_map_t key_map[3] = {

    { KEY1_GPIO_Port, KEY1_Pin, KEY1, 0 },
    { KEY2_GPIO_Port, KEY2_Pin, KEY2, 0 },
    { KEY3_GPIO_Port, KEY3_Pin, KEY3, 0 }
};

void app_uart_handle_effective_frame_callback(void *revobj, uint8_t *revbuff, uint16_t revlen);

void app_dlgcallback(vwm_message_t *pmsg);

typedef struct app_t {
    vwm_win_handle_t main_wnd;
    vwm_timer_handle_t blink_timer;
    vwm_timer_handle_t sensor_timer;

    volatile uint8_t temp;
    volatile uint8_t humi;

    uart_interface_t lcd_uart;
    uint8 lcd_cmd_buffer[CMD_MAX_SIZE]; //指令缓存
} app_t;

app_t g_app;

void app_vkey_press_callback(void *rev_release_obj, uint16_t key_code)
{
    if (key_code == KEY1) {
    } else if (key_code == KEY2) {
    } else if (key_code == KEY3) {
    }
}

void app_vkey_release_callback(void *rev_release_obj, uint16_t key_code)
{
    if (key_code == KEY1) {
    } else if (key_code == KEY2) {
    } else if (key_code == KEY3) {
    }
}

void app_init()
{
    memset(&g_app, 0, sizeof(app_t));

    g_app.main_wnd = msgexec_create_module(app_dlgcallback, 0);

    if (g_app.main_wnd == NULL) {
        log_e("app_init MsgExec_CreateModule error!");
    } else {
        log_i("app_init MsgExec_CreateModule ok!");
    }

    key_scan__init(key_map, 3, 3, app_vkey_press_callback, NULL, app_vkey_release_callback, NULL);

    queue_reset();

    uart_interface__init(&g_app.lcd_uart, &huart2, 0, 0, NULL, 0);

    uart_interface__start_receive(&g_app.lcd_uart, NULL, app_uart_handle_effective_frame_callback);
}

void app_dlgcallback(vwm_message_t *pmsg)
{
    int16_t ledshowdelay;

    switch (pmsg->msg_id) {
        case VWM_INIT_DIALOG:
            g_app.blink_timer = vwm_create_timer(pmsg->dest_win_handle, 0, 500, 1);
            if (g_app.blink_timer == NULL) {
                log_e("app_init create blink_timer error!");
            } else {
                log_i("app_init create blink_timer ok!");
                vwm_start_timer(g_app.blink_timer);
            }

            g_app.sensor_timer = vwm_create_timer(pmsg->dest_win_handle, 0, 1000, 1);
            if (g_app.sensor_timer == NULL) {
                log_e("app_init create sensor_timer error!");
            } else {
                log_i("app_init create sensor_timer ok!");
                vwm_start_timer(g_app.sensor_timer);
            }

            break;

        case VWM_TIMER:
            if (pmsg->data.v == g_app.blink_timer) {
                if (HAL_GPIO_ReadPin(LED_GPIO_Port, LED_Pin) == GPIO_PIN_SET) {
                    if (g_app.humi >= 90) {
                        ledshowdelay = 100;
                    } else if (g_app.humi <= 50) {
                        ledshowdelay = 500;
                    } else {
                        ledshowdelay = (500 - (g_app.humi - 50) * (500 - 100) / (90 - 50));
                        if (ledshowdelay < 0) {
                            ledshowdelay = 100;
                        }
                    }
                    vwm_restart_timer(g_app.blink_timer, ledshowdelay);
                    log_i("LED Timer:%d", ledshowdelay);
                }
                led_toggole();
            } else if (pmsg->data.v == g_app.sensor_timer) {
                if (DHT11_Read_Data((uint8_t *)&g_app.temp, (uint8_t *)&g_app.humi) != 0) {
                    log_e("DHT11_Read_Data error!");
                } else {
                    log_i("Temp:%d; Humi:%d", g_app.temp, g_app.humi);
                    SetTextInt32(0, 2, g_app.temp, 0, 2);
                    SetTextInt32(0, 3, g_app.humi, 0, 2);
                }
            }
            break;

        default:
            break;
    }
}

void NotifyButton(uint16 screen_id, uint16 control_id, uint8 state)
{
    if (state == 1) {
        log_i("Button Pressed, ScreenID:%d; ControlID:%d", screen_id, control_id);
    } else {
        log_i("Button Released, ScreenID:%d; ControlID:%d", screen_id, control_id);
    }
}

/*! 
*  \brief  握手通知
*/
void NOTIFYHandShake()
{
    SetButtonValue(3, 2, 1);
}

/*! 
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
*/
void ProcessMessage(PCTRL_MSG msg, uint16 size)
{
    uint8 cmd_type = msg->cmd_type;                //指令类型
    uint8 ctrl_msg = msg->ctrl_msg;                //消息的类型
    uint8 control_type = msg->control_type;        //控件类型
    uint16 screen_id = PTR2U16(&msg->screen_id);   //画面ID
    uint16 control_id = PTR2U16(&msg->control_id); //控件ID
    uint32 value = PTR2U32(msg->param);            //数值

    switch (cmd_type) {
        case NOTIFY_TOUCH_PRESS:   //触摸屏按下
        case NOTIFY_TOUCH_RELEASE: //触摸屏松开
            //NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4));
            break;
        case NOTIFY_WRITE_FLASH_OK: //写FLASH成功
            //NotifyWriteFlash(1);
            break;
        case NOTIFY_WRITE_FLASH_FAILD: //写FLASH失败
            //NotifyWriteFlash(0);
            break;
        case NOTIFY_READ_FLASH_OK: //读取FLASH成功
            //NotifyReadFlash(1,cmd_buffer+2,size-6);                                     //去除帧头帧尾
            break;
        case NOTIFY_READ_FLASH_FAILD: //读取FLASH失败
            //NotifyReadFlash(0,0,0);
            break;
        case NOTIFY_READ_RTC: //读取RTC时间
            //NotifyReadRTC(cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7],cmd_buffer[8]);
            break;
        case NOTIFY_CONTROL:
        {
            if (ctrl_msg == MSG_GET_CURRENT_SCREEN) //画面ID变化通知
            {
                //NotifyScreen(screen_id);                                            //画面切换调动的函数
            } else {
                switch (control_type) {
                    case kCtrlButton: //按钮控件
                        NotifyButton(screen_id, control_id, msg->param[1]);
                        break;
                    case kCtrlText: //文本控件
                        //NotifyText(screen_id,control_id,msg->param);
                        break;
                    case kCtrlProgress: //进度条控件
                        //NotifyProgress(screen_id,control_id,value);
                        break;
                    case kCtrlSlider: //滑动条控件
                        //NotifySlider(screen_id,control_id,value);
                        break;
                    case kCtrlMeter: //仪表控件
                        //NotifyMeter(screen_id,control_id,value);
                        break;
                    case kCtrlMenu: //菜单控件
                        //NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
                        break;
                    case kCtrlSelector: //选择控件
                        //NotifySelector(screen_id,control_id,msg->param[0]);
                        break;
                    case kCtrlRTC: //倒计时控件
                        //NotifyTimer(screen_id,control_id);
                        break;
                    default:
                        break;
                }
            }
            break;
        }
        case NOTIFY_HandShake: //握手通知
            NOTIFYHandShake();
            break;
        default:
            break;
    }
}

void app_uart_handle_effective_frame_callback(void *revobj, uint8_t *revbuff, uint16_t revlen)
{
    uint16_t i;
    qsize size = 0;
    for (i = 0; i < revlen; i++) {
        queue_push(revbuff[i]);

        size = queue_find_cmd(g_app.lcd_cmd_buffer, CMD_MAX_SIZE); //从缓冲区中获取一条指令

        if (size > 0 && g_app.lcd_cmd_buffer[1] != 0x07) //接收到指令 ，及判断是否为开机提示
        {
            ProcessMessage((PCTRL_MSG)g_app.lcd_cmd_buffer, size); //指令处理
        } else if (size > 0 && g_app.lcd_cmd_buffer[1] == 0x07)    //如果为指令0x07就软重置STM32
        {
            __disable_fault_irq();
            NVIC_SystemReset();
        }
    }
}

void HAL_UART_IDLECallBack(UART_HandleTypeDef *huart)
{
    if (huart == &huart2) {
        uart_interface__rev_data(&g_app.lcd_uart, false);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart2) {
        uart_interface__rev_data(&g_app.lcd_uart, true);
    }
}
