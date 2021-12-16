

#ifndef __INIT_ALL_READMETER_
#define __INIT_ALL_READMETER_  __INIT_ALL_READMETER_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "onchipflash.h"

//-------------------------Define--------------------------
#define UART2_BAUR_1			9600
#define UART2_BAUR_2			2400
#define UART2_BAUR_3			300 

#define UART2_DATALENGTH		UART_WORDLENGTH_8B
#define UART2_DATALENGTH2		UART_WORDLENGTH_9B

#define UART2_PARITY1			UART_PARITY_NONE
#define UART2_PARITY2           UART_PARITY_EVEN
            
#define UART2_STOP_BIT          UART_STOPBITS_1




//-------------------------UART2--------------------------

typedef struct 
{
    uint32_t BaudRate;
    uint32_t WordLength; 
	uint32_t StopBits;
    uint32_t Parity;
} Uart_InterfaceInfo_t;



typedef enum  
{
    METER_TYPE_GELEX,
    METER_TYPE_103,
    METER_GELEX_103,
    METER_HHM_103,
    METER_VSE_103,
    METER_TYPE_LANDIS, 
    METER_TYPE_ELSTER, 
    METER_TYPE_START,
    METER_TYPE_GENIUS,
    METER_TYPE_CPC,  
//    METER_TYPE_STAR, 
    METER_TYPE_UNKNOWN,
} Meter_Type_2;
  

typedef void (*Meter_InitFunctional)(uint8_t Type);
typedef struct 
{
    uint8_t                 type;
	Uart_InterfaceInfo_t    uartInfo;
    Meter_InitFunctional    InitUartMeter;
	Meter_InitFunctional    initFunctional;
}Struct_Infor_Meter;

  
typedef struct UART2_Control_Struct
{
	uint16_t 	Mess_Length_ui16;
	uint8_t		Mess_Status_ui8;			//0x00-no mess			0x01-not check		0x02-checked-ok			0x04-check-fail
	uint8_t		Mess_Pending_ui8;			//0x00-no mess			0x02-have mess
}UART2_Control_Struct;

//-------------------------METER--------------------------




typedef struct Meter_Comm_Struct{
	uint8_t		Step_ui8;					//0x00-wait respond		0x01-send some mess
	uint8_t		Mess_Step_ui8;				//0-34
	uint8_t		ID_Frame_ui8;
	uint8_t		Reading_ui8;
	uint8_t		Error_ui8;					//0x00-no error			0x01-Meter no respond		0x02-Wrong mess format
	uint8_t		Total_OBIS_ui8;
	uint8_t		Num_OBIS_ui8;
	uint8_t		OBIS_Pointer_ui8;
	uint8_t		Num_Block_ui8;
	uint8_t		Reading_Block_ui8;
	uint8_t		Handle_Server_Request_ui8; 
    uint32_t	Flag_ui32;  //them
	uint8_t		Collecting_Object_ui8;   
	uint16_t	Num_Record_ui16;
	uint16_t	Received_Record_ui16;
	uint16_t	Data_Buff_Pointer_ui16;
	uint16_t	Last_Data_Buff_Pointer_ui16;  
	uint32_t	Time_Base_ui32;
	uint32_t	Total_Mess_Sent_ui32;
	uint32_t	Success_Read_Mess_ui32;
	uint32_t	Error_Meter_Norespond_ui32;
	uint32_t	Error_Wrong_Mess_Format_ui32;
    
    uint8_t     Flag_Start_Pack;
    uint16_t    Pos_Obis_Inbuff;
    uint16_t    Pos_Data_Inbuff;
    
    truct_String    Str_Payload;
    truct_String    Str_Payload_2;
    uint16_t        PosNumqty;
    uint8_t         Numqty;
    
    uint16_t        IndexRead;   //elseter
    uint8_t		    Flag_ui8;
    
    uint8_t		Mess_Count_ui8;  //start
    uint8_t		Mess_To_Send_ui8;
    uint8_t		First_Mess_Ok_ui8; 
    uint8_t		fTypeMessRead;       //Flag nay khong reset trong qua trinh doc. Danh dau ban tin dang doc la gi.  1: Opera, Intan
}Meter_Comm_Struct;

typedef struct Meter_ReadForAlert_Struct
{
	uint32_t	Power_Factor_ui32;
	uint32_t	Power_FactorA_ui32;
	uint32_t	Power_FactorB_ui32;
	uint32_t	Power_FactorC_ui32;
	uint32_t	Freq_ui32;
    uint32_t	FreqA_ui32;
	uint32_t	FreqB_ui32;
	uint32_t	FreqC_ui32;
	uint32_t	Vot_PhaseA_ui32;
	uint32_t	Vot_PhaseB_ui32;
	uint32_t	Vot_PhaseC_ui32;
	uint32_t	Cur_PhaseA_ui32;
	uint32_t	Cur_PhaseB_ui32;
	uint32_t	Cur_PhaseC_ui32;
	uint32_t	Count;
	uint32_t	Last_Alarm;
    uint32_t	Tu_ui32;
	uint32_t	Ti_ui32;
}Meter_ReadForAlert_Struct;

typedef struct Meter_AlertSetting_Struct
{
	uint32_t	Vot_PhaseA_Under_ui32;			//x100		(V)
	uint32_t	Vot_PhaseA_Over_ui32;			//x100		(V)
	uint32_t	Vot_PhaseB_Under_ui32;			//x100		(V)
	uint32_t	Vot_PhaseB_Over_ui32;			//x100		(V)
	uint32_t	Vot_PhaseC_Under_ui32;			//x100		(V)
	uint32_t	Vot_PhaseC_Over_ui32;			//x100		(V)
	uint32_t	Freq_Under_ui32;				//x100		(Hz)
	uint32_t	Freq_Over_ui32;					//x100		(Hz)
	uint32_t	Power_Factor_Under_ui32;		//x1000		()
	uint32_t	Cur1Phase_Over_Rated_ui32;		//x100		(A)
	uint32_t	Cur2Phase_Over_Rated_ui32;		//x100		(A)
	uint32_t	Cur1Phase_Under_Average_ui32;	//x100 		(%)
	uint32_t	Yield_Under_uin32;				//
	uint32_t	Yield_Over_uin32;				//
	uint32_t	Vot_Fail_ui32;					//x100 		(V)
	uint32_t	Curr_Fail_ui32;					//x100 		(A)
	uint32_t	Curr_Phase_imbalance_ui32;		//x100 		(%)
}Meter_AlertSetting_Struct;



typedef struct 
{
    uint8_t                 ObisHex;
    truct_String*     Str_Obis;
    uint8_t                 scale;
    truct_String*     Str_Unit;
    uint8_t                 Scale_ReadMeter;
    truct_String*     StrObis_Bill;
    truct_String*     StrObis_Lpf;
}struct_Obis_Scale;


typedef struct 
{
    uint8_t                 ID_event;
    truct_String*           Str_Obis;
}struct_Obis_event;


typedef uint8_t (*_f_ReadMeter)(void);
typedef uint8_t (*_f_Check)(uint32_t value);
typedef struct 
{
    _f_Check            _f_Read_ID;
    _f_Check            _f_Check_Reset_Meter;
    _f_ReadMeter        _f_Connect_Meter;
    _f_Check            _f_Read_TSVH;
    _f_ReadMeter        _f_Read_Bill;
    _f_Check            _f_Read_Event;
    _f_ReadMeter        _f_Read_Lpf;
    _f_ReadMeter        _f_Read_InforMeter;
    _f_ReadMeter        _f_Get_UartData;
    _f_ReadMeter        _f_Check_Meter;
    _f_ReadMeter        _f_Test1Cmd_Respond;
}struct_Func_ReadMeter;
    



extern struct 		Meter_Comm_Struct				Get_Meter_Info;
extern struct 		Meter_Comm_Struct				Get_Meter_Scale;
extern struct 		Meter_Comm_Struct				Get_Meter_Alert;
extern struct 		Meter_Comm_Struct				Get_Meter_Billing;
extern struct 		Meter_Comm_Struct				Get_Meter_Event;
extern struct 		Meter_Comm_Struct				Get_Meter_LProf;
extern struct       Meter_Comm_Struct				Get_Meter_TuTi;

extern struct 		Meter_ReadForAlert_Struct		Meter_ReadForAlert;
extern struct 		Meter_AlertSetting_Struct		Meter_AlertSetting,temp_Meter_AlertSetting;

extern struct_Func_ReadMeter         eMeter_20;
extern truct_String                  StrUartTemp;
extern truct_String                  StrNull;
/*======================== Constant ======================*/
extern const uint16_t 	             fcstab[256];
extern Struct_Infor_Meter           ListMeterInfor[];
extern  uint8_t 	Meter_Type; 
//-------------------------Init function--------------------
void        Init_UART2 (void);
void        Init_Meter_Struct(Meter_Comm_Struct* Meter_Struct, uint8_t* Buff1, uint8_t* Buff2);
void        Init_Meter_Info_Struct (void);
void        Init_Meter_Alert_Struct (void);
void        Init_Meter_Billing_Struct (void);
void        Init_Meter_Event_Struct (void);
void        Init_Meter_ReadForAlert_Val (void);
void        Init_Meter_LProf_Struct (void);
void        Init_Meter_Scale_Struct (void);
void        Init_Meter_AlertSetting_Val (void);
void        Init_MD_Bill(void);
void        Init_Meter_TuTi_Struct (void);


uint16_t    CountFCS16(uint8_t *buff, uint16_t index, uint16_t count);
uint8_t     getObjectCount(uint8_t *buff, uint8_t index);
void        Insert_Read_Time (uint8_t *aTime, uint8_t Year, uint8_t Month, uint8_t Day, uint8_t Hour, uint8_t Min, uint8_t Sec);
uint32_t    Nomalize_Read_Value (uint16_t RawVal, uint8_t ReadScale, uint16_t ServerSettingScale);

uint64_t    ConvertScaleMeter_toDec (uint8_t ScaleMeter, uint64_t* Div);
uint8_t     GetUART2Data(void);
void        ReInit_Uart_Meter(uint8_t type);
uint8_t     _fWaitSaveInFlash (uint8_t* FlagWait);
uint8_t     Scan_DetectMeter (void);
int8_t      Check_Obis_event (struct_Obis_event* StructEvent, uint8_t ID_Event, uint8_t MaxLength) ;
uint8_t     ConvertHex_to_Dec(uint8_t Hex);
void        Func_Scan_NewMeter (void);
void        Packet_Empty_MessHistorical (void);
uint8_t     _fSetStimeMeter_FromDCU (ST_TIME_FORMAT sRTC);
#endif