/*****************************************************************************/
/************************       rtc.h  ***************************************/
/*****************************************************************************/

#include <mega128.h>
#include <bcd.h>
#include <delay.h>



#define SCLK PORTC.7                  //rtc clk pin
#define RST  PORTC.0                  //rtc rst pin
#define IO   PORTC.6                  //rtc out pin
#define IN_IO PINC.6                  //rtc in  pin



void ds1302_write(unsigned char,unsigned char);
unsigned char ds1302_read(unsigned char);
void rtc_set_time();
void rtc_get_time();


int rtc_day;
char rtc[8]; //년월일시분초요
char *day[8] = {" "," MON", " TUE", " WED", "THUR", " FRI", " SAT", " SUN"};

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


 

void rtc_set_time()
{
    ds1302_write(0x8e,0);       //쓰기 잠금 장치 off

    ds1302_write(0x8c,rtc[0]);  //rtc_year write
    
    ds1302_write(0x8a,rtc[6]);  //rtc_day write 월화수목금토일 1,2,3,4,5,6,7,

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
    
    rtc[6]=bcd2bin(ds1302_read(0x8b));       //day bcd 코드 데이터를 10진수 데이터 rtc[6] 배열로 변환
}