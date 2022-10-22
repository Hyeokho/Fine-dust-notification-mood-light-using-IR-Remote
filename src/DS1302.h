/*
  CodeVisionAVR C Compiler
  (C) 1998-2000 Pavel Haiduc, HP InfoTech S.R.L.

  Prototypes for the Dallas Semiconductors
  DS1302 Real Time Clock functions

  BEFORE #include -ING THIS FILE YOU
  MUST DECLARE THE I/O ADDRESS OF THE
  DATA REGISTER OF THE PORT AT WHICH
  DS1302 IS CONNECTED AND
  THE DATA BITS USED FOR IO, SCLK and RST pins 

  EXAMPLE FOR PORTB:
    
  	#asm
        .equ __ds1302_port=0x18
        .equ __ds1302_io=2
        .equ __ds1302_sclk=1
        .equ __ds1302_rst=4
    #endasm
    #include <ds1302.h>
*/

#ifndef _DS1302_INCLUDED_
#define _DS1302_INCLUDED_

#pragma used+

unsigned char ds1302_read(unsigned char addr);
void ds1302_write(unsigned char addr,unsigned char data);
void rtc_init(unsigned char tc_on,unsigned char diodes,unsigned char res);
void rtc_get_time(unsigned char *hour,unsigned char *min,unsigned char *sec);
void rtc_set_time(unsigned char hour,unsigned char min,unsigned char sec);
void rtc_get_date(unsigned char *date,unsigned char *month,unsigned char *year);
void rtc_set_date(unsigned char date,unsigned char month,unsigned char year);

#pragma used-
#pragma library ds1302.lib

#endif

