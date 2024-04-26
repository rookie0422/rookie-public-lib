#ifndef __AT_COMMAND_H
#define __AT_COMMAND_H

#define AT_OK        0
#define AT_ERR      -1
#define AT_TIMEOUT  -2

int ATInit(void);
void ATRecvParser( void * params);
void MQTT_Client_Task(void *Param);

/* eg. buf = "AT+CIPMODE=1"
 *     timeout : ms
 */
int ATSendCmd(char *buf, char *resp, int resp_len, int timeout);

int ATSendData(unsigned char *buf, int len, int timeout);
int ATReadData(unsigned char *c, int timeout);

int ATReadPacket(char *buf, int len, int *resp_len, int timeout);


#endif

