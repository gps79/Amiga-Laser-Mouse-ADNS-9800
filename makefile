#=============================================================================
# MIT License
# 
# Copyright (c) 2021 Grzegorz Pietrusiak
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# 
# Project name: Amiga Laser Mouse ADNS-9800
# Project description: based on PIC18f23k22 microcontroller and ADNS-9800 Laser Gaming Sensor
# PCB for the project: https://www.pcbway.com/project/shareproject/Amiga_Laser_Mouse.html
# Toolchain: SDCC 3.9.0-rc1
# 	gputils-1.5.0-1
#
# Author: Grzegorz Pietrusiak
# Email: gpsspam2@gmail.com
#
#=============================================================================
PROJECT_NAME = amiga_mouse
FAMILY = pic16
PROC    = 18f23k22
#PROC    = 18f24k22
HEXFILE = "$(PROJECT_NAME).hex"
PATHSRC = .
#-----------------------------------------------------------------------------
SDCCDIR = "c:\Program Files\SDCC"
CC = $(SDCCDIR)/bin/sdcc
BINEX = d:/tools/binex/BINEX.EXE
#-----------------------------------------------------------------------------
SRC = eeprom.c uart.c adns9800.c spi.c
OBJS = $(SRC:.c=.o)
#-----------------------------------------------------------------------------
#CRT = 
#CRT = --use-crt=crt0.o
CRT = --use-crt=crt0i.o
#CRT = --use-crt=crt0iz.o
#-----------------------------------------------------------------------------
COPT  = -p$(PROC) -m$(FAMILY) --use-non-free $(CRT)
COPTD = --opt-code-size --optimize-df --obanksel=2
#-----------------------------------------------------------------------------
CFLAGS = $(COPT) $(COPTD)
LDFLAGS = -Wl,-O2,--map
#-----------------------------------------------------------------------------
all: $(HEXFILE)

# Warning!
# A project containing multiple .c files must compile the file containing main() 
# function in the last step.
# Ref.: SDCC Compiler User Guide 3.9.0 -> 3.2.3 Projects with Multiple Source Files
$(HEXFILE): $(OBJS) mouse.c $(PATHSRC)/*.h
	rm -f $(HEXFILE) $(PROJECT_NAME).cod $(PROJECT_NAME).asm $(PROJECT_NAME).lst
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROJECT_NAME) mouse.c $(OBJS)
	@echo "Replacing fusebits in the HEX file to match the format acceptable by MicroBrn flasher"
	@echo "Fusebits set:"
	@echo "300000h CONFIG1L 00 - not configurable"
	@echo "300001h CONFIG1H 29 PRICLKEN, FOSC=1001 - Internal RC oscillator, CLKOUT function on OSC2"
	@echo "300002h CONFIG2L 07 BOREN=11 - Brown-out Reset enabled in hardware only, BORV=00 - VBOR set to 2.85V"
	@echo "300003h CONFIG2H 3c WDTEN=OFF"
	@echo "300004h CONFIG3L 00 - not configurable"
	@echo "300005h CONFIG3H BF default"
	@echo "300006h CONFIG4L 85 XINST=OFF, LVP=ON, STVREN=ON"
	@echo "300007h CONFIG4H 00 - not configurable"
	@echo "300008h CONFIG5L 03 memory not protected"
	@echo "300009h CONFIG5H C0 default"
	@echo "30000Ah CONFIG6L 03 memory not protected"
	@echo "30000Bh CONFIG6H E0 default"
	@echo "30000Ch CONFIG7L 03 memory not protected"
	@echo "30000Dh CONFIG7H 40 default"
	@sed -i 's/:010006008574/:0E0000000029073C00BF850003C003E0034059/g' $(HEXFILE)
	@$(BINEX) /V $(HEXFILE) 2>/dev/null |tail -n 4

%.o: $(PATHSRC)/%.c $(PATHSRC)/*.h
	$(CC)  $(CFLAGS) -c $<

cleanall:
	rm -f *.cod
	rm -f *.hex
	rm -f *.asm
	rm -f *.lst
	rm -f *.map
	rm -f *.o

clean:
	rm -f *.asm
	rm -f *.lst
	rm -f *.map
	rm -f *.o

.PHONY: all clean cleanall
