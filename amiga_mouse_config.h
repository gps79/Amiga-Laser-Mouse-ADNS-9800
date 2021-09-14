#ifndef __AMIGA_MOUSE_CONFIG_H__
#define __AMIGA_MOUSE_CONFIG_H__
//=============================================================================
//
// MIT License
// 
// Copyright (c) 2021 Grzegorz Pietrusiak
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Project name: Amiga Laser Mouse ADNS-9800
// Project description: based on PIC18f23k22 microcontroller and ADNS-9800 Laser Gaming Sensor
// PCB for the project: https://www.pcbway.com/project/shareproject/Amiga_Laser_Mouse.html
// Toolchain: SDCC 3.9.0-rc1
// 	gputils-1.5.0-1
//
// Author: Grzegorz Pietrusiak
// Email: gpsspam2@gmail.com
// 
//=============================================================================
// Includes
//=============================================================================
#include <pic18fregs.h>
#include <stdint.h>

//=============================================================================
// Constant macros
//=============================================================================
#define HIGH (1)
#define LOW  (0)
#define INPUT   (1)
#define OUTPUT  (0)

//=============================================================================
// Microcontroller Input and Output ports
//=============================================================================
#define LMB_IN (PORTBbits.RB4)
#define RMB_IN (PORTBbits.RB3)
#define LMB_OUT (LATBbits.LATB0)
#define RMB_OUT (LATBbits.LATB1)
#define NCS (LATAbits.LATA3)
#define MISO (PORTAbits.RA2)
#define MOSI (LATAbits.LATA0)
#define SCLK (LATAbits.LATA1)
#define UART (LATBbits.LATB2)
#define LMB_IN_PORT_DIRECTION (TRISBbits.RB4)
#define RMB_IN_PORT_DIRECTION (TRISBbits.RB3)
#define LMB_OUT_PORT_DIRECTION (TRISBbits.RB0)
#define RMB_OUT_PORT_DIRECTION (TRISBbits.RB1)
#define NCS_PORT_DIRECTION (TRISAbits.RA3)
#define MISO_PORT_DIRECTION (TRISAbits.RA2)
#define MOSI_PORT_DIRECTION (TRISAbits.RA0)
#define SCLK_PORT_DIRECTION (TRISAbits.RA1)
#define UART_PORT_DIRECTION (TRISBbits.RB2)
#define DEMO_PORT_DIRECTION (TRISBbits.RB5)
#define DEMO_PORT_PULLUP (WPUBbits.WPUB5)
#define DEMO_MODE_ENABLED (LOW == PORTBbits.RB5)
//#define IF_CFG if (LOW == PORTBbits.RB5)
//#define IF_NCFG if (HIGH == PORTBbits.RB5)

#define H (LATCbits.LATC5)
#define V (LATCbits.LATC4)
#define HQ (LATCbits.LATC6)
#define VQ (LATCbits.LATC7)
#define H_PORT_DIRECTION (TRISCbits.RC5)
#define V_PORT_DIRECTION (TRISCbits.RC4)
#define HQ_PORT_DIRECTION (TRISCbits.RC6)
#define VQ_PORT_DIRECTION (TRISCbits.RC7)

//=============================================================================
// EEPROM data layout
//=============================================================================
#define EE_CALIB_RESOLUTION_ADDR 0x00

//=============================================================================
//
//=============================================================================
// The MCU is running at 16MHz
// 1ms = 16000 clock cycles = 4000 Nops
#define DELAY_MS(a) delay1ktcy(4*(a))

//=============================================================================
//
//=============================================================================
// 1us = 16 clock cycles = 4 Nops
#define DELAY_1US { Nop();Nop();Nop();Nop(); }

//=============================================================================
//
//=============================================================================
// The MCU is running at 16MHz
// with 4us precision
// delay_us(0-9) will delay for 10us
// delay_us(10) will delay for 10.25us
// delay_us(11-13) will delay for 14us
// delay_us(14) will delay for 14.25us
// delay_us(15-17) will delay for 18us
// delay_us(18) will delay for 18.25us
// and so on
void delay_us(int16_t i16MicrosecondsP); // "p_microseconds" must be >= 10

#endif // __AMIGA_MOUSE_CONFIG_H__
