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
#include "adns9800.h"
#include <pic18fregs.h>
#include <delay.h>
#include "uart.h"
#include "spi.h"
#include "adns9800_srom_A6.h"

//=============================================================================
#define WRITE_REQUEST 0x80

//=============================================================================
uint8_t ADNS_read_reg(uint8_t u8RegAddrP)
{
    ADNS_com_begin();

    (void)SPI_transfer(u8RegAddrP & 0x7f);
    delay_us(100);
    uint8_t u8Data = SPI_transfer(0);
    Nop();
    ADNS_com_end();
    delay_us(19);

    return u8Data;
}

//=============================================================================
void ADNS_write_reg(uint8_t u8RegAddrP, uint8_t u8DataP)
{
    ADNS_com_begin();
    
    (void)SPI_transfer(u8RegAddrP | WRITE_REQUEST);
    (void)SPI_transfer(u8DataP);
    delay_us(20);
    ADNS_com_end();
    delay_us(100);
}

//=============================================================================
void ADNS_upload_firmware(void)
{
    // Initializing firmware transfer
    ADNS_write_reg(REG_Configuration_IV, 0x02);
    ADNS_write_reg(REG_SROM_Enable, 0x1d);
    DELAY_MS(10);
    ADNS_write_reg(REG_SROM_Enable, 0x18);

    // Transferring the firmware to ADNS
    ADNS_com_begin();
    (void)SPI_transfer(REG_SROM_Load_Burst | WRITE_REQUEST);
    delay_us(15);
    for (uint16_t u16Idx = 0; u16Idx < ADNS_FIRMWARE_LENGTH; u16Idx++)
    {
        (void)SPI_transfer(ADNS_firmware_data[u16Idx]);
    }
    delay_us(2); // 10us delay before exiting burst mode
    ADNS_com_end();
    delay_us(200); // Datasheet says wait 160ms for ADNS to exit the burst mode before starting new communication. Waiting 40us more as 160ms was too short.
}

//=============================================================================
