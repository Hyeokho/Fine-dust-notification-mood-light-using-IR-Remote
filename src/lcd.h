/*****************************************************************************/
/************************       lcd.h  ***************************************/
/*****************************************************************************/

#include <mega128.h>
#include <delay.h>

#define LCD_WDATA   PORTA        // LCD 데이터 버스 정의 (데이터 쓰기)
#define LCD_WINST   PORTA        // LCD 데이터 버스 정의 (명령어 쓰기)
#define LCD_RDATA   PINA         // LCD 데이터 버스 정의 (데이터 읽기)

#define LCD_CTRL    PORTG        // LCD 제어 신호 정의
#define LCD_EN  0                // Enable 신호
#define LCD_RW  1                // 읽기(1)/쓰기(0)
#define LCD_RS  2                // 데이터(1)/명령어(0)

#define RIGHT 1                  // 커서 및 화면 이동 오른쪽 방향 지정
#define LEFT 0                   // 커서 및 화면 이동 왼쪽 방향 지정


#define ON 1  //불리언 상수 정의
#define OFF 0

#define blink 0x0F // 커서 위치에 있는 문자 점멸

#define  col_1  0x80   // 20x4 CLCD의 (0,0)
#define  col_2  0xC0   // 20x4 CLCD의 (1,0)
#define  col_3  0x94   // 20x4 CLCD의 (3,0)
#define  col_4  0xD4   // 20x4 CLCD의 (4,0)


// 20x4 CLCD에 접근하기 위한 배열
int col[4] = {col_1, col_2, col_3, col_4};


void LCD_Data(unsigned char ch);	
void LCD_Comm(unsigned char command);
void LCD_Delay(unsigned char ms);		
void LCD_Char(unsigned char ch);
void LCD_Num(int c);
void LCD_Str(unsigned char *str);
void LCD_Pos(unsigned char x, unsigned char y);
void LCD_Pos2(unsigned char x, unsigned char y);
void LCD_Clear(void);		
void LCD_PORT_Init(void);
void LCD_Init(void);
void LCD_Display_Shift(unsigned char p);	
void LCD_Cursor_Shift(unsigned char p);		
void LCD_Cursor_Home(void);
void blink_OnOff(unsigned char a);

void LCD_Data(unsigned char ch)
{
    LCD_CTRL |= (1 << LCD_RS);    // RS=1, =0으로 데이터 쓰기 사이클
    LCD_CTRL &= ~(1 << LCD_RW);    
    LCD_CTRL |= (1 << LCD_EN);    // LCD Enable
    delay_us(50);                   // 시간지연
    LCD_WDATA = ch;                 // 데이터 출력
    delay_us(50);                   // 시간지연
    LCD_CTRL &= ~(1 << LCD_EN);     // LCD Disable
}

void LCD_Comm(unsigned char command)
{
    LCD_CTRL &= ~(1 << LCD_RS);     // RS==0으로 명령어 쓰기 사이클
    LCD_CTRL &= ~(1 << LCD_RW);    
    LCD_CTRL |= (1 << LCD_EN);      // LCD Enable 
    delay_us(50);                   // 시간지연
    LCD_WINST = command;            // 명령어 쓰기
    delay_us(50);                   // 시간지연
    LCD_CTRL &= ~(1 << LCD_EN);     // LCD Disable
}

void LCD_Delay(unsigned char ms)
{
    delay_ms(ms);
}

void LCD_Char(unsigned char ch)          // 한 문자 출력
{
    LCD_Delay(1);
    LCD_Data(ch);
}

void LCD_Num(int c) //한 숫자 출력
{
    if( c==0 ) LCD_Char(0x30);
    if( c==1 ) LCD_Char(0x31);
    if( c==2 ) LCD_Char(0x32);
    if( c==3 ) LCD_Char(0x33);
    if( c==4 ) LCD_Char(0x34);
    if( c==5 ) LCD_Char(0x35);
    if( c==6 ) LCD_Char(0x36);
    if( c==7 ) LCD_Char(0x37);
    if( c==8 ) LCD_Char(0x38);
    if( c==9 ) LCD_Char(0x39);        
}

void LCD_Str(unsigned char *str)        // 문자열 출력
{
    while(*str != 0) {
        LCD_Char(*str);
        str++;
    }
}

void LCD_Pos(unsigned char x, unsigned char y)  // LCD 포지션 설정, x와 y는 행과 열을 의미함.
{
    LCD_Comm(0x80|(x*0x40+y));                  // x = 가로행(0, 1), y = 세로열(0-15)
}

void LCD_Pos2(unsigned char x, unsigned char y)  // LCD 포지션 설정, x와 y는 행과 열을 의미함.
{
    LCD_Comm(col[x]+y);                  // x = 가로행(0, 1), y = 세로열(0-15)
}

void LCD_Clear(void)    // 화면 클리어 (1)
{
    LCD_Comm(0x01);
    LCD_Delay(2);
}

void LCD_PORT_Init(void)
{
    DDRA  = 0xFF;        // PORTA를 출력으로
    DDRG |= 0x0F;        // PORTG의 하위 4비트를 출력으로
}

void LCD_Init(void)     // LCD 초기화
{
    LCD_PORT_Init();
    
    LCD_Comm(0x38);     // DDRAM, 데이터 8비트사용, LCD 2열로 사용 (6)
    LCD_Delay(4);       // 4ms 지연
    LCD_Comm(0x38);     // DDRAM, 데이터 8비트사용, LCD 2열로 사용 (6)        
    LCD_Delay(4);       // 4ms 지연
    LCD_Comm(0x38);     // DDRAM, 데이터 8비트사용, LCD 2열로 사용 (6)
    LCD_Delay(4);       // 4ms 지연
    LCD_Comm(0x0e);     // Display ON/OFF
    LCD_Delay(2);       // 2ms 지연        
    LCD_Comm(0x06);     // 주소+1 , 커서를 우측 이동 (3)
    LCD_Delay(2);       // 2ms 지연        
    LCD_Clear();        // LCD 화면 클리어
}

void LCD_Display_Shift(unsigned char p)
{
    if(p==RIGHT){
        LCD_Comm(0x1c);
        LCD_Delay(1); // 시간지연
    }
    //표시 화면 전체를 왼쪽으로 이동
    else if(p == LEFT) {
        LCD_Comm(0x18);
        LCD_Delay(1);
    }
}

void LCD_Cursor_Shift(unsigned char p)
{
        if(p == RIGHT) {
                LCD_Comm(0x14);
                LCD_Delay(1);
        }
        else if(p == LEFT) {
                LCD_Comm(0x10);
                LCD_Delay(1);
        }
}

void LCD_Cursor_Home(void)
{
    LCD_Comm(0x02);
    LCD_Delay(2);
}

void blink_OnOff(unsigned char a)      //1 이면 블링크on , 0이면 블링크 off
{
    if(a==1)
        LCD_Comm(blink);
    else
        LCD_Comm(0x06);
}

