/*****************************************************************************/
/************************       dust.h  ***************************************/
/*****************************************************************************/

#include <mega128.h>
#include <delay.h>
#include <stdio.h>


#define ADC_VREF_TYPE 0x00 // A/D 컨버터 사용 기준 전압 REFS 설정
#define ADC_AVCC_TYPE 0x40 // A/D 컨버터 사용 기준 전압 AVCC 설정
#define ADC_RES_TYPE 0x80  // A/D 컨버터 사용 기준 전압 RES 설정
#define ADC_2_56_TYPE 0xC0 // A/D 컨버터 사용 기준 전압 2.56 설정



void ADC_Init();
unsigned int Read_ADC_Data(unsigned char);


int adcRaw =0; // ADC 데이터 저장용
int dust_step = 0; // 미세먼지 단계 설정 // 0 - BAD, 1 - FINE, 2 - GOOD       
float adcVoltage =0; // ADC 데이터를 전압으로 변환한 데이터 저장용   
float dust = 0; // 먼지농도 저장용

char dust_pm[40]; // LCD 화면에 문자열 출력을 위한 문자열 변수

float Median_Filter[40] = {0}; // 데이터를 저장 받기 위한 배열  
int i = 0;              // i, j, a, b 는 반복 변수
int j = 0;
int a = 0;
int b = 0;
float dust_temp = 0;
float switch_temp = 0;


/**
* @brief ADC 채널을 단일 입력 채널로 읽는 함수
* @param adc_input: ADC 하고자 하는 채널의 번호
* @retval AD 변환 값( 0∼1023)
*/
unsigned int Read_ADC_Data(unsigned char adc_input)
{
    unsigned int ADC_Data = 0; // AD 변환 채널 설정
    ADMUX &= ~(0x1f);          // 입력 채널 선택을 위해 MUX 입력을 마스크하여 클리어함. 상위 3비트 기존 값 유지
                               // 초기화 과정에서 설정한 값을 유지하기 위해
    ADMUX |= (adc_input & 0x07); // ADC의 단일 입력만 처리하기 위해 MUX의 하위 3비트만 사용
                                 // 단극성 입력 방법에는 8가지가 잇음 000~111까지 3비트만 씀                         
    ADCSRA |= (1<<ADSC) ;        // AD 변환 시작
    while(!(ADCSRA & (1 << ADIF))); // AD 변환 종료 대기
    ADC_Data = ADCL;                // 변환결과를 저장. 단극성 경우 10비트 양의 정수로 표시되어 0~1023의 범위
    ADC_Data |= ADCH<<8;            // ADCL은 아날로그 변환된 데이터를 저장하는건데 ADCH(상위), ADCL(하위) 각 8비틀르 합쳐서 16비트
    return ADC_Data;                // 변환된 값 반환
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
    // 5<<ADPS0 : AC변환 분주비 설정, 32분주비
    // ADSC비트는 실제 활용에서 ADC를 읽을 때 설정하여 변환한 후 데이터를 읽기 위해
    // 초기화 과정에서는 설정하지 않음

}


/**
* @brief ADC 결과를 읽어 LCD에 값을 표기하는 메인 프로그램, LCD 표시 과정은 LCD 함수 이용함.
*/
void pm_dust(void)
{
        for( i=0; i<49; i++ )
        {
            PORTC.5 = 0; // 적외선 LED ON
        
            delay_us(280); // 280us 대기
        
            adcRaw = Read_ADC_Data(1); // ADC 1번채널 변환 요청
        
            delay_us(40); // 40us 대기
        
            PORTC.5 = 1; // 적외선 LED OFF
        
            delay_us(968); // 968us 대기
        
            adcVoltage = ( (((float)adcRaw * 5) /1024)); // 전압으로 변환, VREF = 5V
        
            // 선형 방정식 적용 단위는 mg/m^3
            dust =  0.17 * (adcVoltage+0.8) - 0.1; 
            
            // 중간값 필터를 적용하기 위해 배열에 값 저장
            Median_Filter[i] = dust;
        }
        
        // 배열에 50개의 값을 저장하고 내림차순으로 정렬한다.
        for(a=0; a<38; a++)
        {
            for(b=0; b<38-a; b++)
            {
                if( Median_Filter[b] < Median_Filter[b+1] )
                {   
                    switch_temp = Median_Filter[b];
                    Median_Filter[b] =  Median_Filter[b+1];
                    Median_Filter[b+1] = Median_Filter[b];
                }
                
            }
        }
        
        switch_temp = 0; // 스위칭값 초기화
        
        // 데이터 배열의 중간에 있는 데이터들의 합을 구한다.
        for(j=10; j<29; j++)
        {
            dust_temp = dust_temp + Median_Filter[j];    
        }
        
        // 데이터의 평균
        dust = dust_temp / 20;               
         
        dust_temp = 0; // 임시변수 초기         
        
        sprintf(dust_pm,"     PM: %.0f        ", dust*1000); // PM단위인 ug/m^3로 출력
        //sprintf(dust_pm,"       V: %.3f        ", adcVoltage); // 5v 단위의 소수점 3번째 자리까지 출력, 값을 보기 위함
        
        // 문자열 출력  
        LCD_Str(dust_pm);             
}

int WHO_PM10()
{
    // WHO 기준에 따라서 30 ug/m^3 = PM 30 이하이면, 좋음
    if( dust*1000 <= 30 )
    {
        dust_step = 2;  // GOOD
    }
    
    // PM 30 초과, 50 미만이면, 괜찮음
    else if ( dust*1000  > 30 && dust*1000 <= 50 )
    {
        dust_step = 1;  // FINE
    }
    // 50초과이면 나쁨
    else
    {
        dust_step = 0;  // BAD
    }
    
    // 단계값, 2 : GOOD, 1: FINE, 0 : BAD 에따른 값 리턴
    return dust_step;
}
