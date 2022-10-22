#include <mega128.h>
#include <stdio.h>
#include <stdio.h>
#include <delay.h>
#include <bcd.h>
#include "lcd.h"

 

#define SCLK PORTC.7                  //rtc clk pin
#define RST  PORTC.0                  //rtc rst pin
#define IO   PORTC.6                  //rtc out pin
#define IN_IO PINC.6                  //rtc in  pin


char Message[40] = "time = "; // LCD 화면에 문자열 출력을 위한 문자열 변수 

char rtc[8]; //년월일시분초
 
void ds1302_write(unsigned char add,unsigned char data)
{     //1302 write
    unsigned char i=0;
    
    RST = 1;  // 1302(활성화)
 
    // 어드레스 출력 (add)
    for(i=0;i<8;i++)
    { // 8번 루프 돌면서 1비트+8=1바이트를 출력
        if((add >> i)&0X01)
        {
            IO = 1 ;  // 비트가 1이면 1출력
        }
        else 
        {
            IO = 0 ;                 // 비트가 0이면 0출력
        }
        
        delay_us(1); //딜레이
        
        SCLK = 1 ;   //클럭을 1로    

        delay_us(1); //딜레이    

        SCLK = 0;    //클럭을 0로
        
        delay_us(1); //딜레이
    }


 

    //데이터출력 (data)
    for(i=0;i<8;i++)
    {  // 8번 루프 돌면서 1비트+8=1바이트를 출력                   
        if((data >> i)&0X01)
        {
            IO = 1 ;  // 비트가 1이면 1출력
        }
        else
        {
            IO = 0 ;                  // 비트가 0이면 0출력
        }
        
        delay_us(1);                   //딜레이
        
        SCLK = 1 ;                     //클럭을 1로
        
        delay_us(1);                   //딜레이
        
        SCLK = 0;                      //클럭을 0로
        
        delay_us(1);                   //딜레이
    }
    
    RST = 0;                           //1302 비활성화

}


 
unsigned char ds1302_read(unsigned char add)
{    //1302 read
    unsigned char i=0;
    
    unsigned char data=0;
    
    RST = 1;  // 1302(활성화)
    
    for(i=0;i<8;i++)
    { // 8번 루프 돌면서 1바이트를 출력   
        if(add & 0x01)
        {
            IO = 1 ;  // 비트가 1이면 1출력
        }
        else
        {
            IO = 0 ;            // 비트가 0이면 0출력
        }
        
        delay_us(1); //딜레이
        
        SCLK = 1 ;   //클럭을 1로
        
        delay_us(1); //딜레이
        
        SCLK = 0;    //클럭을 0으로
        
        delay_us(1); //딜레이
        
        add >>= 1 ; // 어드레스를 1비트씩 우로 시프트
    }
    
    DDRC.6=0; // IO 입력으로 변경
    
    IO=1;     // rtc input
    
    for(i=0;i<8;i++)
    {      // 8번 루프 돌면서 1바이트를 출력
        data>>=1;          //데이터 쉬프트
        
        delay_us(1);       //딜레이     
        
        if(IN_IO)
        {
            data|=0x80;   //입력이 1일 경우
        }
        
        delay_us(1);       //딜레이
        
        SCLK = 1;          //클럭을 1으로
        
        delay_us(1);       //딜레이
        
        SCLK = 0 ;         //클럭을 0으로
        
        delay_us(1);       //딜레이
    }

    DDRC.6=1;        //rtc io pin 출력 설정
        
    RST=0;           //1302 비활성화
    
    return data;       //리턴 데이터
}


 

void rtc_set_time(void)
{
    ds1302_write(0x8e,0);       //쓰기 잠금 장치 off

    ds1302_write(0x8c,rtc[0]);  //rtc_year write

    ds1302_write(0x88,rtc[1]);  //rtc_month write

    ds1302_write(0x86,rtc[2]);  //rtc_date write

    ds1302_write(0x84,rtc[3]);  //rtc_hour write

    ds1302_write(0x82,rtc[4]);  //rtc min write

    ds1302_write(0x80,rtc[5]);  //rtc sec write

    ds1302_write(0x8e,0x80);    //쓰기 잠금 장치 on
}


 

 
void rtc_get_time()
{
    rtc[0]=bcd2bin(ds1302_read(0x8d));       //year bcd 코드 데이터를 10진수 데이터 rtc[0] 배열로 변환 

    rtc[1]=bcd2bin(ds1302_read(0x89));       //month bcd 코드 테이터를 10진수 데이터 rtc[1] 배열로 변환

    rtc[2]=bcd2bin(ds1302_read(0x87));       //date bcd 코드 데이터를 10진수 데이터 rtc[2] 배열로 변환

    rtc[3]=bcd2bin(ds1302_read(0x85));       //hour bcd 코드 데이터를 10진수 데이터 rtc[3] 배열로 변환

    rtc[4]=bcd2bin(ds1302_read(0x83));       //min bcd 코드 데이터를 10진수 데이터 rtc[4] 배열로 변환

    rtc[5]=bcd2bin(ds1302_read(0x81));       //sec bcd 코드 데이터를 10진수 데이터 rtc[5] 배열로 변환
}


 
void main(void)
{
    DDRC=0Xff;
    DDRB=0xff;

 
    UCSR0A=0x00;
    UCSR0B=0x98;                        //송신,수신 허용
    UCSR0C=0x06;                        //비동기 통신 모드, 데이터 사이즈 8bit
    UBRR0H=0x00;
    UBRR0L=0x67;                        //boud rate : 9600 pbs
    
    ACSR=0x80;
    SFIOR=0x00;


 
    rtc[0]=0x10;                        //year 초기값
    rtc[1]=0x09;                        //month 초기값
    rtc[2]=0x09;                        //date 초기값
    rtc[3]=0x15;                        //hour 초기값
    rtc[4]=0x59;                        //min 초기값
    rtc[5]=0x55;                        //sec 초기값
 
    //rtc_set_time();
    LCD_Init();

    #asm("sei")


 
    while (1)
    {
        rtc_get_time();     //RTC 함수 호출
        
        //USRT 통신을 이용하여 데이터 확인 (년 월 일 시간 분 초)
        printf("%d\r\t%d\r\t%d\r\t%d\r\t%d\r\t%d\r\n",rtc[0],rtc[1],rtc[2],rtc[3],rtc[4],rtc[5]);
        
        sprintf(Message,"%d %d %d %d %.2d %.2d",rtc[0],rtc[1],rtc[2],rtc[3],rtc[4],rtc[5]);
        
        LCD_Pos(0,0);
        LCD_Clear();
        LCD_Str(Message);
              
        PORTB=rtc[5];
      };

}
