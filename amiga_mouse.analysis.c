void main()
{
    OSCCONbits.SCS = 3; // internal oscillator
    OSCCONbits.IRCF = 7; // 7 - 16MHz, 6 - 8MHz, 5 - 4MHz
    ADCON1 = 0x0F; //Disable all analog inputs
    ANSELA = 0; //Disable all analog inputs on port A
    ANSELB = 0; //Disable all analog inputs on port B
    CONFIG_PORT_DIRECTION = INPUT;
    INTCON2bits.RBPU = LOW; // RBPU is negated, so zero enable it. serve as general key for all PORTB pull-ups resistors    
    CONFIG_PORT_PULLUP = HIGH;
    LMB_IN_PORT_DIRECTION = INPUT;
    RMB_IN_PORT_DIRECTION = INPUT;
    LMB_OUT_PORT_DIRECTION = OUTPUT;
    RMB_OUT_PORT_DIRECTION = OUTPUT;
    LMB_OUT = HIGH; // button not pressed
    RMB_OUT = HIGH; // button not pressed
    TRISC=0x00; //  Set all of PORTC as outputs. TX and RX must be set as outputs first
    if ((LOW == LMB_IN) && (LOW == RMB_IN))
    g_bDemoEnabled = true;
    UART_PORT_DIRECTION = OUTPUT;
    UART = HIGH;
    UART_puts("Amiga Mouse driver 1.0 by gps79\n");
    H_PORT_DIRECTION = OUTPUT;
    V_PORT_DIRECTION = OUTPUT;
    HQ_PORT_DIRECTION = OUTPUT;
    VQ_PORT_DIRECTION = OUTPUT;
    SPI_begin();
    NCS = LOW;
    NCS = HIGH;
    ADNS_write_reg(REG_Power_Up_Reset, 0x5a); // force reset
    DELAY_MS(50); // 50ms power up time
    (void)ADNS_read_reg(REG_Motion);
    (void)ADNS_read_reg(REG_Delta_X_L);
    (void)ADNS_read_reg(REG_Delta_X_H);
    (void)ADNS_read_reg(REG_Delta_Y_L);
    (void)ADNS_read_reg(REG_Delta_Y_H);
    UART_puts("ADNS9800 Uploading firmware...\n");
    ADNS_upload_firmware();
    uint8_t u8ProductId = ADNS_read_reg(REG_Product_ID);
    if (ADNS_SUPPORTED_PRODUCT_ID == u8ProductId)
    if (0xff == (ADNS_read_reg(REG_Inverse_Product_ID) ^ u8ProductId))
    ADNS_write_reg(REG_SROM_Enable, 0x15); 
    DELAY_MS(10);
    uint8_t u8CrcLow = ADNS_read_reg(REG_Data_Out_Lower);
    uint8_t u8CrcHigh = ADNS_read_reg(REG_Data_Out_Upper);
    if ((0xEF == u8CrcLow) && (0xBE == u8CrcHigh))
    g_bAdnsEnabled = true;
    uint8_t u8LaserDriveMode = ADNS_read_reg(REG_LASER_CTRL0);
    ADNS_write_reg(REG_LASER_CTRL0, u8LaserDriveMode & 0xf0 );
    g_u8Resolution = EE_read_byte(EE_CALIB_RESOLUTION_ADDR);
    if ((g_u8Resolution < 0x01) || (g_u8Resolution > 0xA4))
    UART_puts("Invalid calibration value in EEPROM 0x");
    UART_putb(g_u8Resolution);
    UART_puts(". saving default value 0x44\n");
    g_u8Resolution = 0x44; // setting default resolution
    if (!EE_write_byte(EE_CALIB_RESOLUTION_ADDR, g_u8Resolution))
    UART_puts("Can't store calibration value in EEPROM.\n");
    ADNS_write_reg(REG_Configuration_I, g_u8Resolution);
    UART_puts("Optical Chip Initialized\n");
    UART_puts("SROM CRC error:0x");
    UART_putb(u8CrcHigh);
    UART_putb(u8CrcLow);
    UART_puts("\n");
    UART_puts("Product ID test failed\n");
    UART_puts("Invalid Product ID:");
    UART_putb(u8ProductId);
    UART_puts("\n");
    ADNS_uart_print_register(REG_Product_ID, "Product ID");
    ADNS_uart_print_register(REG_Revision_ID, "Revision ID");
    ADNS_uart_print_register(REG_Motion, "Motion");
    ADNS_uart_print_register(REG_Observation, "Observation");
    ADNS_uart_print_register(REG_SROM_ID, "SROM ID");
    ADNS_uart_print_register(REG_Configuration_I, "Config I");
    ADNS_uart_print_register(REG_Configuration_II, "Config II");
    ADNS_uart_print_register(REG_Configuration_IV, "Config IV");
    ADNS_uart_print_register(REG_Configuration_V, "Config V");
    while(1)
    {
        if (false != g_bAdnsEnabled)
        *((uint8_t *)&motion) = ADNS_read_reg(REG_Motion);
        if (motion.MOT && motion.LP_VALID && !motion.FAULT)
        i16DeltaX = (uint16_t)ADNS_read_reg(REG_Delta_X_L);
        i16DeltaX |= ((uint16_t)ADNS_read_reg(REG_Delta_X_H) << 8);
        i16DeltaY = (uint16_t)ADNS_read_reg(REG_Delta_Y_L);
        i16DeltaY |= ((uint16_t)ADNS_read_reg(REG_Delta_Y_H) << 8);
        UART_puts("Error:motion=");
        UART_putb(*((uint8_t *)&motion));
        UART_puts("\n");
        if (false != g_bDemoEnabled)
        playDemo(&i16DeltaX, &i16DeltaY);
        if (0 == u8Phase) (*pi16DeltaXP)++;
        if (1 == u8Phase) (*pi16DeltaXP)++;
        if (2 == u8Phase) (*pi16DeltaYP)--;
        if (3 == u8Phase) (*pi16DeltaYP)--;
        DELAY_MS(10);
        u8Step++;
        if (u8Step >= 100)
        u8Step = 0;
        u8Phase = (u8Phase + 1) & 0x07;
        while (((i16DeltaX != 0) || (i16DeltaY != 0)))
        if (i16DeltaX > 0) // move +1 step in X direction
        i16DeltaX--;
        u8HorPhase = (u8HorPhase + 1) & 0x03;
        if (i16DeltaX < 0) // move -1 step in X direction
        i16DeltaX++;
        u8HorPhase = (u8HorPhase + 3) & 0x03;
        if (i16DeltaY > 0) // move +1 step in Y direction
        i16DeltaY--;
        u8VerPhase = (u8VerPhase + 1) & 0x03;
        if (i16DeltaY < 0) // move -1 step in Y direction
        i16DeltaY++;
        u8VerPhase = (u8VerPhase + 3) & 0x03;
        SetQuadraturePhases(u8HorPhase, u8VerPhase);
        if (0 == u8HorPhaseP)         { H = LOW;  HQ = LOW;  }
        else if (1 == u8HorPhaseP)    { H = HIGH; HQ = LOW;  }
        else if (2 == u8HorPhaseP)    { H = HIGH; HQ = HIGH; }
        else if (3 == u8HorPhaseP)    { H = LOW;  HQ = HIGH; }
        if (0 == u8VerPhaseP)         { V = LOW;  VQ = LOW;  }
        else if (1 == u8VerPhaseP)    { V = HIGH; VQ = LOW;  }
        else if (2 == u8VerPhaseP)    { V = HIGH; VQ = HIGH; }
        else if (3 == u8VerPhaseP)    { V = LOW;  VQ = HIGH; }
        delay_us(150);
        SetQuadraturePhases(u8HorPhase, u8VerPhase);
        if (CFG && (false != g_bAdnsEnabled))
        bool bApplyNewResolution = false;
        if (!bWaitForButtonsRelease)
        if (LOW == LMB_IN)
        if (g_u8Resolution < 0xA4)
        g_u8Resolution++;
        bApplyNewResolution = true;
        bWaitForButtonsRelease = true;
        if (LOW == RMB_IN)
        if (g_u8Resolution > 0x01)
        g_u8Resolution--;
        bApplyNewResolution = true;
        bWaitForButtonsRelease = true;
        if ((HIGH == RMB_IN) && (HIGH == LMB_IN))
        bWaitForButtonsRelease = false;
        if (bApplyNewResolution)
        UART_puts("New XY resolution:");
        UART_putb(g_u8Resolution);
        UART_puts("\n");
        ADNS_write_reg(REG_Configuration_I, g_u8Resolution);
        if (!EE_write_byte(EE_CALIB_RESOLUTION_ADDR, g_u8Resolution))
        UART_puts("Can't store calibration value in EEPROM.\n");
        DELAY_MS(50); // wait 50ms as a primitive buttons debouncing
        if (HIGH == LMB_IN)
            LMB_OUT = HIGH; // Left Mouse Button not pressed
            LMB_OUT = LOW; // Left Mouse Button pressed
        if (HIGH == RMB_IN) 
            RMB_OUT = HIGH; // Right Mouse Button not pressed
            RMB_OUT = LOW; // Right Mouse Button pressed
    }
}

void ADNS_uart_print_register(uint8_t u8RegIdP, char * szRegNameP)
    uint8_t u8RegValue = ADNS_read_reg(u8RegIdP);
    UART_puts(szRegNameP);
    UART_puts(": 0x");
    UART_putb(u8RegValue);
    UART_puts("\n");
}

void delay_us(int16_t i16MicrosecondsP) // "i16MicrosecondsP" must be >= 10
    i16MicrosecondsP -= 10;
    while (i16MicrosecondsP > 0) // 1 loop lap = 4us
    i16MicrosecondsP -= 4;
}
