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
// Features implemented:
//=============================================================================
// - SPI communication between ADNS-9800 and PIC micro (bit banging)
// - one-way UART communication (TX only) sending debug information via PORTB.RB2 (bit banging)
// - handling of 2 mouse buttons
// - quadrature encoded protocol to send mouse position changes
// - sending the firmware to ADNS-9800
// - check for ADNS-9800 communication errors at startup
// - demo mode - move mouse pointer along the square edge on the screen
// - XY resolution change (calibration) by mouse buttons press if the mouse is started with both buttons pressed
// - read and write of XY resolution calibration value to from/to built-in EEPROM
// - mouse buttons debouncing in calibration mode

//=============================================================================
// FUSES
//=============================================================================

//The following fuses need to be set while flashing:

// Disable extended instruction set
#pragma config XINST=OFF

// Internal oscillator block.
//#pragma config FOSC = INTIO67

// Watchdog timer OFF
//#pragma config WDTEN = OFF

//=============================================================================
#include "amiga_mouse_config.h"
#include <pic18fregs.h>
#include <delay.h>
#include "adns9800.h"
#include "spi.h"
#include "uart.h"
#include "eeprom.h"
#include <stdbool.h>

//=============================================================================
// Global variables
//=============================================================================
bool g_bAdnsEnabled = false;
uint8_t g_u8Resolution = 0;
bool g_bCalibrationMode = false;
uint8_t g_u8GestureMode = 0; // a mode of a ("Yes" or "No") gesture drawn by cursor

//=============================================================================
void delay_us(int16_t i16MicrosecondsP) // "i16MicrosecondsP" must be >= 10
{
    i16MicrosecondsP -= 10;
    while (i16MicrosecondsP > 0) // 1 loop lap = 4us
    {
       Nop();
       Nop();
       Nop();
       i16MicrosecondsP -= 4;
    }
}

//=============================================================================
static inline void SetQuadraturePhases(uint8_t u8HorPhaseP, uint8_t u8VerPhaseP)
{
    if (0 == u8HorPhaseP)         { H = LOW;  HQ = LOW;  }
    else if (1 == u8HorPhaseP)    { H = HIGH; HQ = LOW;  }
    else if (2 == u8HorPhaseP)    { H = HIGH; HQ = HIGH; }
    else if (3 == u8HorPhaseP)    { H = LOW;  HQ = HIGH; }

    if (0 == u8VerPhaseP)         { V = LOW;  VQ = LOW;  }
    else if (1 == u8VerPhaseP)    { V = HIGH; VQ = LOW;  }
    else if (2 == u8VerPhaseP)    { V = HIGH; VQ = HIGH; }
    else if (3 == u8VerPhaseP)    { V = LOW;  VQ = HIGH; }
    // Quadrature pulses should be no shorter than 157us to prevent wrong counter 
    // reading which is done every screen refresh (20ms for PAL) on Amiga
    if (0 == g_u8GestureMode)
        delay_us(150); // normal speed of cursor movement
    else
        DELAY_MS(4); // a speed of cursor shaking
}

//=============================================================================
static inline void ADNS_set_resolution(void)
{
    g_u8Resolution = EE_read_byte(EE_CALIB_RESOLUTION_ADDR);
    if ((g_u8Resolution < 0x01) || (g_u8Resolution > 0xA4))
    {
        UART_puts("Invalid value in EEPROM 0x");
        UART_putb(g_u8Resolution);
        UART_puts(". saving default value 0x44\n");
        g_u8Resolution = 0x44; // setting default resolution
        if (!EE_write_byte(EE_CALIB_RESOLUTION_ADDR, g_u8Resolution))
        {
            UART_puts("Can't store calibration value in EEPROM.\n");
        }
    }
    UART_puts("Setting XY resolution: ");
    UART_putb(g_u8Resolution);
    UART_puts("\n");
    ADNS_write_reg(REG_Configuration_I, g_u8Resolution);
    
    uint8_t u8Stored = ADNS_read_reg(REG_Configuration_I);
    UART_puts("XY resolution read from ADNS: 0x");
    UART_putb(u8Stored);
    UART_puts("\n");
}

//=============================================================================
static inline void ADNS_init(void)
{
    ADNS_com_begin();
    ADNS_com_end(); // ensure that the serial port is reset
    ADNS_write_reg(REG_Power_Up_Reset, 0x5a);
    DELAY_MS(50); // 50ms power up time
    // read registers 0x02 to 0x06 (and discard the data)
    (void)ADNS_read_reg(REG_Motion);
    (void)ADNS_read_reg(REG_Delta_X_L);
    (void)ADNS_read_reg(REG_Delta_X_H);
    (void)ADNS_read_reg(REG_Delta_Y_L);
    (void)ADNS_read_reg(REG_Delta_Y_H);
    // upload the firmware
    UART_puts("ADNS9800 Uploading firmware...\n");
    ADNS_upload_firmware();
    
    // check firmware correctness
    uint8_t u8ProductId = ADNS_read_reg(REG_Product_ID);
    if (ADNS_SUPPORTED_PRODUCT_ID == u8ProductId)
    {
        // Test communication with ADNS-9800 by checking if Product ID matches Inverse Product ID
        if (0xff == (ADNS_read_reg(REG_Inverse_Product_ID) ^ u8ProductId))
        {
            // SROM CRC test
            ADNS_write_reg(REG_SROM_Enable, 0x15); 
            DELAY_MS(10);
            uint8_t u8CrcLow = ADNS_read_reg(REG_Data_Out_Lower);
            uint8_t u8CrcHigh = ADNS_read_reg(REG_Data_Out_Upper);
            if ((0xEF == u8CrcLow) && (0xBE == u8CrcHigh))
            {
                g_bAdnsEnabled = true;
                // enable laser(bit 0 = 0b), in normal mode (bits 3,2,1 = 000b)
                // reading the actual value of the register is important because the real
                // default value is different from what is said in the datasheet, and if you
                // change the reserved bits (like by writing 0x00...) it would not work.
                uint8_t u8LaserDriveMode = ADNS_read_reg(REG_LASER_CTRL0);
                ADNS_write_reg(REG_LASER_CTRL0, u8LaserDriveMode & 0xf0 );
                ADNS_set_resolution();
                UART_puts("Optical Chip Initialized\n");
            }
            else
            {
                UART_puts("SROM CRC error:0x");
                UART_putb(u8CrcHigh);
                UART_putb(u8CrcLow);
                UART_puts("\n");
            }
        }
        else
        {
            UART_puts("Product ID test failed\n");
        }
    }
    else
    {
        UART_puts("Invalid Product ID:");
        UART_putb(u8ProductId);
        UART_puts("\n");
    }
    if (false == g_bAdnsEnabled) g_bCalibrationMode = false; // disable calibration mode if ADNS chip is not initialized
}

//=============================================================================
void ADNS_uart_print_register(uint8_t u8RegIdP, char * szRegNameP)
{
    uint8_t u8RegValue = ADNS_read_reg(u8RegIdP);
    UART_puts(szRegNameP);
    UART_puts(": 0x");
    UART_putb(u8RegValue);
    UART_puts("\n");
}

//=============================================================================
static inline void ADNS_dispRegisters(void)
{
    ADNS_uart_print_register(REG_Product_ID, "Product ID");
    ADNS_uart_print_register(REG_Revision_ID, "Revision ID");
    ADNS_uart_print_register(REG_Motion, "Motion");
    ADNS_uart_print_register(REG_Observation, "Observation");
    ADNS_uart_print_register(REG_SROM_ID, "SROM ID");
    ADNS_uart_print_register(REG_Configuration_I, "Config I");
    ADNS_uart_print_register(REG_Configuration_II, "Config II");
    ADNS_uart_print_register(REG_Configuration_IV, "Config IV");
    ADNS_uart_print_register(REG_Configuration_V, "Config V");
}

//=============================================================================
static inline void setup(void)
{
    OSCCONbits.SCS = 3; // internal oscillator
    OSCCONbits.IRCF = 7; // 7 - 16MHz, 6 - 8MHz, 5 - 4MHz
    
    ADCON1 = 0x0F; //Disable all analog inputs
    ANSELA = 0; //Disable all analog inputs on port A
    ANSELB = 0; //Disable all analog inputs on port B
    DEMO_PORT_DIRECTION = INPUT;
    INTCON2bits.RBPU = LOW; // RBPU is negated, so zero enable it. serve as general key for all PORTB pull-ups resistors    
    DEMO_PORT_PULLUP = HIGH;
    LMB_IN_PORT_DIRECTION = INPUT;
    RMB_IN_PORT_DIRECTION = INPUT;
    LMB_OUT_PORT_DIRECTION = OUTPUT;
    RMB_OUT_PORT_DIRECTION = OUTPUT;
    LMB_OUT = HIGH; // button not pressed
    RMB_OUT = HIGH; // button not pressed
    TRISC=0x00; //  Set all of PORTC as outputs. TX and RX must be set as outputs first

    UART_init();
    UART_puts("Amiga Laser Mouse driver 2.0 by gps79\n");
    
    // Enable demo mode if both buttons are pressed during startup
    if ((LOW == LMB_IN) && (LOW == RMB_IN))
    {
        g_bCalibrationMode = true;
        UART_puts("Calibration ON\n");
    }    

    // Set V, QV, H, HQ ports as outputs
    H_PORT_DIRECTION = OUTPUT;
    V_PORT_DIRECTION = OUTPUT;
    HQ_PORT_DIRECTION = OUTPUT;
    VQ_PORT_DIRECTION = OUTPUT;

    SPI_init();
    ADNS_init();
    ADNS_dispRegisters();
}

//=============================================================================
// Draw a square with mouse cursor moving clockwise.
// - 100 points right in 1s
// - 100 points down in 1s
// - 100 points left in 1s
// - 100 points up in 1s
// - 4s of no movement
// and repeat the sequence again
//=============================================================================
static inline void playDemo(int16_t *pi16DeltaXP, int16_t *pi16DeltaYP)
{
    static uint8_t u8Phase = 0;
    static uint8_t u8Step = 0;
    if (0 == u8Phase) (*pi16DeltaXP)++; // move right
    if (1 == u8Phase) (*pi16DeltaYP)++; // move down
    if (2 == u8Phase) (*pi16DeltaXP)--; // move left 
    if (3 == u8Phase) (*pi16DeltaYP)--; // move up
    DELAY_MS(9); // value set to reach ~100 mouse count pulses/s
    u8Step++;
    if (u8Step >= 100)
    {
        u8Step = 0;
        u8Phase = (u8Phase + 1) & 0x07;
        UART_puts("Demo phase ");
        UART_putb(u8Phase);
        UART_puts("\n");
    }
}

//=============================================================================
static inline void handleMouseButtons(int16_t *pi16DeltaXP, int16_t *pi16DeltaYP)
{
    // Handle XY resolution change by pressing LMB (increase) or RMB (decrease) when in Calibration Mode
    // Both buttons click exits Calibration Mode
    if (g_bCalibrationMode)
    {
        bool bApplyNewResolution = false;
        static bool bWaitForButtonsRelease = false;
        static bool bEnteringCalibrationModeCompleted = false;
        if (bEnteringCalibrationModeCompleted)
        {
            if (!bWaitForButtonsRelease)
            {
                if (LOW == LMB_IN)
                {
                    bWaitForButtonsRelease = true;
                    if (g_u8Resolution < 0xA4)
                    {
                        g_u8Resolution++;
                        bApplyNewResolution = true;
                        *pi16DeltaYP += 10;
                    }
                    else
                    {
                        g_u8GestureMode = 1;
                    }
                }
                if (LOW == RMB_IN)
                {
                    bWaitForButtonsRelease = true;
                    if (g_u8Resolution > 0x01)
                    {
                        g_u8Resolution--;
                        bApplyNewResolution = true;
                        *pi16DeltaYP -= 10;
                    }
                    else
                    {
                        g_u8GestureMode = 1;
                    }
                }
            }
            else
            {
                if ((LOW == LMB_IN) && (LOW == RMB_IN))
                {
                    UART_puts("Calibration OFF\n");
                    g_bCalibrationMode = false;
                    g_u8GestureMode = 5;
                }
            }
        }

        if ((HIGH == RMB_IN) && (HIGH == LMB_IN))
        {
            bEnteringCalibrationModeCompleted = true;
            bWaitForButtonsRelease = false;
        }
        if (bApplyNewResolution)
        {
            UART_puts("New XY Res:");
            UART_putb(g_u8Resolution);
            UART_puts("\n");
            ADNS_write_reg(REG_Configuration_I, g_u8Resolution);
            if (!EE_write_byte(EE_CALIB_RESOLUTION_ADDR, g_u8Resolution))
            {
                UART_puts("Can't store calibration value in EEPROM.\n");
            }
            DELAY_MS(100); // wait 100ms as a primitive buttons debouncing

            uint8_t u8Stored = ADNS_read_reg(REG_Configuration_I);
            UART_puts("XY resolution read from ADNS: 0x");
            UART_putb(u8Stored);
            UART_puts("\n");
        }
    }
    else // Normal buttons handling if not in Calibration Mode
    {
        // Handle mouse buttons
        if (HIGH == LMB_IN)
            LMB_OUT = HIGH; // Left Mouse Button not pressed
        else 
            LMB_OUT = LOW; // Left Mouse Button pressed
        if (HIGH == RMB_IN) 
            RMB_OUT = HIGH; // Right Mouse Button not pressed
        else 
            RMB_OUT = LOW; // Right Mouse Button pressed
    }
    
    // Gesture drawing:
    // "No" - horizontal cursor shake
    if (1 == g_u8GestureMode)
    {
        *pi16DeltaXP -= 20;
        g_u8GestureMode = 2;
    }
    else if (2 == g_u8GestureMode)
    {
        *pi16DeltaXP += 40;
        g_u8GestureMode = 3;
    }
    else if (3 == g_u8GestureMode)
    {
        *pi16DeltaXP -= 20;
        g_u8GestureMode = 4;
    }
    else if (4 == g_u8GestureMode)
    {
        g_u8GestureMode = 0;
    }
    // "Yes" - drawing "V" character
    else if (5 == g_u8GestureMode)
    {
        *pi16DeltaXP += 100;
        *pi16DeltaYP += 100;
        g_u8GestureMode = 6;
    }
    else if (6 == g_u8GestureMode)
    {
        *pi16DeltaXP += 100;
        *pi16DeltaYP -= 100;
        g_u8GestureMode = 7;
    }
    else if (7 == g_u8GestureMode)
    {
        g_u8GestureMode = 0;
    }
}

//=============================================================================
static inline void loop(void)
{
    static int16_t i16DeltaX = 0;
    static int16_t i16DeltaY = 0;

    if (g_bAdnsEnabled)
    {
        // handle mouse X and Y position
        motion_t motion;
        *((uint8_t *)&motion) = ADNS_read_reg(REG_Motion);
        
        if (motion.LP_VALID && !motion.FAULT) // check if no fault occurred
        {
            if (motion.MOT) // if movement occurred
            {
                i16DeltaX = (uint16_t)ADNS_read_reg(REG_Delta_X_L); // registers must be read in the order: REG_Delta_X_L first, then REG_Delta_X_H
                i16DeltaX |= ((uint16_t)ADNS_read_reg(REG_Delta_X_H) << 8);
                i16DeltaY = (uint16_t)ADNS_read_reg(REG_Delta_Y_L); // registers must be read in the order: REG_Delta_Y_L first, then REG_Delta_Y_H
                i16DeltaY |= ((uint16_t)ADNS_read_reg(REG_Delta_Y_H) << 8);

#if 0 // Enable for debug purposes only. It will slow down XY movement handling
                UART_puts("motion=(");
                UART_putb(i16DeltaX>>8);
                UART_putb(i16DeltaX&0xff);
                UART_puts(",");
                UART_putb(i16DeltaY>>8);
                UART_putb(i16DeltaY&0xff);
                UART_puts(")\n");
#endif
            }
        }
        else
        {
            UART_puts("Error:motion=");
            UART_putb(*((uint8_t *)&motion));
            UART_puts("\n");
        }
    }
#ifdef DEMO_MODE
    if (DEMO_MODE_ENABLED)
    {
        playDemo(&i16DeltaX, &i16DeltaY);
    }
#endif
    handleMouseButtons(&i16DeltaX, &i16DeltaY);
    
    // ADNS-9800 coordinates are DeltaX>0 when moving Left, DeltaY>0 when moving Up,
    // Amiga coordinates are DeltaX>0 when moving Right, DeltaY>0 when moving Down,
    // so both coordinates need to be reversed.
    while (((i16DeltaX != 0) || (i16DeltaY != 0)))
    {
        static uint8_t u8HorPhase = 0;
        static uint8_t u8VerPhase = 0;
        if (i16DeltaX > 0) // move +1 step in X direction
        {
            i16DeltaX--;
            u8HorPhase = (u8HorPhase + 1) & 0x03;
        }
        if (i16DeltaX < 0) // move -1 step in X direction
        {
            i16DeltaX++;
            u8HorPhase = (u8HorPhase + 3) & 0x03;
        }
        if (i16DeltaY > 0) // move +1 step in Y direction
        {
            i16DeltaY--;
            u8VerPhase = (u8VerPhase + 1) & 0x03;
        }
        if (i16DeltaY < 0) // move -1 step in Y direction
        {
            i16DeltaY++;
            u8VerPhase = (u8VerPhase + 3) & 0x03;
        }
        SetQuadraturePhases(u8HorPhase, u8VerPhase);
    }
}

//=============================================================================
void main(void)
{
    setup();
    while (1)
    {
        loop();
    }
}

//=============================================================================
