#include "bluetooth.h"
#include "iRobot.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

int BTSend(char *buf, u32 size) {
	char *offset = buf;
	int written = 0;

	while(offset < (buf+size)) {
		u32 toSend = min(BT_MAX_SIZE, (buf+size)-offset);
		XUartNs550_Send(&BT_UART, (u8 *)offset, toSend);
		offset += toSend;
		written += toSend;
		wait(50);
	}

	return written;
}

int BTRead(char *buf, u32 size) {
	memset(buf, 0, size);
	char *offset = buf;
	u32 read = 0;
	u32 totalRead = 0;

	do {
		u32 toRead = min(BT_MAX_SIZE, (buf+size)-offset);
		read = XUartNs550_Recv(&BT_UART, (u8 *)offset, toRead);
		totalRead += read;
		offset += read;
		wait(50);
	} while(read != 0 && offset < (buf+size));

	return totalRead;
}

void BTReadAndEcho() {
	char buf[256];
	int read = 0;
	do {
		read = BTRead(buf, sizeof(buf));
		xil_printf("BT: \"%s\"\r\n", buf);
	}
	while(sizeof(buf) == read);
}



void BTReadFlush() {
	char buf[16];
	while(BTRead(buf, sizeof(buf))) {}
}
