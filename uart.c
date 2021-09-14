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
#include "uart.h"
#include <delay.h>

//=============================================================================
// Sends one byte (8-bit) of data over UART TX pin
// - bitrate: 9600
// - stopbits: 1
// - parity: none
// 
//=============================================================================
void UART_putc(uint8_t u8CharP)
{
    // each bit should last for 104.1666us on the line
    // send start bit
    UART = LOW;
    delay10tcy(39);Nop();Nop();Nop();Nop();Nop();Nop();Nop(); // 104.25us
    
    for (uint8_t u8BitNum = 0; u8BitNum<8; u8BitNum++)
    {
        UART = (u8CharP & 0x01)? HIGH:LOW;
        u8CharP >>= 1;
        delay10tcy(39);Nop();Nop();Nop();Nop(); // 104us - 104.5us
    }
    delay10tcy(1);Nop();Nop(); // 8th bit: 104us
    // send stop bit
    UART = HIGH;
    delay10tcy(36); // delay between two bytes sent 105.25us
}

//=============================================================================
// Sends zero-terminated string to UART port
//=============================================================================
void UART_puts(const char *szStringP)
{
    while(*szStringP)
    {
        UART_putc((uint8_t)*szStringP);
        szStringP++;
    }
}

//=============================================================================
void UART_putb(uint8_t u8ByteP)
{
    static const char aHex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    UART_putc(aHex[u8ByteP>>4]);
    UART_putc(aHex[u8ByteP&0x0f]);
}

//=============================================================================
