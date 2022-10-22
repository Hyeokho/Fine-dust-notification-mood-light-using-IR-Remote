/*
 * 리모콘 예제, 8누르면 2나오고 채널 아래 누르면 1나옴
 * 동작은 됨
 *
 *
*/


#include <mega128.h>
#include <stdio.h>
#include <delay.h>
#include "lcd.h"

void system_init();

interrupt [TIM0_COMP] void timer_comp0();

unsigned int time_count = 0;
unsigned int edge=0;
unsigned char flag = 0; 
unsigned char flag_count = 0; 
unsigned char IR_start=0; 
unsigned char IR_rx_data[4] = { 0x00, 0x00, 0x00, 0x00 };
unsigned char IR_data;
unsigned long mask=0x80;
unsigned char S_flag = 0, old_flag = 0;


char Message[40] = "time = "; // LCD 화면에 문자열 출력을 위한 문자열 변수

interrupt[EXT_INT4] void input(void)   // 입력 캡쳐 인터럽트
{
    // PORTB=0b11100111;
    edge = time_count;
    
    time_count=0;
    
    if(edge > 80 && edge < 100) // lead code를 읽기 위함. 9.0ms  
    {
        //PORTB=0x0F;
        IR_start=1;
    }
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
            //PORTB=0b10111111; 
            IR_rx_data[flag_count] |= mask;
            mask=mask>>1;
            flag++;
        }
        
        if(flag == 8)
        {
            // PORTB=0b01111110;  
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

                IR_data = IR_rx_data[2];
                
                //리모컨 수신 데이터 저장
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

 

void main()
{
    LCD_Init();
    
    DDRE = 0x00;    //IR Rx Pin ( ICP3 )
    
    DDRB = 0xff;
    
    system_init();
    
    while (1)
    {
        switch(IR_data)
        {
            case 0x20 :     //Number 0
                PORTB=0b11111110;
                LCD_Num(1);
                break;

            case 0xA0 :     //Number 1
                PORTB=0b11111101;
                LCD_Num(2);
                break;

            case 0x60 :     //Number 2
                PORTB=0b11111101;
                LCD_Num(3);
                break;

            case 0x10 :     //Number 3
                PORTB=0b11111011;
                LCD_Num(4);
                break;
                
            case 0x90 :     //Number 4
                PORTB=0b11110111;
                LCD_Num(5);
                break;

            case 0x50 :     //Number 5
                PORTB=0b11101111;
                LCD_Num(6);
                break;

            case 0x30 :     //Number 6
                PORTB=0b11011111;
                LCD_Num(7);
                break;

            case 0xB0 :     //Number 7
                PORTB=0b10111111;
                LCD_Num(8);
                break;
            case 0x70 :     //Number 9
                PORTB=0b10111111;
                LCD_Num(9);
                break;    
            
            default :       //그 외 키 입력 시
                //PORTB=0xFF;
                //delay_ms(500);
                //PORTB=0xFF;
                //delay_ms(500);
                break;
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


// 타이머/카운터0 출력비교(TCNT0 = OCRO 일때) 인터럽트 서비스 루틴
interrupt [TIM0_COMP] void timer_comp0(void)
{
    time_count++;
} 