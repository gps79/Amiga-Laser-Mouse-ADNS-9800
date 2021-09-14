@echo off
rem sdcc -mpic16 --use-non-free main.c libio18f23k22.lib
rem sdcc -V -p18f23k22 -mpic16 -Ijhg --use-non-free -L"C:\Program Files\SDCC\bin\..\lib\small" main.c libdev18f23k22.lib libc18f.lib
make
pause