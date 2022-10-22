/*
 * 먼지테스트
 *
 *
*/ 

#include <mega128.h>
#include <delay.h>
#include <stdio.h>
#include "lcd.h"

#define ADC_VREF_TYPE 0x00 // A/D 컨버터 사용 기준 전압 REFS 설정
#define ADC_AVCC_TYPE 0x40 // A/D 컨버터 사용 기준 전압 AVCC 설정
#define ADC_RES_TYPE 0x80  // A/D 컨버터 사용 기준 전압 RES 설정
#define ADC_2_56_TYPE 0xC0 // A/D 컨버터 사용 기준 전압 2.56 설정

// 왜 있는지 모르겠음
/*
unsigned int ADC_Data; // ADC 변수의 설정(16비트 데이터)
unsigned int ADC_Data=0; // 변수를 초기화하여 설정
sfrw ADCW=4; // 16 bit access
ADC_Data |= ADCH<<8; // 상위 바이트 읽기
ADC_Data = ADCL; // 하위 바이트 읽기
ADC_Data = ADCW; // 워드로 읽기
*/


/**
* @brief ADC 채널을 단일 입력 채널로 읽는 함수
* @param adc_input: ADC 하고자 하는 채널의 번호
* @retval AD 변환 값( 0∼1023)
*/
unsigned int Read_ADC_Data(unsigned char adc_input)
{
    unsigned int ADC_Data = 0; // AD 변환 채널 설정
    ADMUX &= ~(0x1f); // 입력 채널 선택을 위해 MUX 입력을 마스크하여 클리어함. 상위 3비트 기존 값 유지
                      // 초기화 과정에서 설정한 값을 유지하기 위해
    ADMUX |= (adc_input & 0x07); // ADC의 단일 입력만 처리하기 위해 MUX의 하위 3비트만 사용
                                 // 단극성 입력 방법에는 8가지가 잇음 000~111까지 3비트만 씀
                           
                          // 인터럽트 발생 주기 1/16us * 32분주 = 100us  = 0.1ms
    ADCSRA |= (1<<ADSC) ; // AD 변환 시작
    while(!(ADCSRA & (1 << ADIF))); // AD 변환 종료 대기
    ADC_Data = ADCL; //ㅡㅡ변환결과를 저장. 단극성 경우 10비트 양의 정수로 표시되어 0~1023의 범위
    ADC_Data |= ADCH<<8; // ADCL은 아날로그 변환된 데이터를 저장하는건데 ADCH(상위), ADCL(하위) 각 8비틀르 합쳐서 16비트
    return ADC_Data;    
}                     

// 프리러닝 모드로의 초기화, 프리런닝모드(Free running mode) : ADC 동작을 반복적으로 수행하게 된다.
void ADC_Init(void)
{
    ADCSRA = 0x00; // ADC 설정을 위한 비활성화

    // ADLAR : 1일때 변환 결과가 A/D 컨버터 데이터 레지스터에 저장될 때 좌측으로 끝을 맞추어 저장
    // REFS = 1 일때 외부의 AVCC 단자로 입력된 전압 사용
    ADMUX = ADC_AVCC_TYPE | (0<ADLAR) | (0<<MUX0);
    // REFS = 1, ADLAR=0, MUX=0(ADC0 선택)
    
    ADCSRA = (1<<ADEN) | (1<<ADFR) | (5<<ADPS0);
    // 1<<ADEN : AD변환 활성화
    // 1<<ADFR : Free Running 모드 활성화, 임의의 시간에 A/D 컨버터 데이터 레지스터를 읽을 수 있음
    // 5<<ADPS0 : AC변환 분주비 설정, 32분주비  --> 1/16us * 32분주 =  
    // ADSC비트는 실제 활용에서 ADC를 읽을 때 설정하여 변환한 후 데이터를 읽기 위해
    // 초기화 과정에서는 설정하지 않음

}

/**
* @brief ADC 결과를 읽어 LCD에 값을 표기하는 메인 프로그램, LCD 표시 과정은 LCD 함수 이용함.
*/
void main(void)
{
    int adcRaw =0; // ADC 데이터 저장용
    
    float adcVoltage =0; // ADC 데이터를 전압으로 변환한 데이터 저장용
    
    float dust = 0; // 먼지농도 저장용
    
    char Message[40]; // LCD 화면에 문자열 출력을 위한 문자열 변수
    char Message2[40]; // LCD 화면에 문자열 출력을 위한 문자열 변수
    
    ADC_Init(); // ADC 초기화 
    
    LCD_Init(); // LCD 초기화
    
    DDRC  = 0xFF;        // PORTC를 출력으로
    
    
    LCD_Pos(0,0);
    LCD_Str("RawData:");
    //16x2 LCD의 0번째 행, 0번째 열부터 RawData가 표시된다.
    LCD_Pos(1,0);
    LCD_Str("R.Value:");
    //16x2 LCD의 1번째 행, 0번째 열부터 R.Value가 표시된다. 
    
    while(1)
    {   
        PORTC.6 = 0; // 적외선 LED ON
        
        delay_us(280);
        
        adcRaw = Read_ADC_Data(1); // ADC 1번채널 변환 요청
        
        delay_us(14);
        
        PORTC.6 = 1; // 적외선 LED OFF
        
        adcVoltage = ( (((float)adcRaw * 5) /1024)); // 전압으로 변환, VREF = 5V
        
        dust = (adcVoltage-0.6)*0.1667;
        
        
        // AD 변환 데이터 출력
        sprintf(Message,"adcV : %.3f ", adcVoltage);
        LCD_Pos(0,0);
        LCD_Str(Message);
        
        
        // AD 변환 데이터를 이용한 전압 출력
        sprintf(Message2,"DUST: %.2f ", dust*1000);
        LCD_Pos(1,0);
        LCD_Str(Message2);
        delay_ms(200);
    }
}