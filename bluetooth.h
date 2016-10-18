#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "xuartns550.h"
#include "xgpio.h"


#define BT_MAX_SIZE 64
#define MILLISECOND 25000

#define min(a, b) ((a) < (b) ? (a) : (b))
#define BTSendWithoutNull(buf) BTSend(buf, sizeof(buf)-1)
#define BTSendCommandAndEcho(buf) BTSendWithoutNull(buf); BTReadAndEcho()

XUartNs550 BT_UART;

int BTSend(char *buf, u32 size);
int BTRead(char *buf, u32 size);
void BTReadAndEcho();
void BTReadFlush();

#endif
