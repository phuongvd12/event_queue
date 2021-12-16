#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "variable.h"


//===============Define address=================
#define ADDR_DCUID         			0x08004000   // sDCU.sDCU_id
#define ADDR_DEV_SERIAL    			0x08004800   //Serial
#define ADDR_MQTTSERVER       		0x08005000   //MQTT server IP & port
#define ADDR_METER_TYPE				0x08005800   //Meter Type Save
#define ADDR_APN_INFO    			0x08006000   //GPRS network APN info
#define ADDR_CONNECT_MODE   		0x08006800   //Modem connection mode
#define ADDR_CONNECT_TIME   		0x08007000   //Modem connect time
#define ADDR_CONNECT_PERIOD   		0x08007800   //Modem connect period
#define ADDR_DISCONNECT_TIME  		0x08008000   //Modem disconnect time
#define ADDR_DEVICE_IP    			0x08008800   //Device IP
#define ADDR_UART_CONFIG   			0x08009000   //Config UART port
#define ADDR_DUTY_HEARTB   			0x0800A000   //Duty Ping Heartbeat
#define ADDR_RADIO_FRE   			0x0800A800   //fix radio freq 4G GSM Auto 4Gpre GSMpre
#define ADDR_TIMESEND_LPF 			0x0800B000   //Config UART port
#define ADDR_PASS_MODERM   			0x0800B800   

#define	ADDR_MAIN_PROGRAM			0x0800C000	   //Page 51 -> page 132 = 82 pages = 164KB
#define	ADDR_UPDATE_PROGRAM			0x0803F000	   //Page 151->200k

#define	ADDR_FLAG_HAVE_NEW_FW		0x08003800	   //lay 2 dia chi phia tren de test truoc

#define	MAX_SIZE_FIRM				104            //page   .. Ca ca la 104 page. trong boot lai dang fix < 102 page
#define TIME_OUT_FTP				120000





/*------------------------------Function-----------------------------*/

HAL_StatusTypeDef	OnchipFlashWriteData (uint32_t andress, uint8_t	*data_address, uint32_t data_length);
HAL_StatusTypeDef	OnchipFlashCopy (uint32_t sourceandress, uint32_t destinationandress, uint32_t data_length);
HAL_StatusTypeDef 	OnchipFlashPageErase (uint32_t pageaddress);
void                Erase_Firmware(uint32_t Add_Flash_update,uint32_t Total_page);
void                OnchipFlashReadData (uint32_t address,uint8_t *destination, uint16_t length_inByte);

void                Save_DCU_ID (void);
//======================================================================
//				FTP
//======================================================================
void        Set_default_FTP(void);
void        _fFTP_Receiv(void);
void        _fFTP_Pending(void);
void        Save_Array(uint32_t ADD, uint8_t* Buff, uint16_t length);
void        Save_PassModerm (void);
void        _fSave_Uart_Config (void);
