#include "app.h"
#include "vMsgExec.h"
#include <string.h>
#include "elog.h"
#include "bsp.h"
#include "dht11.h"
#define LOG_TAG "APP"

void app_dlgcallback(VWM_MESSAGE * pMsg);


typedef struct app_t
{
    VWM_HWIN            main_hwnd;
    VWM_HTIMER          blink_timer;
    VWM_HTIMER          keyscan_timer;
    VWM_HTIMER          dht_timer;
    uint8_t             temp;
    uint8_t             humi;
    

}app_t;

app_t g_app;


void app_init()
{
    memset(&g_app, 0, sizeof(app_t));

    g_app.main_hwnd =  MsgExec_CreateModule(app_dlgcallback, 0);

    if(g_app.main_hwnd == NULL){
        log_e("app_init MsgExec_CreateModule error!");
    }else{
        log_i("app_init MsgExec_CreateModule ok!");
    }
    

}

void app_dlgcallback(VWM_MESSAGE * pMsg)
{
    switch (pMsg->MsgId)
    {
    case VWM_INIT_DIALOG:
        g_app.blink_timer =  VWM_CreateTimer(pMsg->hWin, 0, 2000, 1);
        if (g_app.blink_timer == NULL){
            log_e("app_init VWM_CreateTimer error!");
        }else{
            log_i("app_init VWM_CreateTimer ok!");
            VWM_StartTimer(g_app.blink_timer);
        }
        //break;

        g_app.keyscan_timer =  VWM_CreateTimer(pMsg->hWin, 0, 10, 1);
        if (g_app.keyscan_timer == NULL){
            log_e("app_init keyscan_timer error!");
        }else{
            log_i("app_init keyscan_timer ok!");
            VWM_StartTimer(g_app.keyscan_timer);
        }
        //break;

        g_app.dht_timer =  VWM_CreateTimer(pMsg->hWin, 0, 5000, 1);
        if (g_app.dht_timer == NULL){
            log_e("app_init dht_timer error!");
        }else{
            log_i("app_init dht_timer ok!");
            VWM_StartTimer(g_app.dht_timer);
        }
        break;

    case VWM_TIMER:
        if(pMsg->Data.v == g_app.blink_timer) {
            led_g_toggle();
        }else if(pMsg->Data.v == g_app.dht_timer) {
            DHT11_Read_Data(&g_app.temp,&g_app.humi);
            log_i("temp: %d,humi: %d",g_app.temp,g_app.humi);
        }else if(pMsg->Data.v == g_app.keyscan_timer) {
            keyscan();
        }

        break;
    
    default:
        break;
    }
}

