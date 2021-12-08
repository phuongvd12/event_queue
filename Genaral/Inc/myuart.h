#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "variable.h"


#ifndef __myuart_H
#define __myuart_H
/*======================== Structs ======================*/


/*======================== FUNCTION SWI ======================*/
uint32_t        Cal_Time(uint32_t Millstone_Time, uint32_t Systick_now);
uint32_t        Cal_Time_s(uint32_t Millstone_Time, uint32_t Systick_now);
bool            Check_Time_Out(uint32_t Millstone_Time, uint32_t Time_Period_ms);
bool            Check_Time_Out_2(uint32_t Millstone_Time, uint32_t Time_Period_ms);
void            Get_RTC(void);
void            Set_RTC(void);
uint8_t         Set_RTC_2(ST_TIME_FORMAT sRTC_Var);
/*
 * 		Find String
 */
int16_t         Find_String(uint8_t *s, uint8_t Length, uint8_t *Master, int16_t Area) ;
int16_t         Find_String_V2(truct_String *sTemp_Receiv,truct_String *sResponding);
int16_t         RC(int16_t Result,uint8_t Off_Set);
int16_t         Find_str_fix(int16_t pos_fix,truct_String *sTemp_Receiv,truct_String *sResponding,int16_t Pos_Res, int16_t *Pos_sharp);

/*
 * Copy String
 */
uint8_t	        Copy_String(truct_String *sTarget, truct_String *sCopy, uint8_t Length_Copy, uint8_t Length_Max);
uint8_t	        Copy_String_section(truct_String *sTarget, truct_String *sCopy, int Pos_Target, int Pos_Copy, int Length_Copy, int Length_Max);
uint8_t         Cut_String(truct_String *str_in,int *Pos_find,truct_String *Path);
uint8_t         Cut_Value(truct_String *str_in,int *Pos_find,uint32_t *Value);

/*======================== FUNCTION ======================*/
void            Init_UART(struct_Uart_Data *pUart, uint8_t *pAdd);
void            Reset_Buff(truct_String *str);
void            Reset_UART(struct_Uart_Data *pUart);
uint8_t	        Copy_String_2 (truct_String *sTarget, truct_String *sCopy);
uint8_t	        Copy_String_STime (truct_String *sTarget, ST_TIME_FORMAT Stime);

uint8_t	        Copy_String_toTaget (truct_String *sTarget, uint16_t Pos, truct_String *sCopy);
uint8_t         Compare_String (truct_String Str1, truct_String Str2);
void            ConvertStringsTime_tosTime (uint8_t* Buff, ST_TIME_FORMAT* Stime);

void            Copy_STime_fromsTime (ST_TIME_FORMAT *sTimeTaget, uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec);
int8_t          Check_DiffTime (ST_TIME_FORMAT sTime1, ST_TIME_FORMAT sTime2, uint32_t* DiffValue);
void            AddPoint_to_StringDec (truct_String *StrDec, uint8_t Scale);  


#endif /*  */

