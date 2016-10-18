/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "platform.h"
#include "math.h"

#include "xparameters.h"
#include "xbasic_types.h"
#include "xgpio.h"
#include "xstatus.h"
#include "xuartns550.h"
#include "mb_interface.h"
#include "bluetooth.h"
#include "iRobot.h"
#include "iRobot_commands.h"


#define POLL_DELAY 50
#define DEFAULT_SPEED 128 // mm/s

// Bluetooth pins
XGpio BT_Gpio_RTS;
XGpio BT_Gpio_CTS;
XGpio BT_Gpio_STATUS;
XGpio BT_Gpio_RESET;

double sind(double angle)
{
	double angleradians = angle * M_PI / 180.0f;
	return sin(angleradians);
}

double cosd(double angle)
{
	double angleradians = angle * M_PI / 180.0f;
	return cos(angleradians);
}

int initGpio(XGpio *GpioInput, u16 DeviceId, u32 direction)
{
	int Status;

	/*
	 * Initialize the GPIO driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	 Status = XGpio_Initialize(GpioInput, DeviceId);
	 if (Status != XST_SUCCESS)  {
		 return XST_FAILURE;
	 }


	 /*
	  * Set the direction for all signals to be inputs
	  */
	 XGpio_SetDataDirection(GpioInput, 1, direction);

	 return XST_SUCCESS;
}

double calculateAngle(char prevCommand, double prevAngle)
{
	if (prevCommand != 'R'&& prevCommand != 'L')
	{
		return prevAngle;
	}
	else
	{
		double tdelta = (POLL_DELAY + 650) / 1000.0; // 15 ms is the wait delay in each movement function for the iRobot
		double radius = 165.0; //mm

		double angle_delta_radians = (DEFAULT_SPEED * tdelta) / radius;
		double angle_delta_degrees = angle_delta_radians * 180 / M_PI;


		// return new angle
		if (prevCommand == 'R')
		{
			return prevAngle + angle_delta_degrees;
		}
		else
		{
			return prevAngle - angle_delta_degrees;
		}
	}
}

char readCommand()
{
	char buf = ' ';
	if(!BTRead(&buf, 1))
		return '0';

	char ret;
	switch(buf)
	{
		case 'F':
			ret = 'F';
			break;
		case 'S':
			ret = 'S';
			break;
		case 'B':
			ret = 'B';
			break;
		case 'R':
			ret = 'R';
			break;
		case 'L':
			ret = 'L';
			break;
		default:
			ret = '0';
			break;
	}
	return ret;
}

void executeCommand(char c)
{

	switch(c)
	{
		case 'F':
			forward();
			break;
		case 'S':
			stop();
			break;
		case 'B':
			reverse();
			break;
		case 'R':
			turn(0xFF, 0xFF);
			break;
		case 'L':
			turn(0, 1);
			break;
	}
}

void connectToBluetooth()
{
    u32 connected = XGpio_DiscreteRead(&BT_Gpio_STATUS, 1);

    while(!connected)
    {
    	connected = XGpio_DiscreteRead(&BT_Gpio_STATUS, 1);
    	wait(250);
    }
}

void initializeiRobot()
{
	XUartNs550_SetBaud(XPAR_RS232_UART_0_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ, 57600);
	XUartNs550_SetLineControlReg(XPAR_RS232_UART_0_BASEADDR, XUN_LCR_8_DATA_BITS);

	// Initialize
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0x80);
	XUartNs550_SendByte(XPAR_RS232_UART_0_BASEADDR,0x84);

	// Sleep for 2 seconds
	wait(2000);
}

void inttochar(char* ptr, int integer)
{
	int i = 3;
	int j;
	int en = 1;

	// initializing ptr array
	for(j = 0; j < 4; j++)
	{
		ptr[j] = '0';
	}

	// calculating individual bits from integer input
	while (i >= 0)
	{
		if (i == 3 && en == 1)
		{
			ptr[i] = integer%10 + '0';
		}
		else
		{
			ptr[i] = (integer/10)%10 + '0';
		}


		if (en == 0)
		{
			i -= 1;
			integer = integer/10;
		}
		else
		{
			i -= 1;
			en = 0;
		}
	}
}

//double distance_fun(double start_x, double start_y, double end_x, double end_y)
//{
//	return sqrt( pow(end_x - start_x, 2) + pow(end_y - start_y, 2) );
//}

void sendCoord(double x, double y)
{
	int intx, inty;
	char charx[4], chary[4], combo[10];


	intx = ceil(x);
	inttochar(charx, intx);

	inty = ceil(y);
	inttochar(chary, inty);

	int i;

	for(i = 0; i < 10; i++)
	{
		if(i == 0 || i == 1 || i == 2 || i == 3)
			combo[i] = charx[i];

		else if(i == 4)
			combo[i] = ',';

		else if(i == 5 || i == 6 || i == 7 || i == 8)
			combo[i] = chary[i - 5];

		else if(i == 9)
			combo[i] = '\0';
	}

	BTSend(combo, 10);
}

void getCoord(double * x, double * y, double angle, double speed)
{
	// every 50 ms AND when a new cmd is issued,
		// figure out direction (angle) and speed
		// time since last poll (usually 50 ms, but can be shorter if new cmd is issued)
		// hypotenuse = speed(cm/s) * time(ms) - previousDistance(cm)
		// x = cos(angle(degrees)) * hypotenuse
		// y = sin(angle(degrees)) * hypotenuse
		// units = ?
		// send x-y coordinates to Android via Bluetooth


	double hypo = speed * (POLL_DELAY/1000.0); //6.4
	(*x) += cosd(angle) * hypo;
	(*y) += sind(angle) * hypo;
	sendCoord(*x, *y);
}

void print(char *str);

int main()
{
    init_platform();

    initGpio(&BT_Gpio_RTS,    XPAR_RTS_DEVICE_ID,    0);
    initGpio(&BT_Gpio_CTS,    XPAR_CTS_DEVICE_ID,    1);
    initGpio(&BT_Gpio_STATUS, XPAR_STATUS_DEVICE_ID, 1);
	initGpio(&BT_Gpio_RESET,  XPAR_RESET_DEVICE_ID,  1);

	XGpio_DiscreteWrite(&BT_Gpio_RESET, 1, 1); // RESET is active low

	XUartNs550Format BTFormat = {
			 .BaudRate = XUN_NS16550_MAX_RATE,
			 .DataBits = XUN_FORMAT_8_BITS,
			 .Parity   = XUN_FORMAT_NO_PARITY,
			 .StopBits = XUN_FORMAT_1_STOP_BIT,
		};

	int status = XUartNs550_Initialize(&BT_UART, XPAR_UART_BT_DEVICE_ID);

	if (status != XST_SUCCESS){
		return XST_FAILURE;
	}
	XUartNs550_SetDataFormat(&BT_UART, &BTFormat);

	connectToBluetooth();

    initializeiRobot();

    double angle = -90;
    double x = 240;
    double y = 300;

    BTReadFlush();
    char prev = 'S';
    while(1)
    {

    	char command = readCommand();
    	angle = calculateAngle(prev, angle);
    	if(command == '0')
    	{
    		command = prev;
    	}
    	else
    	{
    		prev = command;
    	}
    	executeCommand(command);

    	// poll for irobot information
    	if(prev == 'F')
    	{
    		getCoord(&x, &y, angle, DEFAULT_SPEED);
    	}
    	else if(prev == 'B')
    	{
    		getCoord(&x, &y, angle + 180, DEFAULT_SPEED);
    	}
    	else
    	{

    		getCoord(&x, &y, angle, 0);
    	}

    	wait(POLL_DELAY);
    }

//    int i;
//    for(i = 0; i < 10; i++)
//    	getCoord(&x, &y, angle, DEFAULT_SPEED);
//
//    angle = 45;
//    for(i = 0; i < 10; i++)
//    	getCoord(&x, &y, angle, DEFAULT_SPEED);

//    angle = 180;
//    for(i = 0; i < 10; i++)
//    	getCoord(&x, &y, angle, DEFAULT_SPEED);
//
//    angle = 270;
//    for(i = 0; i < 10; i++)
//    	getCoord(&x, &y, angle, DEFAULT_SPEED);

   return 0;
}
