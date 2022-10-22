/*
 * 종합
 *
 *
*/ 

#include <mega128.h>
#include <delay.h>
#include <stdio.h>
#include "rtc.h"
#include "lcd.h"
#include "dust.h"


#define UP 0x06
#define DOWN 0x86
#define MENU 0xD2
#define SEL 0x16

#define LED_OFF 5


void system_init();
void Init_Timer2(void);
void Mood_Light(int);
void Home_Light();
interrupt [TIM0_COMP] void timer_comp0();






char Message[40]; // LCD 화면에 문자열 출력을 위한 문자열 변수
char Message2[40]; // LCD 화면에 문자열 출력을 위한 문자열 변수

char *dust_step2[20] = {" BAD", "FINE", "GOOD"};





/// 리모콘 변수
unsigned int time_count = 0;
unsigned int edge=0;
unsigned char flag = 0; 
unsigned char flag_count = 0; 
unsigned char IR_start=0; 
unsigned char IR_rx_data[4] = { 0x00, 0x00, 0x00, 0x00 };
unsigned char IR_data;
unsigned long mask=0x80;
unsigned char S_flag = 0, old_flag = 0;


int PM_STEP = 0;
int PM_TEMP;


// 기본 커서값
int Cursor_val = 1;

// 세부 동작 메뉴에서 사용할 커서 값 변수 선언
int Cursor_4 = 0;
int Cursor_8 = 0;


// 평소에 켜져있는 LED 설정, RED = 1, GREEN = 2, 3 = BLUE, 4 = YELLOW
int HOME_LED = 4;

int GOOD_LED = 3;
int FINE_LED = 2;
int BAD_LED = 1;





void Home()
{
    // 미세먼지 농도가 GOOD 이면
    if( WHO_PM10() == 2 )
    {
        Mood_Light(GOOD_LED);
        
        delay_ms(300);
        
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
        while( WHO_PM10() == 2 && IR_data != MENU  )
        { 
            Home_Light();
            
            delay_ms(200);
        }
    }
    
    // 미세먼지 농도가 FINE 이면
    if( WHO_PM10() == 1 )
    {
        Mood_Light(FINE_LED);
        
        delay_ms(300);
        
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
        // 미세먼지 농도가 GOOD이나 BAD으로 바뀌기 전까지 무한 반복
        while( WHO_PM10() == 1 && IR_data != MENU  )
        { 
            Home_Light();
            
            delay_ms(200);
        }
    }
    
    // 미세먼지 농도가 BAD 이면
    if( WHO_PM10() == 0 )
    {
        Mood_Light(BAD_LED);
        
        delay_ms(300);
        
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
        
        // 미세먼지 농도가 BAD나 FINE으로 바뀌기 전까지 무한 반복
        while( WHO_PM10() == 0 && IR_data != MENU  )
        { 
            Home_Light();
            
            delay_ms(200);
        }
    }


}

void Home2()
{
    
    PM_TEMP = WHO_PM10();

    // 이전의 미세먼지 단계와 새로 측정한 미세먼지 단계가 같으면
    if( PM_STEP == PM_TEMP )
    {
        Home_Light();
    }
    // 새로 측정한 미세먼지 단계가 GOOD로 바뀌었으면
    else if( PM_TEMP == 2  )
    {   
        PM_STEP = 2;
        
        Home_Light();
        
        Mood_Light(GOOD_LED);
        
        delay_ms(300);
        
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
        
        Mood_Light(HOME_LED);
        
    
    }
    // 새로 측정한 미세먼지 단계가 FINE으로 바뀌었으면
    else if( PM_TEMP == 1  )
    {
        PM_STEP = 1;
        
        Home_Light();
        
        Mood_Light(FINE_LED);
        
        delay_ms(300);
        
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
        
        Mood_Light(HOME_LED);        
       
    
    }    
    // 새로 측정한 미세먼지 단계가 BAD로 바뀌었으면
    else if( PM_TEMP == 0  )
    {
        PM_STEP = 0;
        
        Home_Light();
        
        Mood_Light(BAD_LED);
        
        delay_ms(300);
        
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
        
        Mood_Light(HOME_LED);
            
    }
}






 

void Home_Light()
{ 
    rtc_get_time();     //RTC 함수 호출
    
    Mood_Light(HOME_LED);
    
    LCD_Pos2(0,0);
    sprintf(Message,"%.2d,%.2d,%.2d        %s", rtc[0],rtc[1],rtc[2],day[rtc[6]]);
    LCD_Str(Message);
    
    LCD_Pos2(1,0);
    sprintf(Message,"      %.2d:%.2d:%.2d      ", rtc[3],rtc[4],rtc[5]);
    LCD_Str(Message);
    
    LCD_Pos2(2,0);
    pm_dust();
    
    LCD_Pos2(3,16);
    LCD_Str(dust_step2[WHO_PM10()]);
}



void Mood_Light(int HOME_LED)
{
    switch(HOME_LED)
    {
        case 1 :     
            PORTD=0xE0;  // 5번 1, 67번, 1 
            break;
        
        case 2 :     // 평소에 빨간
            PORTD=0x80;  // 5번 1, 67번, 1 
            break;

        case 3 :     // 평소에 빨간
            PORTD=0x40;  // 5번 1, 67번, 1 
            break;
            
        case 4 :     // 평소에 빨간
            PORTD=0xA0;  // 5번 1, 67번, 1 
            break;  
        case 5 :     // 평소에 빨간
            PORTD=0xC0;  // 5번 1, 67번, 1 
            break;                        
        default :       //그 외 키 입력 시
            break;
    }
    

}  


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

void Dust_Setting2()
{
    LCD_Pos2(0,0);
    LCD_Str("Dust Setting");
    
    LCD_Pos2(1,0);
    LCD_Str("4. LED OFF");
    
    LCD_Pos2(2,0);
    LCD_Str("5. Exit");
}

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








// 포트 세팅은 각 초기화함수에 되어 있지만 한 눈에보기 좋게 정의
void PORT_set()
{
    // RTC 및 적외선 출력
    DDRC=0xff;
    
    // LCD
    DDRA  = 0xFF;        // PORTA를 출력으로
    DDRG |= 0x0F;        // PORTG의 하위 4비트를 출력으로
    
    

}

void RTC_Init()
{
     DDRC=0xff;
     
     rtc[0]=0x18;                        //year 초기값
     rtc[1]=0x12;                        //month 초기값
     rtc[2]=0x02;                        //date 초기값
     rtc[3]=0x10;                        //hour 초기값
     rtc[4]=0x30;                        //min 초기값
     rtc[5]=0x00;                        //sec 초기값
     rtc[6]=0x07;                        //day 초기값 1234567
     
     rtc_set_time();                    // 원하는 시간설정후 주석 해제 
}
 



void main(void)
{
    DDRE = 0x00;    //IR Rx Pin ( ICP3 )
    DDRB = 0xff;
    DDRD = 0xE0;    // LED 제
    
    
    
    LCD_Init(); // LCD 초기화
    
    ADC_Init(); // ADC 초기화 
    RTC_Init(); // RTC 초기화
    
    system_init();// 리모컨 이니셜
    //Init_Timer2();
                           
    PORTG &= ~(1<<4);
    PORTD =0xC0 ;
 
    
    // 미세먼지 농도 단계 임시저장 변수
    PM_STEP=WHO_PM10(); 
    
    Home_Light();
    
    

    
    while(1)
    {   
       
       Home();
       delay_ms(200);
        
        if(IR_data == MENU)
        {   
            LCD_Clear();
            Sensor_Project_Menu();
            
            while(1)
            {
                LCD_Pos2(Cursor_val, 0);
                blink_OnOff(1);
                
                if(Cursor_val == 1 && IR_data == SEL )
                {
                    LCD_Clear();
                    break;
                }
                
                if(Cursor_val == 2 && IR_data == SEL )
                {
                   LCD_Clear();   
                   Light_Setting();
                   
                   IR_data = 0x00; //선텍갮 초기화
                   Cursor_8 = 1; // 커서값 초기화
                   
                   while(1)
                   {    

                        LCD_Pos2(Cursor_8, 0);
                       
                        // 5번 1이면 켜짐 , 6.7 0이면 켜짐 
                        // 평소에 켜지는 조명 레드
                        if(Cursor_8 == 1 && IR_data == SEL )
                        {
                            HOME_LED = 1;
                            PORTD=0xE0;  // 5번 1, 67번, 1 
                            IR_data = 0x00;
                            Sensor_Project_Menu();
                            break;
                        }
                        if(Cursor_8 == 2 && IR_data == SEL )
                        {
                            HOME_LED = 2;
                            PORTD=0x80;                            
                            IR_data = 0x00;
                            Sensor_Project_Menu();
                            break;
                        }
                        if(Cursor_8 == 3 && IR_data == SEL )
                        {
                            HOME_LED = 3;
                            PORTD=0x40;
                            IR_data = 0x00;
                            Sensor_Project_Menu();
                            break;
                        }
                        
                        if( Cursor_8 >= 4)
                        {  
                            LCD_Clear();
                            Light_Setting2();
                            
                            while(1)
                            {
                                LCD_Pos2(Cursor_8-3, 0);
                                
                                if(Cursor_8 < 4 )
                                {
                                    LCD_Clear();   
                                    Light_Setting();
                                    break;
                                }
                                
                                if(Cursor_8 == 4 && IR_data == SEL )
                                {
                                    HOME_LED = 4;
                                    PORTD=0xA0;                                    
                                    break;
                                }
                                if(Cursor_8 == 5 && IR_data == SEL )
                                {
                                    HOME_LED = 5;                                    
                                    PORTD =0xC0 ;
                                    break;
                                }
                                
                                if(Cursor_8 == 6 && IR_data == SEL )
                                {
                                    break;
                                }           
                            }
                            

                        }
                       
                        if(Cursor_8 >= 4 && IR_data == SEL )
                        {
                            IR_data = 0x00;
                            Sensor_Project_Menu();
                            break;
                        }       
                        
                        
                   } 
                }
                
                // dust setting
                if(Cursor_val == 3 && IR_data == SEL )
                {   
                   LCD_Clear();   
                   Dust_Setting();
                   
                   IR_data = 0x00; //선텍갮 초기화
                   Cursor_8 = 1; // 커서값 초기화
                   
                   while(1)
                   {    

                        LCD_Pos2(Cursor_8, 0);
                       
                        // good 세팅
                        if(Cursor_8 == 1 && IR_data == SEL )
                        {
                            LCD_Clear();   
                            Dust_Good_Light();
                   
                            IR_data = 0x00; //선텍갮 초기화
                            Cursor_8 = 1; // 커서값 초기화
                            
                            while(1)
                            {
                                LCD_Pos2(Cursor_8, 0);
                                
                                // good되면  red켜짐
                                if(Cursor_8 == 1 && IR_data == SEL)
                                {
                                    GOOD_LED = 1;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                if(Cursor_8 == 2 && IR_data == SEL)
                                {
                                    GOOD_LED = 2;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                if(Cursor_8 == 3 && IR_data == SEL)
                                {
                                    GOOD_LED = 3;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                
                                if( Cursor_8 >= 4)
                                {  
                                    LCD_Clear();
                                    Dust_Good_Light2();
                            
                                    while(1)
                                    {
                                        LCD_Pos2(Cursor_8-3, 0);
                                
                                        if(Cursor_8 < 4 )
                                        {
                                            LCD_Clear();   
                                            Dust_Good_Light();
                                            break;
                                        }
                                
                                        if(Cursor_8 == 4 && IR_data == SEL )
                                        {
                                            GOOD_LED = 4;                                    
                                            break;
                                        }
                                        if(Cursor_8 == 5 && IR_data == SEL )
                                        {
                                            GOOD_LED = 5;                                    
                                            break;
                                        }
                                
                                        if(Cursor_8 == 6 && IR_data == SEL )
                                        {
                                            break;
                                        }
                                    }
    
                                }
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

                        // fine 세팅
                        if(Cursor_8 == 2 && IR_data == SEL )
                        {
                            LCD_Clear();   
                            Dust_Fine_Light();
                   
                            IR_data = 0x00; //선텍갮 초기화
                            Cursor_8 = 1; // 커서값 초기화
                            
                            while(1)
                            {
                                LCD_Pos2(Cursor_8, 0);
                                
                                // fine되면  red켜짐
                                if(Cursor_8 == 1 && IR_data == SEL)
                                {
                                    FINE_LED = 1;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                if(Cursor_8 == 2 && IR_data == SEL)
                                {
                                    FINE_LED = 2;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                if(Cursor_8 == 3 && IR_data == SEL)
                                {
                                    FINE_LED = 3;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                
                                if( Cursor_8 >= 4)
                                {  
                                    LCD_Clear();
                                    Dust_Fine_Light2();
                            
                                    while(1)
                                    {
                                        LCD_Pos2(Cursor_8-3, 0);
                                
                                        if(Cursor_8 < 4 )
                                        {
                                            LCD_Clear();   
                                            Dust_Fine_Light();
                                            break;
                                        }
                                
                                        if(Cursor_8 == 4 && IR_data == SEL )
                                        {
                                            FINE_LED = 4;                                    
                                            break;
                                        }
                                        if(Cursor_8 == 5 && IR_data == SEL )
                                        {
                                            FINE_LED = 5;                                    
                                            break;
                                        }
                                
                                        if(Cursor_8 == 6 && IR_data == SEL )
                                        {
                                            break;
                                        }
                                    }
    
                                }
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
                         // Bad 세팅
                        if(Cursor_8 == 3 && IR_data == SEL )
                        {
                            LCD_Clear();   
                            Dust_Bad_Light();
                   
                            IR_data = 0x00; //선텍갮 초기화
                            Cursor_8 = 1; // 커서값 초기화
                            
                            while(1)
                            {
                                LCD_Pos2(Cursor_8, 0);
                                
                                // fine되면  red켜짐
                                if(Cursor_8 == 1 && IR_data == SEL)
                                {
                                    BAD_LED = 1;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                if(Cursor_8 == 2 && IR_data == SEL)
                                {
                                    BAD_LED = 2;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                if(Cursor_8 == 3 && IR_data == SEL)
                                {
                                    BAD_LED = 3;
                                    IR_data = 0x00;
                                    LCD_Clear();
                                    Dust_Setting();
                                    break;                                    
                                }
                                
                                if( Cursor_8 >= 4)
                                {  
                                    LCD_Clear();
                                    Dust_Bad_Light2();
                            
                                    while(1)
                                    {
                                        LCD_Pos2(Cursor_8-3, 0);
                                
                                        if(Cursor_8 < 4 )
                                        {
                                            LCD_Clear();   
                                            Dust_Bad_Light();
                                            break;
                                        }
                                
                                        if(Cursor_8 == 4 && IR_data == SEL )
                                        {
                                            BAD_LED = 4;                                    
                                            break;
                                        }
                                        if(Cursor_8 == 5 && IR_data == SEL )
                                        {
                                            BAD_LED = 5;                                    
                                            break;
                                        }
                                
                                        if(Cursor_8 == 6 && IR_data == SEL )
                                        {
                                            break;
                                        }
                                    }
    
                                }
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
                        if( Cursor_8 >= 4)
                        {  
                            LCD_Clear();
                            Dust_Setting2();
                            
                            while(1)
                            {
                                LCD_Pos2(Cursor_8-3, 0);
                                
                                if(Cursor_8 < 4 )
                                {
                                    LCD_Clear();   
                                    Dust_Setting();
                                    break;
                                }
                                
                                // LED OFF 안키
                                if(Cursor_8 == 4 && IR_data == SEL )
                                {
                                    GOOD_LED = 5;
                                    FINE_LED = 5;
                                    BAD_LED = 5;                                   
                                    break;
                                }
                                
                                if(Cursor_8 == 5 && IR_data == SEL )
                                {
                                    break;
                                }
                                if(Cursor_8 == 6 )
                                {
                                    Cursor_8 = Cursor_8-1;
                                }             
                            }
                        }                                                                                             
                   
                        if(Cursor_8 >= 4 && IR_data == SEL )
                        {
                            IR_data = 0x00;
                            Sensor_Project_Menu();
                            break;
                        }    
                            
                            
                
                        
                        
                            

                        
                   }                   
                   
                   
                   
                                    
                }
                
            }
        }
    }
}






interrupt[EXT_INT4] void input(void)   // 입력 캡쳐 인터럽트
{
    edge = time_count;
    
    time_count=0;
    
    // 리드코드인식단계
    if(edge > 80 && edge < 100) // lead code를 읽기 위함. 9.0ms  
    {
        IR_start=1;
    }
    
    // 리드코드가 들어왔으면
    else if(IR_start==1)
    {
        if(edge > 8 && edge < 13)//  0        1.125ms
        {
            IR_rx_data[flag_count] &= ~mask;
            mask = mask >>1;
            flag++;
        }
        else if(edge > 19 && edge < 28)//  1        2.25ms
        {
            IR_rx_data[flag_count] |= mask;
            mask=mask>>1;
            flag++;
        }
        
        if(flag == 8)
        {  
            flag = 0;
            mask = 0x80;
            flag_count++;
        }

        if(flag_count == 4)// data를 모두 받으면..
        {   
            //data가 정확히 들어왔는가?
            if((IR_rx_data[2] ^ IR_rx_data[3]) == 0xFF)
            {
                PORTB=0b00111100;
                
                if(S_flag == 0)        
                {
                    old_flag = S_flag;
                    S_flag = 1;    
                }
                else
                {
                    old_flag = S_flag;
                     S_flag = 0;
                }

                //리모컨 수신 데이터 저장
                IR_data = IR_rx_data[2];
                
                if(IR_data == DOWN && Cursor_val < 3 )
                {
                    Cursor_val = Cursor_val + 1;
                }
                if(IR_data == DOWN && Cursor_8 < 6 )
                {
                    Cursor_8 = Cursor_8 + 1;
                }                
                if( IR_data == UP && Cursor_val > 1 )
                {
                    Cursor_val = Cursor_val - 1;     
                }
                if( IR_data == UP && Cursor_8 > 1 )
                {     
                    Cursor_8 = Cursor_8 - 1;            
                }

                
                IR_start=0;
                mask=0x01;
                flag_count = 0;
                IR_rx_data[0] = 0x00;
                IR_rx_data[1] = 0x00;
                IR_rx_data[2] = 0x00;
                IR_rx_data[3] = 0x00;
                flag = 0;
                }


            else
            {
                //PORTB=0b00111111;
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

void system_init()
{
    // 타이머/카운터0 출력비교(TCNT0 = OCRO 일때) 인터럽트 서비스 루틴
    // 인터럽트 발생 주기 1/16us * 8분주 * 200 = 100us  = 0.1ms
    TIMSK = 0x02;         // OCIE0 = 1 출력 비교 인터럽트 인에이블
    TCCR0 = 0x0A;  // 프리스케일 = CK/8, CTC 모드
    OCR0 = 199;      // 출력 비교 레지스터값
    TCNT0 = 0x00;      // 타이머/카운터0 레지스터 초기값

   // 인터럽트 초기화
   EICRB = 0x02;        // 외부 인터럽트 4 : 02 :하강 에지
   EIMSK = 0x10;        // 외부 인터럽트 4 인에이블     
   SREG = 0x80;         // 전역 인터럽트 인에이블 비트 I 셋. 
}

// TCCR2 (타이머/카운터 제어 레지스터2) 사용
void Init_Timer2(void)
{
   // 타이머/카운터 제어 레지스터2
   TCCR2 = 0x00; // TCCR2 사용정지

   // 타이머/카운터 인터럽트 마스크 레지스터
   TIMSK |=(1<<TOIE2); // ?????????????

   // (1<<CS22)는 100 가됨 TCCR2와 100 을 or 논리연산 하면
   // TCCR2 | 00000100 연산임
   // CS22의 값(2)만큼 왼쪽 시프트 (2 만큼 시프트 ) = 256분주 설정 [CS값에 따라 분주 설정]
   TCCR2 |=(1<<CS22);
  
   SREG |=0X80; // 
}



// 타이머/카운터0 출력비교(TCNT0 = OCRO 일때) 인터럽트 서비스 루틴
interrupt [TIM0_COMP] void timer_comp0(void)
{
    time_count++;
}
