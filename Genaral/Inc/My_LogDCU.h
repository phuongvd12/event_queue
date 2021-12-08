

#ifndef __My_LogDCU_H
#define __My_LogDCU_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "variable.h"
#include "at_commands.h"

/*-----------------------------Define---------------*/
#define         MAX_INDEX_LOG               ((ADDR_TOP_LOG_DCU - ADDR_BASE_LOG_DCU)/MAX_LOG_MESS + 1)         //999 
#define         MAX_LENGTH_SAVE_LOG         190
#define         MAX_LENGTH_BUFF_LOG_TOTAL   1024

/*-----------------------------Extern var struct---------------*/
typedef struct {
    truct_String        LogUart;
    uint16_t            CountRead;
    uint16_t            IndexStart;
    uint16_t            IndexStop;
    uint8_t             fReq_SendLog;
    uint32_t            Landmark_SendLog;
    truct_String        strLogTotal;
    uint8_t             IsLogParamImport;
    truct_String        strCutLog;
}struct_Log_Manage;


extern struct_Log_Manage                 sLogDCU;

/*-----------------------------Function---------------*/
void            _fPackStringToLog (uint8_t* Buff, uint8_t length);
void            InitLogDCU_Var (void);
uint8_t         _fReadLogDCU (uint16_t Index, uint8_t ReqBy);

//Giai doan 2
void            Check_Request_Read_Log (void);
uint8_t         fReadIndexLog_to_Serial (uint16_t IndexStart, uint16_t IndexStop);
uint8_t         fReadIndexLog_to_Server (uint16_t IndexStart, uint16_t IndexStop);
uint8_t         Check_New_Log_Modem (void);
void            _fLogDCU_ParaImportant (void);
void            Log_Last_100Byte_UartSim (truct_String strUart);
uint8_t         Packet_Data_Log_DCU_To_Form_Hes (truct_String *str);

#endif 