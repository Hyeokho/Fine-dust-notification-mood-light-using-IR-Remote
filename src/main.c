/*
 * 센서응용시스템 - 기말 프로젝트
 * IR Remote를 이용한 미세먼지 무드등
 * 임베디드 시스템 전공
 * 2013146005
 * 권혁호
 *
 * 제출일 18/12/04
*/ 

#include <mega128.h>
#include <delay.h>
#include <stdio.h>
#include "rtc.h"    // RTC 관련 함수 정의
#include "lcd.h"    // LCD 관련 함수 정의
#include "dust.h"   // 먼지센서 관련 함수 정의


#define UP 0x06    // 리모컨의 "UP" 버튼 값 정의
#define DOWN 0x86  // 리모컨의 "DOWN" 버튼 값 정의
#define MENU 0xD2  // 리모컨의 "MENU" 버튼 값 정의
#define SEL 0x16   // 리모컨의 "SELECT" 버튼 값 정의


// 초기화 함수
void system_init(void);
void RTC_Init(void);
void PORT_set(void);
interrupt [TIM0_COMP] void timer_comp0();


// LED 및 LCD 출력 관련 함
void Home(void);
void Home_Light(void);
void Mood_Light(int);


// 메뉴 출력 관련 함수
void Sensor_Project_Menu(void);
void Light_Setting(void);
void Light_Setting2(void);
void Dust_Setting(void);
void Dust_Setting2(void);
void Dust_Good_Light(void);
void Dust_Good_Light2(void);
void Dust_Fine_Light(void);
void Dust_Fine_Light2(void);
void Dust_Bad_Light(void);
void Dust_Bad_Light2(void);


char Message[40]; // LCD 화면에 문자열 출력을 위한 문자열 변수
char *dust_step2[20] = {" BAD", "FINE", "GOOD"}; // 미세먼지 단계를 출력할 배열 선언


// 리모컨 제어 관련 전역변수 선언
unsigned int time_count = 0;  // 타이머/카운터0 출력비교 변수 
unsigned int edge=0; // "time_count" 값을 임시저장 하여 수신한 비트가 1인지 0인지 판단 
unsigned char flag = 0; // 4바이트의 리모컨 입력 데이터를 1바이트 씩 나누어 받을건데 이때 1바이트 전부 받기위한 인덱스 변수 
unsigned char flag_count = 0; // 리모컨으로부터 1바이트 입력 받으면 4까지 1씩 증가 
unsigned char IR_start=0; // 1이면 : 리드코드 수신 확인 후 본격적인 데이터 수신 시작, 0이면 : 리드코드 미 수신 
unsigned char IR_rx_data[4] = { 0x00, 0x00, 0x00, 0x00 }; // 4바이트의 데이터를 저장할 배열 선언
unsigned char IR_data; // 리모컨으로 부터 수신한 데이터 저장
unsigned long mask=0x80; // 0 또는 1을 마스크 하기위한 변수


// 기본 메뉴에서 사용할 커서 값 선언, 기본 메뉴는 1페이지이기 때문에 3까지 증가하는 변수
int Cursor_val = 1;

// 세부 동작 메뉴에서 사용할 커서 값 선언, 세부 동작 메뉴는 2페이지이기 때문에 7까지 증가하는 변수
int Cursor_8 = 0;


// 평소에 켜져있는 LED 설정, 1 = RED, 2 = GREEN, 3 = BLUE, 4 = YELLOW, 5 = LED OFF
int HOME_LED = 4; // 평소 켜져잇는 LED는 노란색
int GOOD_LED = 3; // "GOOD" 단계로 미세먼지 단계가 바뀌면 파란색 LED 점멸
int FINE_LED = 2; // "FINE" 단계로 미세먼지 단계가 바뀌면 초록색 LED 점멸
int BAD_LED = 1;  // "BAD" 단계로 미세먼지 단계가 바뀌면 빨간색  LED 점멸
int LED_OFF = 5;  // LED를 OFF 시키는 값 정의 



// 포트 세팅은 각 초기화함수에 되어 있지만 한 눈에보기 좋게 정의
void PORT_set()
{
    // RTC 및 적외선 출력
    DDRC=0xff;
    
    // LCD 출력
    DDRA  = 0xFF;        
    DDRG |= 0x0F;
    
    // 릴레이, LED 제어 PORTD 상위 3비트만 출력
    DDRD = 0xE0;
    
    // IR receiver 입력
    DDRE.4 = 0;             
}



void main(void)
{
    DDRE = 0x00;    // IR Rx Pin (리모컨 수신 핀 PE4)
    DDRD = 0xE0;    // 릴레이, LED 제어 포트 상위 3비트만 출력
      
    LCD_Init(); // LCD 초기화
    ADC_Init(); // ADC 초기화 
    RTC_Init(); // RTC 초기화
    
    system_init();// 리모컨 초기화
 
                           
    PORTD =0xC0; // 처음 켜지면 LED ON (YELLOW)
 
    
    Home_Light(); // 시간 및 미세먼지 출력
    
    
    while(1)
    {   
       
       Home(); // 기본 화면 출력
       delay_ms(200);
        
       // "MENU" 버튼이 눌리면
        if(IR_data == MENU)
        {   
            LCD_Clear(); // LCD 초기화
            Sensor_Project_Menu();  // 메뉴 출력
            
            while(1)
            {   
                LCD_Pos2(Cursor_val, 0); // 커서의 위치를 조정함으로써 현재 사용자가 어느 항목에 있는지 알 수음 
                blink_OnOff(1);          // 블링크 ON
                
                // 기본 메뉴에서 1번(Home)을 선택하면 기본화면 출력 
                if(Cursor_val == 1 && IR_data == SEL )
                {
                    LCD_Clear();
                    break;
                }
                
                // 2번 Light Setting 선택
                if(Cursor_val == 2 && IR_data == SEL )
                {
                   LCD_Clear();   
                   Light_Setting(); // 메뉴 출력
                   
                   IR_data = 0x00; // 선텍값 초기화
                   Cursor_8 = 1;   // 커서값 초기화
                   
                   while(1)
                   {    
                        // 세부 메뉴가 2페이지 이상이기 때문에 7까지 증가하는 커서 변수 이용
                        LCD_Pos2(Cursor_8, 0);
                       
                        // 평소에 켜지는 조명 RED 설정 후 이전 메뉴로 복귀
                        if(Cursor_8 == 1 && IR_data == SEL )
                        {
                            HOME_LED = 1; // 1이면 RED, 2이면 GREEN, ... 5까지
                            PORTD=0xE0;   // LED RED ON 
                            IR_data = 0x00; // 선택값 초기화
                            Sensor_Project_Menu();
                            break;
                        }
                        
                        // 평소에 켜지는 조명 GREEN 설정 후 이전 메뉴로 복귀
                        if(Cursor_8 == 2 && IR_data == SEL )
                        {
                            HOME_LED = 2;
                            PORTD=0x80;                            
                            IR_data = 0x00;
                            Sensor_Project_Menu();
                            break;
                        }
                        
                        // 평소에 켜지는 조명 BLUE 설정 후 이전 메뉴로 복귀
                        if(Cursor_8 == 3 && IR_data == SEL )
                        {
                            HOME_LED = 3;
                            PORTD=0x40;
                            IR_data = 0x00;
                            Sensor_Project_Menu();
                            break;
                        }
                        
                        // 커서의 값이 4이상이면 2페이지로 이동
                        if( Cursor_8 >= 4)
                        {  
                            LCD_Clear();
                            Light_Setting2();
                            
                            while(1)
                            {   
                                // 커서값 -3 을 해주어야 의도한 위치로 커서 이동
                                LCD_Pos2(Cursor_8-3, 0);
                                
                                // 커서값이 4 미만이면 1페이지로 이동
                                if(Cursor_8 < 4 )
                                {
                                    LCD_Clear();   
                                    Light_Setting();
                                    break;
                                }
                                
                                // 평소에 켜지는 조명 YELLOW 설정
                                if(Cursor_8 == 4 && IR_data == SEL )
                                {
                                    HOME_LED = 4;
                                    PORTD=0xA0;                                    
                                    break;
                                }
                                
                                // 평소에 조명이 켜지지 않도록 설정
                                if(Cursor_8 == 5 && IR_data == SEL )
                                {
                                    HOME_LED = 5;                                    
                                    PORTD =0xC0 ;
                                    break;
                                }
                                
                                // EXIT, 메뉴 탈출
                                if(Cursor_8 == 6 && IR_data == SEL )
                                {
                                    break;
                                }           
                            }
                            

                        }
                        
                        // 2페이지에서 세팅을 마쳤을 때 선택값 초기화 후 메인메뉴 출력
                        if(Cursor_8 >= 4 && IR_data == SEL )
                        {
                            IR_data = 0x00;
                            LCD_Clear();
                            Sensor_Project_Menu();
                            break;
                        }       
                        
                        
                   } 
                }
                
                // 3번 Dust Setting 선택
                if(Cursor_val == 3 && IR_data == SEL )
                {   
                   LCD_Clear();   
                   Dust_Setting(); // Dust_Setting 메뉴 출력
                   
                   IR_data = 0x00; //선텍값  초기화
                   Cursor_8 = 1;   // 커서값 초기화
                   
                   while(1)
                   {    

                        LCD_Pos2(Cursor_8, 0);
                       
                        // GOOD 단계에서 켜질 LED 세팅
                        if(Cursor_8 == 1 && IR_data == SEL )
                        {
                            LCD_Clear();   
                            Dust_Good_Light(); // GOOD 단계에서 켜질 LED 선택할 메뉴 출력
                   
                            IR_data = 0x00; //선텍값  초기화
                            Cursor_8 = 1;   // 커서값 초기화
                            
                            while(1)
                            {
                                LCD_Pos2(Cursor_8, 0);
                                
                                // GOOD 단계가 되면 RED 켜지게 세팅 후 이전 메뉴로 복귀
                                if(Cursor_8 == 1 && IR_data == SEL)
                                {
                                    GOOD_LED = 1;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                } 
                                // GOOD 단계가 되면 GREEN 켜지게 세팅 후 이전 메뉴로 복귀
                                if(Cursor_8 == 2 && IR_data == SEL)
                                {
                                    GOOD_LED = 2;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                // GOOD 단계가 되면 BLUE 켜지게 세팅 후 이전 메뉴로 복귀
                                if(Cursor_8 == 3 && IR_data == SEL)
                                {
                                    GOOD_LED = 3;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                
                                // 커서의 값이 4이상이면 2페이지로 이동 
                                if( Cursor_8 >= 4)
                                {  
                                    LCD_Clear();
                                    Dust_Good_Light2(); // GOOD 단계에서 켜질 LED 선택할 메뉴 2페이지 출력
                            
                                    while(1)
                                    {   
                                        // 커서값 -3 을 해주어야 의도한 위치로 커서 이동
                                        LCD_Pos2(Cursor_8-3, 0);
                                        
                                        // 커서값이 4 미만이면 1페이지로 이동
                                        if(Cursor_8 < 4 )
                                        {
                                            LCD_Clear();   
                                            Dust_Good_Light();
                                            break;
                                        }
                                        
                                        // GOOD단계가 되면 YELLOW 켜짐 
                                        if(Cursor_8 == 4 && IR_data == SEL )
                                        {
                                            GOOD_LED = 4;                                    
                                            break;
                                        }
                                        
                                        // GOOD 단계가 되어도 LED 켜지지 않음
                                        if(Cursor_8 == 5 && IR_data == SEL )
                                        {
                                            GOOD_LED = 5;                                    
                                            break;
                                        }
                                        
                                        // EXIT, 메뉴 탈출
                                        if(Cursor_8 == 6 && IR_data == SEL )
                                        {
                                            break;
                                        }
                                    }
    
                                }
                                
                                // 2페이지에서 세팅을 마쳤을 때 선택값 초기화 후 이전 메뉴 출력
                                if(Cursor_8 >= 4 && IR_data == SEL )
                                {
                                    Cursor_8=1;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;
                                }                                                 
                            }
                        }

                        // FINE 단계에서 켜질 LED 세팅
                        if(Cursor_8 == 2 && IR_data == SEL )
                        {
                            LCD_Clear();   
                            Dust_Fine_Light(); // FINE 단계에서 켜질 LED 선택할 메뉴 출력
                   
                            IR_data = 0x00; // 선텍값 초기화
                            Cursor_8 = 1;   // 커서값 초기화
                            
                            while(1)
                            {
                                LCD_Pos2(Cursor_8, 0);
                                
                                // FINE 단계가 되면 RED 켜지게 세팅 후 이전 메뉴로 복귀
                                if(Cursor_8 == 1 && IR_data == SEL)
                                {
                                    FINE_LED = 1;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                
                                // FINE 단계가 되면 GREEN 켜지게 세팅 후 이전 메뉴로 복귀
                                if(Cursor_8 == 2 && IR_data == SEL)
                                {
                                    FINE_LED = 2;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                
                                // FINE 단계가 되면 BLUE 켜지게 세팅 후 이전 메뉴로 복귀
                                if(Cursor_8 == 3 && IR_data == SEL)
                                {
                                    FINE_LED = 3;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                
                                // 커서의 값이 4이상이면 2페이지로 이동 
                                if( Cursor_8 >= 4)
                                {  
                                    LCD_Clear();
                                    Dust_Fine_Light2(); // FINE 단계에서 켜질 LED 선택할 메뉴 2페이지 출력
                            
                                    while(1)
                                    {
                                        // 커서값 -3 을 해주어야 의도한 위치로 커서 이동
                                        LCD_Pos2(Cursor_8-3, 0);
                                        
                                         // 커서값이 4 미만이면 1페이지로 이동
                                        if(Cursor_8 < 4 )
                                        {
                                            LCD_Clear();   
                                            Dust_Fine_Light();
                                            break;
                                        }
                                        
                                        // FINE 단계가 되면 YELLOW 켜짐 
                                        if(Cursor_8 == 4 && IR_data == SEL )
                                        {
                                            FINE_LED = 4;                                    
                                            break;
                                        }
                                        
                                        // FINE 단계가 되어도 LED 켜지지 않음
                                        if(Cursor_8 == 5 && IR_data == SEL )
                                        {
                                            FINE_LED = 5;                                    
                                            break;
                                        }
                                        
                                        // EXIT, 메뉴 탈출
                                        if(Cursor_8 == 6 && IR_data == SEL )
                                        {
                                            break;
                                        }
                                    }
                                } 
                                
                                // 2페이지에서 세팅을 마쳤을 때 선택값 초기화 후 이전메뉴 출력
                                if(Cursor_8 >= 4 && IR_data == SEL )
                                {
                                    Cursor_8=1;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;
                                }                                   
                                           
                            }
                            
                        }
                        
                        // BAD 단계에서 켜질 LED 세팅
                        if(Cursor_8 == 3 && IR_data == SEL )
                        {
                            LCD_Clear();   
                            Dust_Bad_Light(); // BAD 단계에서 켜질 LED 선택할 메뉴 출력
                   
                            IR_data = 0x00; // 선텍값 초기화
                            Cursor_8 = 1;   // 커서값 초기화
                            
                            while(1)
                            {
                                LCD_Pos2(Cursor_8, 0);
                                
                                // BAD 단계가 되면 RED 켜지게 세팅 후 이전 메뉴로 복귀
                                if(Cursor_8 == 1 && IR_data == SEL)
                                {
                                    BAD_LED = 1;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                
                                // BAD 단계가 되면 GREEN 켜지게 세팅 후 이전 메뉴로 복귀
                                if(Cursor_8 == 2 && IR_data == SEL)
                                {
                                    BAD_LED = 2;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                
                                // BAD 단계가 되면 BLUE 켜지게 세팅 후 이전 메뉴로 복귀
                                if(Cursor_8 == 3 && IR_data == SEL)
                                {
                                    BAD_LED = 3;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                
                                // 커서의 값이 4이상이면 2페이지로 이동
                                if( Cursor_8 >= 4)
                                {  
                                    LCD_Clear();
                                    Dust_Bad_Light2(); // BAD 단계에서 켜질 LED 선택할 메뉴 2페이지 출력
                            
                                    while(1)
                                    {    
                                        // 커서값 -3 을 해주어야 의도한 위치로 커서 이동
                                        LCD_Pos2(Cursor_8-3, 0);
                                        
                                        // 커서값이 4 미만이면 1페이지로 이동
                                        if(Cursor_8 < 4 )
                                        {
                                            LCD_Clear();   
                                            Dust_Bad_Light();
                                            break;
                                        }
                                        
                                        // BAD 단계가 되면 YELLOW 켜짐
                                        if(Cursor_8 == 4 && IR_data == SEL )
                                        {
                                            BAD_LED = 4;                                    
                                            break;
                                        }
                                        
                                        // BAD 단계가 되어도 LED 켜지지 않음
                                        if(Cursor_8 == 5 && IR_data == SEL )
                                        {
                                            BAD_LED = 5;                                    
                                            break;
                                        }
                                        
                                        // EXIT, 메뉴 탈출
                                        if(Cursor_8 == 6 && IR_data == SEL )
                                        {
                                            break;
                                        }
                                    }
                                } 
                                
                                // 2페이지에서 세팅을 마쳤을 때 선택값 초기화 후 이전메뉴 출력
                                if(Cursor_8 >= 4 && IR_data == SEL )
                                {
                                    Cursor_8=1;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;
                                }                                                                
                            }
                        }
                        
                        // Dust Setting 메뉴 2p
                        if( Cursor_8 >= 4)
                        {  
                            LCD_Clear();
                            Dust_Setting2(); // Dust Setting 메뉴 2p 출력
                            
                            while(1)
                            {    
                                // 커서값 -3 을 해주어야 의도한 위치로 커서 이동
                                LCD_Pos2(Cursor_8-3, 0);
                                
                                // 커서값이 4 미만이면 1페이지로 이동
                                if(Cursor_8 < 4 )
                                {
                                    LCD_Clear();   
                                    Dust_Setting();
                                    break;
                                }
                                
                                // GOOD, FINE, BAD 어느 단계로 바뀌어도 LED 가 켜기지 않음
                                // 기본 LED만 켜짐
                                if(Cursor_8 == 4 && IR_data == SEL )
                                {
                                    GOOD_LED = 5;
                                    FINE_LED = 5;
                                    BAD_LED = 5;                                   
                                    break;
                                }
                                
                                // EXIT, 메뉴 탈출
                                if(Cursor_8 == 5 && IR_data == SEL )
                                {
                                    break;
                                } 
                                
                                // 메뉴는 5번 까지 있기 때문에 UP 버튼이 눌리면 한칸 내려줌
                                if(Cursor_8 == 6 )
                                {
                                    Cursor_8 = Cursor_8-1;
                                }             
                            }
                        }                                                                                             
                        
                        // 2페이지에서 세팅을 마쳤을 때 선택값 초기화 후 메인 메뉴 출력
                        if(Cursor_8 >= 4 && IR_data == SEL )
                        {
                            IR_data = 0x00;
                            LCD_Clear();
                            Sensor_Project_Menu();
                            break;
                        }    
                   }                                     
                }
            }
        }
    }
}

// RTC 초기화, SCLK, RST, DAT의 포트 설정은 "rtc.h"에 정의 
void RTC_Init()
{
     DDRC=0xff;
     
     rtc[0]=0x18;                        // year, 년도  초기값
     rtc[1]=0x12;                        // month, 월  초기값
     rtc[2]=0x02;                        // date, 일 초기값
     rtc[3]=0x10;                        // hour, 시간 초기값
     rtc[4]=0x30;                        // min, 분  초기값
     rtc[5]=0x00;                        // sec, 초 초기값
     rtc[6]=0x07;                        // day, 요일  초기값
     
     //rtc_set_time();                    // 원하는 시간설정후 주석 해제 
}



// PORTE.4에 하강엣지 신호가 들어오면 인터럽트 발생
// 리모컨으로부터 신호를 입력 받으면 인터럽트
interrupt[EXT_INT4] void input(void)
{
    edge = time_count; // 가장 처음 하강엣지 신호가 들어오면 다음 하강엣지까지의
                       // 시간을 이용하여 0, 1을 판별하기 때문에 시간을 edge에 저장한다.
    
    time_count=0;      // 시간 초기화
    
    // 리드코드 인식 단계, 9.0 ms
    // 본 프로젝트에 사용한 리모컨은 리드코드가 9.0 ms의 길이를 가지기 때문에
    // 오차범위를 감안하여 설정한다.
    if(edge > 80 && edge < 100)
    {
        IR_start=1; // 리드코드가 성공적으로 수신되었으면
    }
    
    // 리드코드가 정상적으로 수신되었으면
    // 하강엣지부터 다음 하강엣지 까지의 시간에 따라서 "1", "0"을 판별한다.
    // 본 프로젝트에 사용한 리모컨은 하강엣지 부터 다음 하강엣지 까지의 시간이
    // 1.125ms 일때 "0", 2.25,s "1" 이다
    else if(IR_start==1)
    {
       
        // 하강엣지부터 다음 하강엣지의 시간이 1.125ms 이면 "0" 
        if(edge > 8 && edge < 13)
        {
            IR_rx_data[flag_count] &= ~mask; // 32비트의 가장 앞에서부터 차례대로 마스크한다.
            mask = mask >>1; // 오른쪽으로 1비트 시프트 하여 다음 비트를 받을 준비한다.
            flag++;          // flag값을 1 증가시켜서 8번째 비트까지 받는것을 체크한다.
        } 
        
        // 하강엣지부터 다음 하강엣지의 시간이 2.25ms 이면 "1" 
        else if(edge > 19 && edge < 28)//  1        2.25ms
        {
            IR_rx_data[flag_count] |= mask;  // 32비트의 가장 앞에서부터 차례대로 셀렉트한다.
            mask=mask>>1; // 오른쪽으로 1비트 시프트 하여 다음 비트를 받을 준비한다.
            flag++;       // flag값을 1 증가시켜서 8번째 비트까지 받는것을 체크한다.
        }
        
        // 4바이트 중 하나의 바이트의 데이터를 전부 받으면
        if(flag == 8)
        {  
            flag = 0;  // flag 초기화
            mask = 0x80; // 다시 8비트의 처음부터 받기위해 마스크값 초기화
            flag_count++; // 다음 바이트의 데이터를 수신하기 위해 flag_count 값 증가
        }
        
        // 32비트의 커스텀코드, 데이터코드를 전부 수신하면
        if(flag_count == 4)
        {   
            // 데이터가 정상적으로 수신 되었는지 체크섬을 통해 오류검출한다.
            // 데이터 코드 2바이트 중 앞 바이트의 보수가 뒤 바이트로 정의된다.
            // 정상적으로 수신 되었다면 데이터 코드 2개의 바이트를 체크섬하면
            // 0b11111111의 결과가 될 것이다.
            
            // 정상적으로 수신 되었다면 = 오류가 없다면
            if((IR_rx_data[2] ^ IR_rx_data[3]) == 0xFF)
            {
                // 리모컨 수신 데이터 중 의미 있는것은 데이터코드의 앞 1바이트이기 때문에
                // IR_rx_data[2] 만 있으면 된다.
                IR_data = IR_rx_data[2];
                
                // DOWN이 입력되었으면 최대 설정값에 따라 두개의  커서값을 감소시킨다.
                if(IR_data == DOWN && Cursor_val < 3 )
                {
                    Cursor_val = Cursor_val + 1;
                }
                if(IR_data == DOWN && Cursor_8 < 6 )
                {
                    Cursor_8 = Cursor_8 + 1;
                }
                
                // UP이 입력되었으면 최대 설정값에 따라 두개의  커서값을 감소시킨다.                
                if( IR_data == UP && Cursor_val > 1 )
                {
                    Cursor_val = Cursor_val - 1;     
                }
                if( IR_data == UP && Cursor_8 > 1 )
                {     
                    Cursor_8 = Cursor_8 - 1;            
                }

                // 리모컨으로 부터 다시 데이터를 받을 수 있도록
                // 변수를 초기화 한다.
                IR_start=0;
                mask=0x01;
                flag_count = 0;
                IR_rx_data[0] = 0x00;
                IR_rx_data[1] = 0x00;
                IR_rx_data[2] = 0x00;
                IR_rx_data[3] = 0x00;
                flag = 0;
                }

            // 오류가 발생 했으면 값을 저장하지 않고 초기화 한다.
            else
            {
                // 리모컨으로 부터 다시 데이터를 받을 수 있도록
                // 변수를 초기화 한다.
                IR_start=0;
                mask=0x01;
                flag_count = 0;
                IR_rx_data[0] = 0x00;
                IR_rx_data[1] = 0x00;
                IR_rx_data[2] = 0x00;
                IR_rx_data[3] = 0x00;
                flag = 0;
            }
            
        }
        
    } 
    
}

// 타이머/카운터0 출력비교(TCNT0 = OCRO 일때) 초기화 함수
void system_init()
{
    // 타이머/카운터0 출력비교(TCNT0 = OCRO 일때) 인터럽트 서비스 루틴
    // 인터럽트 발생 주기 1/16us * 8분주 * 200 = 100us  = 0.1ms
    TIMSK = 0x02;   // OCIE0 = 1 출력 비교 인터럽트 인에이블
    TCCR0 = 0x0A;   // 프리스케일 = CK/8, CTC 모드
    OCR0 = 199;     // 출력 비교 레지스터값
    TCNT0 = 0x00;   // 타이머/카운터0 레지스터 초기값

    // 인터럽트 초기화
    EICRB = 0x02;        // 외부 인터럽트 4 : 02 :하강 에지
    EIMSK = 0x10;        // 외부 인터럽트 4 인에이블     
    SREG = 0x80;         // 전역 인터럽트 인에이블 비트 I 셋. 
}


// 타이머/카운터0 출력비교(TCNT0 = OCRO 일때) 인터럽트 서비스 루틴
interrupt [TIM0_COMP] void timer_comp0(void)
{
    time_count++;
}





// 기본 화면 출력 함수
void Home()
{
    // 미세먼지 농도가 GOOD 이면 GOOD 단계의 LED를 세번 깜빡이고 기본 LED를 킨다.
    if( WHO_PM10() == 2 )
    {
        Mood_Light(GOOD_LED);    // GOOD_LED는 GOOD단계에서 켜질 LED, 
        delay_ms(300);           // 1 = RED, 2 = GREEN, 3 = BLUE, 4 = YELLOW, 5 = LED OFF
        
        Mood_Light(LED_OFF);
        delay_ms(300);
        
        Mood_Light(GOOD_LED);
        delay_ms(300);
        
        Mood_Light(LED_OFF);
        delay_ms(300);
        
        Mood_Light(GOOD_LED);
        delay_ms(300);  
        
        Mood_Light(LED_OFF);
        delay_ms(300);         
        
        // 미세먼지 농도가 BAD나 FINE으로 바뀌기 전까지 무한 반복
        // 단, 메뉴 버튼이 눌리면 메뉴로 진잆한다.
        while( WHO_PM10() == 2 && IR_data != MENU  )
        { 
            Home_Light();  // 시간, 미세먼지 출력
            
            delay_ms(200);
        }
    }
    
    // 미세먼지 농도가 FINE 이면 FINE 단계의 LED를 세번 깜빡이고 기본 LED를 킨다.
    if( WHO_PM10() == 1 )
    {
        Mood_Light(FINE_LED);    // FINE_LED는 FINE단계에서 켜질 LED, 
        delay_ms(300);           // 1 = RED, 2 = GREEN, 3 = BLUE, 4 = YELLOW, 5 = LED OFF
        
        Mood_Light(LED_OFF);
        delay_ms(300);
        
        Mood_Light(FINE_LED);
        delay_ms(300);

        Mood_Light(LED_OFF);
        delay_ms(300);

        Mood_Light(FINE_LED);
        delay_ms(300);

        Mood_Light(LED_OFF);
        delay_ms(300);
                
        // 미세먼지 농도가 BAD나 GOOD로 바뀌기 전까지 무한 반복
        // 단, 메뉴 버튼이 눌리면 메뉴로 진잆한다.
        while( WHO_PM10() == 1 && IR_data != MENU  )
        { 
            Home_Light();  // 시간, 미세먼지 출력
            
            delay_ms(200);
        }
    }
    
    // 미세먼지 농도가 BAD 이면 BAD 단계의 LED를 세번 깜빡이고 기본 LED를 킨다.
    if( WHO_PM10() == 0 )
    {
        Mood_Light(BAD_LED);     // BAD_LED는 BAD단계에서 켜질 LED, 
        delay_ms(300);           // 1 = RED, 2 = GREEN, 3 = BLUE, 4 = YELLOW, 5 = LED OFF
        
        Mood_Light(LED_OFF);
        delay_ms(300);
        
        Mood_Light(BAD_LED);
        delay_ms(300);
        
        Mood_Light(LED_OFF);
        delay_ms(300);

        Mood_Light(BAD_LED);
        delay_ms(300);
        
        Mood_Light(LED_OFF);
        delay_ms(300);
        
        // 미세먼지 농도가 GOOD이나 FINE으로 바뀌기 전까지 무한 반복
        // 단, 메뉴 버튼이 눌리면 메뉴로 진잆한다.
        while( WHO_PM10() == 0 && IR_data != MENU  )
        { 
            Home_Light();  // 시간, 미세먼지 출력
            
            delay_ms(200);
        }
    }


}


// 기본화면에서 RTC로부터 시간을 받아와 년,월,일,시,분,초,요일을 출력하고
// PM단위의 미세먼지 농도와 "WHO"기준 미세먼지 농도 단계를 함께 출력한다. 
void Home_Light()
{ 
    rtc_get_time();  // RTC로 부터 년,월,시,분,초,요일을 받아온다.
    
    Mood_Light(HOME_LED); // HOME_LED는 기본으로 켜질 LED, 
                          // 1 = RED, 2 = GREEN, 3 = BLUE, 4 = YELLOW, 5 = LED OFF
    
    LCD_Pos2(0,0); // LCD_Pos2함수는 20x4 CLCD를 제어하기 위해 새로 선언한 외부 함수
                   // "lcd.h 파일에 정의"
    sprintf(Message,"%.2d,%.2d,%.2d        %s", rtc[0],rtc[1],rtc[2],day[rtc[6]]); // 년,월,일,요일을 출력
    LCD_Str(Message);
    
    LCD_Pos2(1,0);
    sprintf(Message,"      %.2d:%.2d:%.2d      ", rtc[3],rtc[4],rtc[5]); // 시,분,초 출력
    LCD_Str(Message);
    
    LCD_Pos2(2,0);
    pm_dust(); // 미세먼지 농도 출력 단위 = [PM = ug/m^3]
    
    LCD_Pos2(3,16);
    LCD_Str(dust_step2[WHO_PM10()]); // 현재 미세먼지의 단계 출력, 기준은 WHO 미세먼지 기준
}


// 파라미터 값에 따라 LED ON
void Mood_Light(int HOME_LED)
{
    switch(HOME_LED)
    {
        case 1 :         // RED ON
            PORTD=0xE0;  // PORTD.5에 연결된 릴레이와 PORTD.6, PORTD.7에 연결된 릴레이
            break;       // 동작 방식이 달라서 5번 포트는 "1"일때 접점이 붙고
                         // 6번, 7번포트는 "0"일때 접점이 붙는다.
                         // 접점이 붙는다 = LED가 켜진다.
                                 
        case 2 :         // GREEN ON
            PORTD=0x80;   
            break;

        case 3 :         // BLUE ON
            PORTD=0x40;  
            break;
            
        case 4 :         // YELLOW ON
            PORTD=0xA0;  // 빨간색과 초록색을 동시에 ON 
            break;
              
        case 5 :         // LED OFF
            PORTD=0xC0;   
            break;
                                    
        default :       //그 외 입력 시
            break;
    }
}  

// 메인 메뉴에서 "MENU" 버튼을 눌렀을 때 메뉴 출력
void Sensor_Project_Menu()
{
    LCD_Pos2(0,0);
    LCD_Str("Sensor_Project_Menu");
    
    LCD_Pos2(1,0);
    LCD_Str("1. Home");
    
    LCD_Pos2(2,0);
    LCD_Str("2. Light Setting");
    
    LCD_Pos2(3, 0);
    LCD_Str("3. Dust Setting");
}


// Light_Setting 메뉴의 1p 출력
void Light_Setting()
{
    LCD_Pos2(0,0);
    LCD_Str("Light Setting");
    
    LCD_Pos2(1,0);
    LCD_Str("1. Red");
    
    LCD_Pos2(2,0);
    LCD_Str("2. Green");
    
    LCD_Pos2(3, 0);
    LCD_Str("3. Blue");
}

// Light_Setting 메뉴의 2p 출력
void Light_Setting2()
{
    LCD_Pos2(0,0);
    LCD_Str("Light Setting");
    
    LCD_Pos2(1,0);
    LCD_Str("4. Yellow");
    
    LCD_Pos2(2,0);
    LCD_Str("5. LED OFF");
    
    LCD_Pos2(3,0);
    LCD_Str("6. Exit");
}

// Dust_Setting 메뉴의 1p 출력
void Dust_Setting()
{
    LCD_Pos2(0,0);
    LCD_Str("Dust Setting");
    
    LCD_Pos2(1,0);
    LCD_Str("1. Good");
    
    LCD_Pos2(2,0);
    LCD_Str("2. Fine");
    
    LCD_Pos2(3, 0);
    LCD_Str("3. Bad");
}

// Dust_Setting 메뉴의 2p 출력
void Dust_Setting2()
{
    LCD_Pos2(0,0);
    LCD_Str("Dust Setting");
    
    LCD_Pos2(1,0);
    LCD_Str("4. LED OFF");
    
    LCD_Pos2(2,0);
    LCD_Str("5. Exit");
}

// Dust-Good Light 메뉴의 1p 출력
void Dust_Good_Light()
{
    LCD_Pos2(0,0);
    LCD_Str("Dust-Good Light");
    
    LCD_Pos2(1,0);
    LCD_Str("1. Red");
    
    LCD_Pos2(2,0);
    LCD_Str("2. Green");
    
    LCD_Pos2(3, 0);
    LCD_Str("3. Blue");
}

// Dust-Good Light 메뉴의 2p 출력
void Dust_Good_Light2()
{
    LCD_Pos2(0,0);
    LCD_Str("Dust-Good Light");
    
    LCD_Pos2(1,0);
    LCD_Str("4. Yellow");
    
    LCD_Pos2(2,0);
    LCD_Str("5. LED OFF");
    
    LCD_Pos2(3,0);
    LCD_Str("6. Exit");
}

// Dust-Fine Light 메뉴의 1p 출력
void Dust_Fine_Light()
{
    LCD_Pos2(0,0);
    LCD_Str("Dust-Fine Light");
    
    LCD_Pos2(1,0);
    LCD_Str("1. Red");
    
    LCD_Pos2(2,0);
    LCD_Str("2. Green");
    
    LCD_Pos2(3, 0);
    LCD_Str("3. Blue");
}

// Dust-Fine Light 메뉴의 2p 출력
void Dust_Fine_Light2()
{
    LCD_Pos2(0,0);
    LCD_Str("Dust-Fine Light");
    
    LCD_Pos2(1,0);
    LCD_Str("4. Yellow");
    
    LCD_Pos2(2,0);
    LCD_Str("5. LED OFF");
    
    LCD_Pos2(3,0);
    LCD_Str("6. Exit");
}

// Dust-Bad Light 메뉴의 1p 출력
void Dust_Bad_Light()
{
    LCD_Pos2(0,0);
    LCD_Str("Dust-Bad Light");
    
    LCD_Pos2(1,0);
    LCD_Str("1. Red");
    
    LCD_Pos2(2,0);
    LCD_Str("2. Green");
    
    LCD_Pos2(3, 0);
    LCD_Str("3. Blue");
}

// Dust-Bad Light 메뉴의 2p 출력
void Dust_Bad_Light2()
{
    LCD_Pos2(0,0);
    LCD_Str("Dust-Bad Light");
    
    LCD_Pos2(1,0);
    LCD_Str("4. Yellow");
    
    LCD_Pos2(2,0);
    LCD_Str("5. LED OFF");
    
    LCD_Pos2(3,0);
    LCD_Str("6. Exit");
}