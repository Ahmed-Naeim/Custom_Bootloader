/**
 ******************************************************************************
 * @file           : bootloader.h
 * @author         : Ahmed Naeim
 * @brief          : Contains declaration for bootloader
 ******************************************************************************
**/
#ifndef INC_BOOTLOADER_BOOTLOADER_H_
#define INC_BOOTLOADER_BOOTLOADER_H_

/**********************************************Includes Start**********************************************/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "usart.h"
#include "crc.h"
/**********************************************Includes End**********************************************/

/**********************************************Macro Declaration Start**********************************************/

#define BL_DEBUG_UART             		  	 	&huart2
#define BL_HOST_COMMUNICATION_UART				&huart2
#define CRC_ENGINE_OBJ							&hcrc

#define BL_DEBUG_DISABLE						0
#define BL_DEBUG_ENABLE							1
#define BL_DEBUG_ENABLE							DEBUG_INFO_ENABLE

#define BL_ENABLE_UART_DEBUG_MESSAGE			0x00
#define BL_ENABLE_SPI_DEBUG_MESSAGE				0x01
#define BL_ENABLE_CAN_DEBUG_MESSAGE				0x02
#define BL_DEBUG_METHOD							(BL_ENABLE_UART_DEBUG_MESSAGE)

#define BL_HOST_BUFFER_RX_LENGTH 				200

/* Command Code Defines */
#define CBL_GET_VER_CMD							0x10
#define CBL_GET_HELP_CMD						0x11
#define	CBL_GET_CID_CMD							0x12
#define	CBL_GET_RDP_STATUS_CMD					0x13
#define	CBL_GO_TO_ADDR_CMD						0x14
#define	CBL_FLASH_ERASE_CMD						0x15
#define	CBL_MEM_WRITE_CMD						0x16
#define	CBL_ENABLE_R_W_PROTECT_CMD				0x17
#define	CBL_MEM_READ_CMD						0x18
#define	CBL_READ_PAGE_STATUS_CMD				0x19
#define	CBL_OTP_READ_CMD						0x20
#define	CBL_DIS_R_W_PROTECT_CMD					0x21

#define CBL_VENDOR_ID							100
#define CBL_SW_MAJOR_VERSION					1
#define CBL_SW_MINOR_VERSION					0
#define CBL_SW_PATCH_VERSION					0

#define CRC_TYPE_SIZE_BYTE						4

#define CRC_VERIFICATION_FAILED					0x00
#define CRC_VERIFICATION_PASSED					0x01

#define CBL_SEND_NACK							0xAB
#define CBL_SEND_ACK							0xCD

/*Start Address of page 2*/
#define FLASH_PAGE2_BASE_ADDRESS				0x08008000U

#define ADDRESS_IS_VALID						0x01
#define ADDRESS_IS_INVALID						0x00


#define STM32F103_FLASH_SIZE					(1024 * 64)
#define STM32F103_SRAM_SIZE						(1024 * 20)
#define STM32F103_FLASH_END						(FLASH_BASE + STM32F103_FLASH_SIZE)
#define STM32F103_SRAM_END						(SRAM_BASE + STM32F103_SRAM_SIZE)

#define CBL_FLASH_MAX_PAGE_NUMBER 				128					/*CBL --> Custom Bootloader*/
#define CBL_FLASH_MASS_ERASE					0xFF


#define INVALID_PAGE_NUMBER 					0x00
#define VALID_PAGE_NUMBER						0x01
#define UNSUCCESSFUL_ERASE						0x02
#define SUCCESSFUL_ERASE						0x03

#define HAL_SUCCESSFUL_ERASE					0xFFFFFFFF


/* CBL_MEM_WRITE_CMD */
#define FLASH_PAYLOAD_WRITE_FAILED   0x00
#define FLASH_PAYLOAD_WRITE_PASSED   0x01

#define FLASH_LOCK_WRITE_FAILED      0x00
#define FLASH_LOCK_WRITE_PASSED      0x01


/* CBL_GET_RDP_STATUS_CMD */
#define ROP_LEVEL_READ_INVALID       0x00
#define ROP_LEVE_READL_VALID         0X01

/* CBL_CHANGE_ROP_Level_CMD */
#define ROP_LEVEL_CHANGE_INVALID     0x00
#define ROP_LEVEL_CHANGE_VALID       0X01

#define CBL_ROP_LEVEL_0              0x00
#define CBL_ROP_LEVEL_1              0x01
#define CBL_ROP_LEVEL_2              0x02

/**********************************************Macro Declaration End**********************************************/



/**********************************************Macro Functions Start**********************************************/

/**********************************************Macro Functions End**********************************************/



/**********************************************Data Types Declaration Start**********************************************/

typedef enum{
	BL_NACK =0,
	BL_OK=1
}BL_Status;

typedef void (*pMainApp) (void);
typedef void (*JumpPtr) (void);

/**********************************************Data Types Declaration End**********************************************/


/**********************************************Software Interfaces Declaration Start**********************************************/

void BL_Print_Message(char *format, ...);
BL_Status BL_UART_Featch_Host_Command(void);

/**********************************************Software Interfaces Declaration End**********************************************/

#endif /* INC_BOOTLOADER_BOOTLOADER_H_ */
