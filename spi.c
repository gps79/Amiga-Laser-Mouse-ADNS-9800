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

#include "spi.h"
#include <stdint.h>
#include "amiga_mouse_config.h"

//=============================================================================

static inline uint8_t SPI_bitBangData(uint8_t u8DataToSendP)
{
    uint8_t u8ReceivedData = 0;

    for (uint8_t u8BitNum=8; u8BitNum != 0; u8BitNum--)
    {
        SCLK = LOW; // The ADNS-9800 sensor outputs MISO bits on falling edges of SCLK

        // Send a bit to Slave; It takes more than max(t_setup,MOSI; t_dly,MISO)
        MOSI = (u8DataToSendP & 0x80)? HIGH : LOW; // Take most significant bit from the byte to send
        u8DataToSendP <<= 1;                       // and move all bits to the left
        // t_setup,MOSI = 120ns - minimum time between setting bit on MOSI and rising edge of SCLK
        // t_dly,MISO = 120ns - maximum time from falling edge of SCLK to show up new bit value on MISO
        u8ReceivedData <<= 1; // To receive data from the Slave shift all bits of the output byte to the left,
        u8ReceivedData |= (MISO == LOW)? 0x00 : 0x01; // receive one bit from MISO and store it as the least significant bit in the output byte
        
        SCLK = HIGH; // The ADNS-9800 Sensor reads MOSI on rising edges of SCLK.
        Nop(); // t_hold,MOSI = 200ns
    } 
    return u8ReceivedData;
}

//=============================================================================

uint8_t SPI_transfer(uint8_t u8DataP)
{
    uint8_t u8ReceivedData;
    u8ReceivedData = SPI_bitBangData(u8DataP);
    //delay_us(120); // SPI time between write or write and read commands = 120us
                     // SPI time between read and subsequent commands = 20us
    return u8ReceivedData;
}

//=============================================================================

void SPI_init(void)
{
    NCS_PORT_DIRECTION = OUTPUT;
    MISO_PORT_DIRECTION = INPUT;
    MOSI_PORT_DIRECTION = OUTPUT;
    SCLK_PORT_DIRECTION = OUTPUT;
    NCS = HIGH;
}

//=============================================================================
