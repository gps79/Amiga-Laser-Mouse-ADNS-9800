#ifndef __ADNS9800_H__
#define __ADNS9800_H__
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
#include <stdint.h>
#include "amiga_mouse_config.h"

//=============================================================================
#define ADNS_SUPPORTED_PRODUCT_ID 0x33
#define ADNS_FIRMWARE_LENGTH 3070

//=============================================================================
// Registers
//=============================================================================
#define REG_Product_ID                           0x00
#define REG_Revision_ID                          0x01
#define REG_Motion                               0x02
#define REG_Delta_X_L                            0x03
#define REG_Delta_X_H                            0x04
#define REG_Delta_Y_L                            0x05
#define REG_Delta_Y_H                            0x06
#define REG_SQUAL                                0x07
#define REG_Pixel_Sum                            0x08
#define REG_Maximum_Pixel                        0x09
#define REG_Minimum_Pixel                        0x0a
#define REG_Shutter_Lower                        0x0b
#define REG_Shutter_Upper                        0x0c
#define REG_Frame_Period_Lower                   0x0d
#define REG_Frame_Period_Upper                   0x0e
#define REG_Configuration_I                      0x0f
#define REG_Configuration_II                     0x10
#define REG_Frame_Capture                        0x12
#define REG_SROM_Enable                          0x13
#define REG_Run_Downshift                        0x14
#define REG_Rest1_Rate                           0x15
#define REG_Rest1_Downshift                      0x16
#define REG_Rest2_Rate                           0x17
#define REG_Rest2_Downshift                      0x18
#define REG_Rest3_Rate                           0x19
#define REG_Frame_Period_Max_Bound_Lower         0x1a
#define REG_Frame_Period_Max_Bound_Upper         0x1b
#define REG_Frame_Period_Min_Bound_Lower         0x1c
#define REG_Frame_Period_Min_Bound_Upper         0x1d
#define REG_Shutter_Max_Bound_Lower              0x1e
#define REG_Shutter_Max_Bound_Upper              0x1f
#define REG_LASER_CTRL0                          0x20
#define REG_Observation                          0x24
#define REG_Data_Out_Lower                       0x25
#define REG_Data_Out_Upper                       0x26
#define REG_SROM_ID                              0x2a
#define REG_Lift_Detection_Thr                   0x2e
#define REG_Configuration_V                      0x2f
#define REG_Configuration_IV                     0x39
#define REG_Power_Up_Reset                       0x3a
#define REG_Shutdown                             0x3b
#define REG_Inverse_Product_ID                   0x3f
#define REG_Motion_Burst                         0x50
#define REG_SROM_Load_Burst                      0x62
#define REG_Pixel_Burst                          0x64

//=============================================================================
typedef struct
{
    unsigned FRAME_PIX_FIRST   : 1; // Frame capture data is from pixel 0,0
    unsigned OP_MODE           : 2; // Operating mode of the sensor: 00 = Run, 01 = Rest 1, 10 = Rest 2, 11 = Rest 3
    unsigned                   : 2; 
    unsigned LP_VALID          : 1; // laser power is valid
    unsigned FAULT             : 1; // Indicates that the XY_LASER is shorted to GND.
    unsigned MOT               : 1; // Motion since last report or Shutdown
} motion_t;

//=============================================================================
static inline void ADNS_com_begin(void)
{
  NCS = LOW;
}

//=============================================================================
static inline void ADNS_com_end(void)
{
  NCS = HIGH;
}

//=============================================================================
uint8_t ADNS_read_reg(uint8_t u8RegAddrP);

//=============================================================================
void ADNS_write_reg(uint8_t u8RegAddrP, uint8_t u8DataP);

//=============================================================================
void ADNS_upload_firmware(void);

//=============================================================================

#endif // __ADNS9800_H__