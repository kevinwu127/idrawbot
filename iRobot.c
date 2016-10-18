/*
 * iRobot.c
 *
 *  Created on: May 20, 2015
 *      Author: CS152B
 */

#include "iRobot.h"

void wait(int time){
	// wait for 15ms
	int i;
	for (i=0;i<MILLISECOND*time;i++)
		asm("nop");
}

void start(){
	//Enable Control
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,128);	//128 start command
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,132);	//131 is safe mode, 132 is full mode
}

void forward(){
	// Go forward
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,137);	//drive command
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0x00);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0x80);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0x80);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0);
	wait(15);
}

void reverse(){
	// Go backward
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,137);	//drive command
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0xff);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0xb0);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0x80);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0);
	wait(15);
}

void distance(u8 upper, u8 lower){
	// Go distance
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,156);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,upper);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,lower);
	wait(15);
}

void turn(u8 upper, u8 lower){
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,137);	//drive command
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0x00);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0x80);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,upper);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,lower);
	wait(15);
}

void angle(u8 upper, u8 lower){
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,157);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,upper);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,lower);
	wait(15);
}

void stop(){
	// Stop
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,137);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0);
	wait(15);
}

u8 sensor_request(){
	// Poll iRobot's bump sensor
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,142);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,7);
	return XUartNs550_RecvByte(XPAR_RS232_UART_0_BASEADDR);
}
