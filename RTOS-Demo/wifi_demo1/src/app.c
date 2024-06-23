#include "app.h"
#include "vmsgexec.h"
#include <string.h>
#include "elog.h"
#include "bsp.h"
#include "vkey.h"
#include "esp8266.h"
#include "uart_interface.h"
#include "dht11.h"
#define LOG_TAG "APP"

extern uart_interface_t esp_wifi_usart;

const key_map_t key_map[2] = {

    { KEY1_GPIO_Port, KEY1_Pin, KEY1, 0 },
    { KEY2_GPIO_Port, KEY2_Pin, KEY2, 0 }
};

typedef struct app_t {
    vwm_win_handle_t main_wnd; //主窗口

    vwm_timer_handle_t dht_timer;
    vwm_timer_handle_t key_scan_timer;
    vwm_timer_handle_t wifi_get_ip_timer;

    uint8_t temp;
    uint8_t humi;

} app_t;

void app_dlgcallback(vwm_message_t *pmsg);

app_t g_app;

void app_init()
{
    memset(&g_app, 0, sizeof(app_t));

    g_app.main_wnd = msgexec_create_module(app_dlgcallback, 0);

    if (g_app.main_wnd == NULL) {
        log_e("app_init MsgExec_CreateModule error!");
    } else {
        log_i("app_init MsgExec_CreateModule ok!");
    }

    key_scan__init(g_app.main_wnd, key_map, 2, 3);

    esp8266_init(&huart3);

    //uart_interface__init(&g_app.lcd_uart, &huart1,0,0,NULL,0);

    //uart_interface__start_receive(&g_app.lcd_uart,NULL,app_uart_receive_effective_frame_callback);
}

void app_dlgcallback(vwm_message_t *pmsg)
{
    switch (pmsg->msg_id) {
        case VWM_INIT_DIALOG:

            g_app.dht_timer = vwm_create_timer(pmsg->dest_win_handle, 0, 2000, 1);
            if (g_app.dht_timer == NULL) {
                log_e("app_init VWM_CreateTimer dht_timer error!");
            } else {
                log_i("app_init VWM_CreateTimer dht_timer ok!");
                vwm_start_timer(g_app.dht_timer);
            }

            g_app.key_scan_timer = vwm_create_timer(pmsg->dest_win_handle, 0, 10, 1);
            if (g_app.key_scan_timer == NULL) {
                log_e("app_init VWM_CreateTimer key_scan_timer error!");
            } else {
                log_i("app_init VWM_CreateTimer key_scan_timer ok!");
                vwm_start_timer(g_app.key_scan_timer);
            }

            g_app.wifi_get_ip_timer = vwm_create_timer(pmsg->dest_win_handle, 0, 5000, 1);
            if (g_app.wifi_get_ip_timer == NULL) {
                log_e("app_init VWM_CreateTimer wifi_get_ip_timer error!");
            } else {
                log_i("app_init VWM_CreateTimer wifi_get_ip_timer ok!");
                vwm_start_timer(g_app.wifi_get_ip_timer);
            }

            break;

        case VWM_TIMER:

            if (pmsg->data.v == g_app.key_scan_timer) {
                key_scan__scan();
            } else if (pmsg->data.v == g_app.dht_timer) {
                DHT11_Read_Data(&g_app.temp, &g_app.humi);
                //log_i("temp: %d,humi: %d",g_app.temp,g_app.humi);
                HAL_GPIO_TogglePin(LED_B_GPIO_Port, LED_B_Pin);
            } else if (pmsg->data.v == g_app.wifi_get_ip_timer) {
                log_i("get IP CIFSR...\r\n");
                while (esp8266_send_cmd("AT+CIFSR\r\n", "OK"))
                    ;
            }

            break;

        case VWM_KEY:

            if (pmsg->data.v == key_map[0].key_code) {
                if (key_scan__getkeystate(key_map[0].key_code) == GPIO_PIN_RESET) {
                    log_i("KEY1     RESET");

                } else if (key_scan__getkeystate(key_map[0].key_code) == GPIO_PIN_SET) {
                    log_i("KEY1     SET");
                    log_i("temp: %d", g_app.temp);
                }
            } else if (pmsg->data.v == key_map[1].key_code) {
                if (key_scan__getkeystate(key_map[1].key_code) == GPIO_PIN_RESET) {
                    log_i("KEY2     RESET");

                } else if (key_scan__getkeystate(key_map[1].key_code) == GPIO_PIN_SET) {
                    log_i("KEY2     SET");
                    log_i("humi: %d", g_app.humi);
                }
            }

            break;

        default:
            break;
    }
}
