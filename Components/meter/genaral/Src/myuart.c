#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "variable.h"
#include "myuart.h"
#include "usart.h"
#include "sim900.h"
#include "rtc.h"
#include "onchipflash.h"
#include "stdlib.h"
#include "t_mqtt.h"
#include "pushdata.h"

/*======================== Init Structs ======================*/

// Ham tra ve vi tri cua xau S trong chuoi Master
// Length : do dai cua xau s
// Master : Mang Dich Can tim kiem
// Area : khoang tim kiem xau S trong mang Dich (bat dau tu vtri 0)
// Ham  = -1 : neu ko tim thay chuoi
//		= i  : (0-32768) tim dc chuoi, i la vi tri chuoi
int16_t Find_String(uint8_t *s, uint8_t Length, uint8_t *Master, int16_t Area) 
{
	int16_t i,j;
	uint8_t Check = 1;
	if (Area <  Length) return -1;
	for (i = 0; i <= Area - Length; ++i) {
		if (*s == *(Master +i)) {// tim dc ky tu dau giong
			Check = 1;
			for (j = 1; j < Length; ++j)
				if (*(s+j) != *(Master + i + j)) Check = 0;
			if (Check == 1) return (i+1);
		}
	}
	return -1;
}

/* TESTED
 * Ham tra ve vi tri cua xau sTemp_Receiv trong chuoi sResponding
 * Ham  = -1 : neu ko tim thay chuoi
 * 		= i  : (0-32768) tim dc chuoi, i la vi tri chuoi
 * sTemp_Receiv : cau truc du lieu CHUAN can so sanh
 * 		- Dang du lieu vao "abc...|xy|zt" = tim 2 chuoi "abc...xy" va "abc...zt"
 * 		- Dang 2 : "!abc12"  : ham se tra ve TRUE (>=0) khi khong tim thay chuoi "abc12"
 * sResponding : chuoi nhan duoc qua UART
 * trong sTemp_Receiv chua ky tu "|" - phia sau la so sanh OR 
 * 	Note :
 * 		- do dai ham se ko phai la Do dai thuc su cua ham
 * 	ex : find s "CREG: 0,|1|5" = find 2 string "CREG: 0,1" va "CREG: 0,5"
 * 	Except :
 * 		- Khong tim duoc 2 doan ky tu trong sTemp_Receiv khac nhau hoan toan "|error|CPIN"
 */

int16_t Find_String_V2(truct_String *sTemp_Receiv,truct_String *sResponding)
{
    int16_t i = 0,j = 0;
    int16_t Pos_Sharp_first = 0, Pos_Sharp = 0;
    uint8_t Check;

	uint8_t Check_sharp = 0;
	uint8_t	Off_Set = 0;
	if (*sTemp_Receiv->Data_a8 == '!') Off_Set = 1;
	Check = 1;

	for (i = 0; i < sResponding->Length_u16; ++i) {
		if (*(sTemp_Receiv->Data_a8+Off_Set) == *(sResponding->Data_a8 +i)) {// tim dc ky tu dau giong
			Check 	= 1;
			j 		= 1 + Off_Set;
			Check_sharp = 0;
			Pos_Sharp = 0;
			while (j < sTemp_Receiv->Length_u16) {
				if (*(sTemp_Receiv->Data_a8+j) == '|') {
					if (Pos_Sharp == 0) {// Luu lai vi tri '|' dau tien
						Check_sharp = 1;
						Pos_Sharp_first = i+j;
					}
					Check = Find_str_fix(j+1 ,sTemp_Receiv,sResponding,Pos_Sharp_first,&Pos_Sharp);
					if (Check == 1) return RC(i,Off_Set);
				}
				if ((Check_sharp == 0) && (*(sTemp_Receiv->Data_a8+j) != *(sResponding->Data_a8 + i + j)))
				{
					Check = 0;
					break;
				}
				if (Check_sharp == 0) j++;
				else {
					if (j < Pos_Sharp) j = Pos_Sharp;
					else j++;
				}
			}
			if (Check == 1) return RC(i,Off_Set);
		}
	}
	return RC(-1,Off_Set);
}

int16_t RC(int16_t Result,uint8_t Off_Set)
{
	if (Off_Set == 0) return Result;
	if (Result >= 0) return -1;
	else return 0;
}

/*
 * return 	-1 	: neu ko tim thay chuoi
 * 			1	: Neu chuoi dung
 */

int16_t Find_str_fix(int16_t pos_fix,truct_String *sTemp_Receiv,truct_String *sResponding,int16_t Pos_Res, int16_t *Pos_sharp)
{
	 int16_t Result,count;
	 Result = 1,count = 0;
	 while ((pos_fix + count) < sTemp_Receiv->Length_u16) {
		  if (*(sTemp_Receiv->Data_a8+pos_fix+count) == '|') {
			   *Pos_sharp = pos_fix + count;
			   return Result;
		  }
		  if ((Pos_Res + count) > sResponding->Length_u16) { // Vuot qua Sau dich
			   *Pos_sharp = sResponding->Length_u16 + 1;
			   Result = -1;
		  }
		  if (*(sTemp_Receiv->Data_a8+pos_fix+count) != *(sResponding->Data_a8 + Pos_Res + count)) Result = -1;
		  count++;
	 }

	 return Result;
}

/*======================== FUNCTION ======================*/
bool Check_Time_Out(uint32_t Millstone_Time, uint32_t Time_Period_ms)
{	
	if (Cal_Time(Millstone_Time,RT_Count_Systick_u32) >= Time_Period_ms) return 1;
	
	return 0;
}

bool Check_Time_Out_2(uint32_t Millstone_Time, uint32_t Time_Period_ms)
{	
	if (Millstone_Time == 0) return 0;
	
	if (Cal_Time(Millstone_Time,RT_Count_Systick_u32) >= Time_Period_ms) return 1;
	
	return 0;
}
// Return : ms - time period (count Systick Timer)
uint32_t Cal_Time(uint32_t Millstone_Time, uint32_t Systick_now) 
{
	if (Systick_now < Millstone_Time)
		return (0xFFFFFFFF - Millstone_Time + Systick_now);
	return (Systick_now - Millstone_Time);
}	

// Return : second - time period (count Systick Timer)
uint32_t Cal_Time_s(uint32_t Millstone_Time, uint32_t Systick_now) 
{
	if (Systick_now < Millstone_Time)
		return ((0xFFFFFFFF - Millstone_Time + Systick_now)/1000);
	return ((Systick_now - Millstone_Time)/1000);
}	


void Init_UART(struct_Uart_Data *pUart, uint8_t *pAdd)
{
	pUart->str_Receiv.Data_a8 = pAdd;	// Gan gia tri con tro
	pUart->Time_Out_16u = 3000; //ms
	Reset_UART(pUart);
}

void Reset_Buff(truct_String *str)
{
	uint16_t i;
	for (i=0;i < str->Length_u16; i++)
		*(str->Data_a8+i) = 0;
	str->Length_u16 = 0;
}


void Reset_UART(struct_Uart_Data *pUart)
{
	if (pUart->OverStack == 1) {
		pUart->OverStack = 0;
	}
	pUart->Kind_Data_u8 = 0;	
	pUart->Pending = 0;
	pUart->Success = NONE;	
	pUart->Systick_Last_Recerver_u32 = 0;
	Reset_Buff(&pUart->str_Receiv);
}


/*
 * 		COPY STRING
 * Param :
 * 		- truct_String *sTarget : Chuoi su dung
 * 		- truct_String *sCopy	: chuoi bi Copy
 * 		- Length_Copy			: so Ky tu Copy tu *sCopy (=255 - Copy all)
 * 		- Length_Max			: Do dai toi da co the Copy
 * Return :
 * 		0 - FALSE 	: 	ERROR :tran chuoi copy vao,
 * 		1 - True 	:	Copy thanh con
 */
uint8_t	Copy_String(truct_String *sTarget, truct_String *sCopy, uint8_t Length_Copy, uint8_t Length_Max)
{
	uint8_t var;
	if (Length_Copy == 255) Length_Copy = sCopy->Length_u16;
    
	if ((sTarget->Length_u16 + Length_Copy) >= Length_Max) {		
		return 0;
	}
	for (var = 0; var < Length_Copy; ++var) {
		*(sTarget->Data_a8 +sTarget->Length_u16 + var) = *(sCopy->Data_a8 + var);
	}
	sTarget->Length_u16 +=  Length_Copy;
	return 1;
}

uint8_t	Copy_String_section(truct_String *sTarget, truct_String *sCopy, int Pos_Target, int Pos_Copy, int Length_Copy, int Length_Max)
{	
	uint16_t var;
	if ((sTarget->Length_u16 + Length_Copy) >= Length_Max) 
    {
		return 0;
	}
	for (var = 0; var < Length_Copy; ++var) {
		*(sTarget->Data_a8 + var + Pos_Target) = *(sCopy->Data_a8 + var + Pos_Copy);
	}
	sTarget->Length_u16 +=  Length_Copy;
	return 1;
}

/*
 * Param :
 * 		str_in : chuoi dau vao can chia
 * 		Pos_find : bi tri bat dau tim trong str_in, va vi tri tra ve sau khi tim
 * 		Path : chuoi sau khi da cat ra
 * 	Return :
 * 		> 0 : OK
 * 		0 : Not find
 */

uint8_t Cut_String(truct_String *str_in,int *Pos_find,truct_String *Path)
{
	int Pos_Copy;
	int Length_Copy;
	Pos_Copy = *Pos_find;
// Find PATH
	Pos_Copy++;
	Length_Copy = *(str_in->Data_a8 + Pos_Copy);
	Pos_Copy++;	
	if (Copy_String_section(Path,str_in,0,Pos_Copy,Length_Copy,LENGTH_FTP) == FALSE) return 0;
	Pos_Copy += Path->Length_u16;
	if (Pos_Copy > str_in->Length_u16) return 0;	
	*Pos_find = Pos_Copy;
	return 1;
}

/*
 * return
 *   0 : false
 *   >0 : OK
 */

uint8_t Cut_Value(truct_String *str_in,int *Pos_find,uint32_t *Value)
{	
    int Pos_Copy;
    int Length_Copy;
	// Find PATH
	Pos_Copy = *Pos_find;
	Pos_Copy++;
	*Value = 0;
	Length_Copy = *(str_in->Data_a8 + Pos_Copy);
	if (Length_Copy > 4) return 0;
	while (Length_Copy > 0) {
		Length_Copy--;
		Pos_Copy++;
		*Value = *Value << 8;
		*Value |= *(str_in->Data_a8 + Pos_Copy);
	}
	*Pos_find = Pos_Copy;
	return 1;
}

void Get_RTC(void)
{
    HAL_RTC_GetTime(&hrtc,&sRTCTime,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&sRTCDate,RTC_FORMAT_BIN);
	sRTC.sec 	= sRTCTime.Seconds;
	sRTC.min 	= sRTCTime.Minutes;
	sRTC.hour 	= sRTCTime.Hours;
	sRTC.day 	= sRTCDate.WeekDay;
	sRTC.date 	= sRTCDate.Date;
	sRTC.month 	= sRTCDate.Month;
	sRTC.year 	= sRTCDate.Year;
}

void Set_RTC(void)
{
	sRTCTime.Seconds 	= sRTC.sec;
	sRTCTime.Minutes 	= sRTC.min;
	sRTCTime.Hours 		= sRTC.hour;
	sRTCDate.WeekDay 	= sRTC.day;
	sRTCDate.Date 		= sRTC.date;
	sRTCDate.Month 		= sRTC.month;
	sRTCDate.Year 		= sRTC.year;
	HAL_RTC_SetTime(&hrtc,&sRTCTime,RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc,&sRTCDate,RTC_FORMAT_BIN);
}


uint8_t Set_RTC_2(ST_TIME_FORMAT sRTC_Var)
{
    RTC_TimeTypeDef 		sRTCTime_Temp = {0};
    RTC_DateTypeDef 		sRTCDate_Temp = {0};

    if(Check_update_Time(&sRTC_Var) == TRUE)
    {
        sRTCTime_Temp.SubSeconds     = sRTC_Var.SubSeconds;
        sRTCTime_Temp.Seconds 	     = sRTC_Var.sec;
        sRTCTime_Temp.Minutes        = sRTC_Var.min;
        sRTCTime_Temp.Hours 		 = sRTC_Var.hour;
        sRTCDate_Temp.WeekDay 	     = sRTC_Var.day;
        sRTCDate_Temp.Date 		     = sRTC_Var.date;
        sRTCDate_Temp.Month 		 = sRTC_Var.month;
        sRTCDate_Temp.Year 		     = sRTC_Var.year;



        HAL_RTC_SetTime(&hrtc,&sRTCTime_Temp,RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc,&sRTCDate_Temp,RTC_FORMAT_BIN);
        return 1;
    }
    return 0;
}


uint8_t	Copy_String_2 (truct_String *sTarget, truct_String *sCopy)
{
	uint16_t var;
    
	for (var = 0; var < sCopy->Length_u16; ++var) {
		*(sTarget->Data_a8 +sTarget->Length_u16 + var) = *(sCopy->Data_a8 + var);
	}
	sTarget->Length_u16 +=  sCopy->Length_u16;
    
	return 1;
}

//copy 1 chuoi str vao 1 vi tri o trong chuoi dich. Pos <= length taget
uint8_t	Copy_String_toTaget (truct_String *sTarget, uint16_t Pos, truct_String *sCopy)
{
	uint16_t var;
    
    if(Pos > sTarget->Length_u16) return 0;
    
    //dich chuoi taget ra sau "length vi tri" can copy vao
    for(var = sTarget->Length_u16; var > Pos; var--)
    {
        *(sTarget->Data_a8 + var + sCopy->Length_u16 - 1) = *(sTarget->Data_a8 + var - 1);
    }
    
    sTarget->Length_u16 += sCopy->Length_u16;
    //ghi giá tri chuoi copy vao vi tri pos
    
	for (var = 0; var < sCopy->Length_u16; ++var) 
    {
		*(sTarget->Data_a8 +Pos + var) = *(sCopy->Data_a8 + var);
	}
    
	return 1;
}


uint8_t	Copy_String_STime (truct_String *sTarget, ST_TIME_FORMAT Stime)
{
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.year/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.year%10 + 0x30);
    
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.month/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.month%10 + 0x30);
    
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.date/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.date%10 + 0x30);
    
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.hour/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.hour%10 + 0x30);
    
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.min/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.min%10 + 0x30);
    
	return 1;
}


uint8_t Compare_String (truct_String Str1, truct_String Str2)
{
    uint16_t  i = 0;
    
    if(Str1.Length_u16 != Str2.Length_u16) return 0;
    
    for(i = 0; i < Str1.Length_u16; i++)
      if(*(Str1.Data_a8 + i) != *(Str2.Data_a8 + i)) return 0;
    
    return 1;
}


void ConvertStringsTime_tosTime (uint8_t* Buff, ST_TIME_FORMAT* Stime)
{
    //convert thoi gian
    Stime->year  = (Buff[0] - 0x30) * 10 + (Buff[1] - 0x30);
    Stime->month = (Buff[2] - 0x30) * 10 + (Buff[3] - 0x30);
    Stime->date  = (Buff[4] - 0x30) * 10 + (Buff[5]- 0x30); 
    
    Stime->hour  = (Buff[6] - 0x30) * 10 + (Buff[7] - 0x30);
    Stime->min   = (Buff[8] - 0x30) * 10 + (Buff[9] - 0x30);
    Stime->sec   = (Buff[10] - 0x30) * 10 + (Buff[11]- 0x30); 
}


void Copy_STime_fromsTime (ST_TIME_FORMAT *sTimeTaget, uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec)
{
    sTimeTaget->year = year;
    sTimeTaget->month = month;
    sTimeTaget->date = date;
    sTimeTaget->hour = hour;
    sTimeTaget->min = min;
    sTimeTaget->sec = sec;
}



//DiffValue: sai khac cua stime2 -stime1.

int8_t Check_DiffTime (ST_TIME_FORMAT sTime1, ST_TIME_FORMAT sTime2, uint32_t* DiffValue)    //1-2
{
    uint32_t Sec1 = 0;
    uint32_t Sec2 = 0;
    
    Sec1 = HW_RTC_GetCalendarValue_Second(sTime1 ,0);   
    Sec2 = HW_RTC_GetCalendarValue_Second(sTime2, 0);
    
    if(Sec1 >= Sec2) 
    {
        *DiffValue = 0;
        return 1;
    }else
    {
        *DiffValue = Sec2 - Sec1;
    }
    return 0;
}



//them dau . thap phan vao trong chuoi Decimal cua 1 so

void AddPoint_to_StringDec (truct_String *StrDec, uint8_t Scale)
{
    uint16_t i = 0;
    uint16_t j = 0;
    uint8_t Count = 0;
    
    if((StrDec->Length_u16 == 1) && (*(StrDec->Data_a8) == 0x30)) return;
    
    if(StrDec->Length_u16 >= (Scale + 1))
    {
        if(Scale != 0)
        {
            //chuyen cac chu so dich ra sau 1 de lay 1 vi tri cho
            for(i = 0; i < Scale; i++)
            {
                *(StrDec->Data_a8 + StrDec->Length_u16 - i) = *(StrDec->Data_a8 + StrDec->Length_u16 - i - 1);
            }
        
            *(StrDec->Data_a8 + StrDec->Length_u16 - i) = '.';                                                           
            //them dau cham vao vi tri da chon
            StrDec->Length_u16++;
            //neu sau dau. có cac so 0 bi thua thi bo di
            for(j = (StrDec->Length_u16 - 1); j >= (StrDec->Length_u16 - 1 - Scale); j--)
            {
              if(*(StrDec->Data_a8 + j) == '0')
              {
                  Count++;  
              }else if(*(StrDec->Data_a8 + j) == '.')
              {
                  Count++;
                  break;
              }
              else break;
            }
            
            StrDec->Length_u16 -= Count;
        } 
    }else
    {
        if(Scale!= 0)
        {
            //chuyen cac chu so dich ra sau: (scale - strDecLength) + 2.vi tri
            for(i = 0; i < StrDec->Length_u16; i++)
            {
                *(StrDec->Data_a8 + Scale + 1 - i) = *(StrDec->Data_a8 + StrDec->Length_u16 - i - 1);
            }
        
            //ghi them 0. vao
            *(StrDec->Data_a8 + 0) = '0';
            *(StrDec->Data_a8 + 1) = '.';
            for(i = 0; i < (Scale - StrDec->Length_u16); i++)
                *(StrDec->Data_a8 + 2 + i) = '0';
                
            StrDec->Length_u16 = Scale + 2;
            //neu sau dau. có cac so 0 bi thua thi bo di
            for(j = (StrDec->Length_u16 - 1); j >= 1 ; j--)
              if(*(StrDec->Data_a8 + j) == '0')
              {
                  Count++;  
              }else if(*(StrDec->Data_a8 + j) == '.')
              {
                  Count++;
                  break;
              }
              else  break;
            
            StrDec->Length_u16 -= Count;
        }
    }
    
}




