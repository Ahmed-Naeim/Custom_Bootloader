/**
 ******************************************************************************
 * @file           : bootloader.c
 * @author         : Ahmed Naeim
 * @brief          : Contains static code for bootloader
 ******************************************************************************
**/

#include "Bootloader/bootloader.h"



/*****************************************Global Variables Start*****************************************/

static uint8_t BL_HOST_BUFFER[BL_HOST_BUFFER_RX_LENGTH];		/* Array where I will receive the data */


static uint8_t Bootloader_Supported_CMDs[12] = {
    CBL_GET_VER_CMD,
    CBL_GET_HELP_CMD,
    CBL_GET_CID_CMD,
    CBL_GET_RDP_STATUS_CMD,
    CBL_GO_TO_ADDR_CMD,
    CBL_FLASH_ERASE_CMD,
    CBL_MEM_WRITE_CMD,
	CBL_ENABLE_R_W_PROTECT_CMD,
    CBL_MEM_READ_CMD,
    CBL_READ_PAGE_STATUS_CMD,
    CBL_OTP_READ_CMD,
	CBL_DIS_R_W_PROTECT_CMD
};

/*****************************************Global Variables End*****************************************/



/*****************************************Static Functions Declarations Start*****************************************/
static void Bootloader_Get_Version(uint8_t *Host_Buffer);
static void Bootloader_Get_Help(uint8_t *Host_Buffer);
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer);
static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer);
static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer);
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);
static void Bootloader_Memory_Write(uint8_t *Host_Buffer);
static void Bootloader_Enable_RW_Protection(uint8_t *Host_Buffer);
static void Bootloader_Memory_Read(uint8_t *Host_Buffer);
static void Bootloader_Get_Page_Protection_Status(uint8_t *Host_Buffer);
static void Bootloader_Read_OTP(uint8_t *Host_Buffer);
static void Bootloader_Change_Read_Protection_Level(uint8_t *Host_Buffer);

static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC);
static void Bootloader_Send_ACK(uint8_t Reply_Len);
static void Bootloader_Send_NACK();
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len);


/*****************************************Static Functions Declarations End*****************************************/


/*****************************************Software Interface Implementation Start*****************************************/

BL_Status BL_UART_Featch_Host_Command(void){
	/*
	 * Host Command Format:
	 * Command Length (1 byte =N) + Command Code (1 Byte) + Details (N Bytes) as Memory address or page number + CRC (4 Bytes)
	 * */

	BL_Status Status =BL_NACK;
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint8_t Data_Length = 0;

	memset(BL_HOST_BUFFER,0,BL_HOST_BUFFER_RX_LENGTH);

	HAL_Status = HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, BL_HOST_BUFFER, 1, HAL_MAX_DELAY);
	if (HAL_Status != HAL_OK)
	{
		Status = BL_NACK;
	}
	else{
		Data_Length = BL_HOST_BUFFER[0];				/* Put number of bytes to make bootloader receive from the host in the first index */

		HAL_Status = HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, &BL_HOST_BUFFER[1], Data_Length, HAL_MAX_DELAY); /* start from index [1] to write the command code*/
		if (HAL_Status != HAL_OK){
			Status = BL_NACK;
		}
		else{
			switch(BL_HOST_BUFFER[1]){
			case CBL_GET_VER_CMD:
				BL_Print_Message("Read the bootloader version from the MCU !! \r\n");
				Bootloader_Get_Version(BL_HOST_BUFFER);
				Status = BL_OK;
				break;

			case CBL_GET_HELP_CMD:
				BL_Print_Message("Read the commands supported by the bootloader !! \r\n");
				Bootloader_Get_Help(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			case CBL_GET_CID_CMD:
				BL_Print_Message("Read the MCU chip identification number !! \r\n");
				Bootloader_Get_Chip_Identification_Number(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			case CBL_GET_RDP_STATUS_CMD:
				BL_Print_Message("Read the Flash Read protection level !! \r\n");
				Bootloader_Read_Protection_Level(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			case CBL_GO_TO_ADDR_CMD:
				BL_Print_Message("Jump bootloader into specified address !! \r\n");
				Bootloader_Jump_To_Address(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			case CBL_FLASH_ERASE_CMD:
				BL_Print_Message("Mass erase or page erase to the user flash !! \r\n");
				Bootloader_Erase_Flash(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			case CBL_MEM_WRITE_CMD:
				BL_Print_Message("Write data into different memories in MCU !! \r\n");
				Bootloader_Memory_Write(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			case CBL_ENABLE_R_W_PROTECT_CMD:
				BL_Print_Message("Enable Read/Write protect on different pages of user flash !! \r\n");
				Bootloader_Enable_RW_Protection(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			case CBL_MEM_READ_CMD:
				BL_Print_Message("Read data form different memories in MCU !! \r\n");
				Bootloader_Memory_Read(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			case CBL_READ_PAGE_STATUS_CMD:
				BL_Print_Message("Read all pages protection status !! \r\n");
				Bootloader_Get_Page_Protection_Status(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			case CBL_OTP_READ_CMD:
				BL_Print_Message("Read the OTP content !! \r\n");
				Bootloader_Read_OTP(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			case CBL_DIS_R_W_PROTECT_CMD:
				BL_Print_Message("Disable Read/Write protect on different pages of user flash !! \r\n");
				Bootloader_Change_Read_Protection_Level(BL_HOST_BUFFER);
				Status = BL_OK;

				break;

			default:
				BL_Print_Message("Invalid Code Command Received from the Host !! \r\n");
				break;

			}
		}
	}

	return Status;
}


void BL_Print_Message(char *format, ...){
	char Messsage[100] = {0};
	/* holds the information needed by va_start, va_arg, va_end */
	va_list args;
	/* Enables access to the variable arguments */
	va_start(args, format);
	/* Write formatted data from variable argument list to string */
	vsprintf(Messsage, format, args);
#if (BL_DEBUG_METHOD == BL_ENABLE_UART_DEBUG_MESSAGE)
	/* Trasmit the formatted data through the defined UART */
	HAL_UART_Transmit(BL_DEBUG_UART, (uint8_t *)Messsage, sizeof(Messsage), HAL_MAX_DELAY);
#elif (BL_DEBUG_METHOD == BL_ENABLE_SPI_DEBUG_MESSAGE)
	/* Trasmit the formatted data through the defined SPI */
#elif (BL_DEBUG_METHOD == BL_ENABLE_CAN_DEBUG_MESSAGE)
	/* Trasmit the formatted data through the defined CAN */
#endif
	/* Performs cleanup for an object initialized by a call to va_start */
	va_end(args);
}
/*****************************************Software Interface Implementation End*****************************************/


/*****************************************Static Functions Implementation Start*****************************************/

static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len, uint32_t Host_CRC){
	uint8_t CRC_Status = CRC_VERIFICATION_FAILED;
	uint32_t MCU_CRC_Calculated = 0;
	uint8_t Data_Counter =0;
	uint32_t Data_buffer = 0;			/*because function waits for uint32_t parameter but the input is uint8_t *pData*/

	/*Calculate CRC32*/
	for (Data_Counter = 0; Data_Counter < Data_Len; ++Data_Counter) {
		Data_buffer = (uint32_t) pData[Data_Counter];		/*explicit casting to turn uint8_t into uint32_t*/
		MCU_CRC_Calculated = HAL_CRC_Accumulate(CRC_ENGINE_OBJ, &Data_buffer, 1); /*1 because it is already Data_len because the for loop*/
	}

	/*Reset CRC Calculation Unit*/
	__HAL_CRC_DR_RESET(CRC_ENGINE_OBJ);


	/*Compare the host CRC and Calculated CRC */
	if(MCU_CRC_Calculated == Host_CRC){
		CRC_Status = CRC_VERIFICATION_PASSED;
	}
	else{
		CRC_Status = CRC_VERIFICATION_FAILED;
	}

	return CRC_Status;

}

static void Bootloader_Send_ACK(uint8_t Reply_Len){
	uint8_t Ack_Value [2] = {0};
	Ack_Value [0] = CBL_SEND_ACK;
	Ack_Value [1] =Reply_Len;
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART, (uint8_t *)Ack_Value, 2, HAL_MAX_DELAY);

}

static void Bootloader_Send_NACK(){
	uint8_t Ack_Value = CBL_SEND_NACK;
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART, &Ack_Value, 1, HAL_MAX_DELAY);
}

static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len){
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART, Host_Buffer, 4, HAL_MAX_DELAY);

}

static void Bootloader_Get_Version(uint8_t *Host_Buffer){
	uint8_t BL_Version[4] = {CBL_VENDOR_ID,CBL_SW_MAJOR_VERSION,CBL_SW_MINOR_VERSION,CBL_SW_PATCH_VERSION};
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;

#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Read The bootloader version from the MCU !! \r\n");

#endif

	/*Extract CRC32 and Packet Length sent by the host*/

	Host_CMD_Packet_Len = Host_Buffer[0] +1 ;
	Host_CRC32 = *((uint32_t *) (Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE_BYTE));

	/*CRC Verificatipn*/
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] ,Host_CMD_Packet_Len - 4 ,Host_CRC32)){

#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("CRC Verification Passed !! \r\n");

#endif
		Bootloader_Send_ACK(4);
		Bootloader_Send_Data_To_Host((uint8_t *) BL_Version, 4);
	}
	else{

#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed !! \r\n");

#endif
		Bootloader_Send_NACK();
	}
}
static void Bootloader_Get_Help(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Read the commands supported by the bootloader \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		Bootloader_Send_ACK(12);
		Bootloader_Send_Data_To_Host((uint8_t *)(&Bootloader_Supported_CMDs[0]), 12);
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		Bootloader_Send_NACK();
	}
}
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
	uint16_t MCU_Identification_Number = 0;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Read the MCU chip identification number \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		/* Get the MCU chip identification number */
		MCU_Identification_Number = (uint16_t)((DBGMCU->IDCODE) & 0x00000FFF);
		/* Report chip identification number to HOST */
		Bootloader_Send_ACK(2);
		Bootloader_Send_Data_To_Host((uint8_t *)&MCU_Identification_Number, 2);
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		Bootloader_Send_NACK();
	}
}

static void Bootloader_jump_to_user_app(void){
	/*Value of the main stack pointer of our main application */
	uint32_t MSP_Value = *((volatile uint32_t *)FLASH_PAGE2_BASE_ADDRESS);
	/*Reset Handler Definition Function of our main application */
	uint32_t MainAppAddr = *((volatile uint32_t *) (FLASH_PAGE2_BASE_ADDRESS + 4));
	/*Fetch the reset handler address of the user application */
	pMainApp ResetHandler_Address = (pMainApp) MainAppAddr;

	/*Set Main Stack Pointer*/
	__set_MSP(MSP_Value);

	/*Deinitialize of Modules*/
	HAL_RCC_DeInit();						/*MANTADORY*/ /*Resets the RCC clock configuration to the default reset state.*/

	/*Jump to Application reset handler*/
	ResetHandler_Address();
}

static uint8_t CBL_STM32F103_Get_RDP_Level(){
	FLASH_OBProgramInitTypeDef FLASH_OBProgram;
	/* Get the Option byte configuration */
	HAL_FLASHEx_OBGetConfig(&FLASH_OBProgram);

	return (uint8_t)(FLASH_OBProgram.RDPLevel);

}

static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
	uint8_t RDP_Level = 0;

#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Read the FLASH Read Protection Out level \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		Bootloader_Send_ACK(1);
		/* Read Protection Level */
		RDP_Level = CBL_STM32F103_Get_RDP_Level();
		/* Report Valid Protection Level */
		Bootloader_Send_Data_To_Host((uint8_t *)&RDP_Level, 1);
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		Bootloader_Send_NACK();
	}
}

static uint8_t Host_Jump_Address_Verification(uint32_t Jump_Address){
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	if((Jump_Address >= SRAM_BASE) && (Jump_Address <= STM32F103_SRAM_END))
	{
		Address_Verification = ADDRESS_IS_VALID;
	}
	if((Jump_Address >= FLASH_BASE) && (Jump_Address <= STM32F103_FLASH_END))
	{
		Address_Verification = ADDRESS_IS_VALID;
	}
	else{
		Address_Verification = ADDRESS_IS_INVALID;
	}


	return Address_Verification;

}
static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
	uint32_t HOST_Jump_Address = 0;
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Read the commands supported by the bootloader \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		Bootloader_Send_ACK(1);
		/*extract address from the host from host packet*/
		HOST_Jump_Address = *((uint32_t *) &Host_Buffer [2]);

		/*Verify Address is Valid*/
		Address_Verification = Host_Jump_Address_Verification(HOST_Jump_Address);
		if(ADDRESS_IS_VALID == Address_Verification)
		{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Address verification succeeded \r\n");
#endif
			/*address verification succeeded*/
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
			/*prepare address to jump*/
			JumpPtr Jump_Address = (JumpPtr) (HOST_Jump_Address + 1);
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Jump to ox%X \r\n",Jump_Address);
#endif
			Jump_Address();
		}
		else
		{
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
		}

	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		Bootloader_Send_NACK();
	}
}


static uint8_t Perform_Flash_Erase (uint8_t Page_Number, uint16_t Number_of_Pages){

	uint8_t Page_Validity_Status = INVALID_PAGE_NUMBER;
	FLASH_EraseInitTypeDef pEraseInit;
	uint8_t Remaining_Pages = 0;
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint32_t Page_Error = 0;

	if(Number_of_Pages > CBL_FLASH_MAX_PAGE_NUMBER)
	{
		/*Number of Pages is out of range*/
		Page_Validity_Status = INVALID_PAGE_NUMBER;
	}
	else{
		Page_Validity_Status = VALID_PAGE_NUMBER;
		if((Page_Number <= (CBL_FLASH_MAX_PAGE_NUMBER -1)) || (CBL_FLASH_MASS_ERASE == Page_Number)){
			if(CBL_FLASH_MASS_ERASE == Page_Number)
			{
				/*Flash MASS ERASE activation*/
				pEraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Flash MASS ERASE activation \r\n");
#endif
			}
			else{
				/*Pages Erase ONLY*/
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Flash MPages Erase ONLY activation \r\n");
#endif
				Remaining_Pages = CBL_FLASH_MAX_PAGE_NUMBER - Page_Number;
				/*If user entered more pages than the available number from the page number entered*/
				if(Number_of_Pages > Remaining_Pages){
					Number_of_Pages = Remaining_Pages;
				}
				else{
					/*Nothing*/
				}
				pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
				pEraseInit.PageAddress = Page_Number; 								/*Initial FLASH page address to erase when mass erase is disabled*/
				pEraseInit.NbPages = Number_of_Pages; 								/*Number of pages to be erased*/


			}

			pEraseInit.Banks = 3U;													 /*Both Bank 1 and Bank 2*/

			HAL_Status = HAL_FLASH_OB_Unlock();										/*Unlock Flash control register*/
			HAL_Status = HAL_FLASHEx_Erase(&pEraseInit, &Page_Error);				/*Erase Flash memory pages*/

			if(HAL_SUCCESSFUL_ERASE == Page_Error){
				Page_Validity_Status = SUCCESSFUL_ERASE;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("SUCCESSFUL ERASE \r\n");
#endif

			}
			else{
				Page_Validity_Status = UNSUCCESSFUL_ERASE;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("UNSUCCESSFUL ERASE \r\n");
#endif
			}
			HAL_Status = HAL_FLASH_Lock();											/*Lock Flash control register*/
		}
		else{
			Page_Validity_Status = UNSUCCESSFUL_ERASE;
		}


	}
	return Page_Validity_Status;
}

static void Bootloader_Erase_Flash(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
	uint8_t Erase_Status = 0;

#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Mass erase or Page erase to the user flash !! \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));

	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		Bootloader_Send_ACK(1);
		Erase_Status = Perform_Flash_Erase(Host_Buffer[2],Host_Buffer[3]);
		if(SUCCESSFUL_ERASE == Erase_Status){
			/*report Erase Passed*/
			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
		}
		else{
			/*report Erase Failed*/
			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
		}

	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		Bootloader_Send_NACK();
	}

}


static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint32_t Payload_Start_Address, uint16_t Payload_Len){
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	uint16_t Payload_Counter = 0;

	/* Unlock the FLASH control register access */
  HAL_Status = HAL_FLASH_Unlock();

	if(HAL_Status != HAL_OK){
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	}
	else{
		for(Payload_Counter = 0; Payload_Counter < Payload_Len; Payload_Counter++){
			/* Program a byte at a specified address */
			HAL_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Payload_Start_Address + Payload_Counter, Host_Payload[Payload_Counter]);
			if(HAL_Status != HAL_OK){
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
				break;
			}
			else{
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED;
			}
		}
	}

	if((FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status) && (HAL_OK == HAL_Status)){
		/* Locks the FLASH control register access */
		HAL_Status = HAL_FLASH_Lock();
		if(HAL_Status != HAL_OK){
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
		}
		else{
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED;
		}
	}
	else{
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	}

	return Flash_Payload_Write_Status;
}

static void Bootloader_Memory_Write(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
	uint32_t Host_CRC32 = 0;
	uint32_t HOST_Address = 0;
	uint8_t Payload_Len = 0;
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;

#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Write data into different memories of the MCU \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		/* Send acknowledgement to the HOST */
		Bootloader_Send_ACK(1);
		/* Extract the start address from the Host packet */
		HOST_Address = *((uint32_t *)(&Host_Buffer[2]));
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("HOST_Address = 0x%X \r\n", HOST_Address);
#endif
		/* Extract the payload length from the Host packet */
		Payload_Len = Host_Buffer[6];
		/* Verify the Extracted address to be valid address */
		Address_Verification = Host_Jump_Address_Verification(HOST_Address);
		if(ADDRESS_IS_VALID == Address_Verification){
			/* Write the payload to the Flash memory */
			Flash_Payload_Write_Status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[7], HOST_Address, Payload_Len);
			if(FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status){
				/* Report payload write passed */
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
				BL_Print_Message("Payload Valid \r\n");
#endif
			}
			else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
				BL_Print_Message("Payload InValid \r\n");
#endif
				/* Report payload write failed */
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);
			}
		}
		else{
			/* Report address verification failed */
			Address_Verification = ADDRESS_IS_INVALID;
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
		}
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		/* Send Not acknowledge to the HOST */
		Bootloader_Send_NACK();
	}
}
static void Bootloader_Enable_RW_Protection(uint8_t *Host_Buffer){

}
static void Bootloader_Memory_Read(uint8_t *Host_Buffer){

}
static void Bootloader_Get_Page_Protection_Status(uint8_t *Host_Buffer){

}
static void Bootloader_Read_OTP(uint8_t *Host_Buffer){

}


static uint8_t Change_ROP_Level(uint32_t ROP_Level){
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	FLASH_OBProgramInitTypeDef FLASH_OBProgramInit;
	uint8_t ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;

	/* Unlock the FLASH Option Control Registers access */
	HAL_Status = HAL_FLASH_OB_Unlock();
	if(HAL_Status != HAL_OK){
		ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Failed -> Unlock the FLASH Option Control Registers access \r\n");
#endif
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("Passed -> Unlock the FLASH Option Control Registers access \r\n");
#endif
		FLASH_OBProgramInit.OptionType = OPTIONBYTE_RDP; /* RDP option byte configuration */
		FLASH_OBProgramInit.Banks = 3U; /* Both Banks */
		FLASH_OBProgramInit.RDPLevel = ROP_Level;
		/* Program option bytes */
		HAL_Status = HAL_FLASHEx_OBProgram(&FLASH_OBProgramInit);
		if(HAL_Status != HAL_OK){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Failed -> Program option bytes \r\n");
#endif
			HAL_Status = HAL_FLASH_OB_Lock();
			ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
		}
		else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
			BL_Print_Message("Passed -> Program option bytes \r\n");
#endif
			/* Launch the option byte loading */
			HAL_FLASH_OB_Launch();
			/* Lock the FLASH Option Control Registers access */
			HAL_Status = HAL_FLASH_OB_Lock();
			if(HAL_Status != HAL_OK){
				ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
			}
			else{
				ROP_Level_Status = ROP_LEVEL_CHANGE_VALID;
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
				BL_Print_Message("Passed -> Program ROP to Level : 0x%X \r\n", ROP_Level);
#endif
			}

		}
	}
	return ROP_Level_Status;
}

static void Bootloader_Change_Read_Protection_Level(uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0;
 	uint32_t Host_CRC32 = 0;
	uint8_t ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
	uint8_t Host_ROP_Level = 0;

#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Message("Change read protection level of the user flash \r\n");
#endif
	/* Extract the CRC32 and packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0] + 1;
	Host_CRC32 = *((uint32_t *)((Host_Buffer + Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
	/* CRC Verification */
	if(CRC_VERIFICATION_PASSED == Bootloader_CRC_Verify((uint8_t *)&Host_Buffer[0] , Host_CMD_Packet_Len - 4, Host_CRC32)){
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Passed \r\n");
#endif
		Bootloader_Send_ACK(1);
		/* Request change the Read Out Protection Level */
		Host_ROP_Level = Host_Buffer[2];
		/* Warning: When enabling read protection level 2, it s no more possible to go back to level 1 or 0 */
		if((CBL_ROP_LEVEL_2 == Host_ROP_Level)){
			ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
		}
		else{
			if(CBL_ROP_LEVEL_0 == Host_ROP_Level){
				Host_ROP_Level = 0xAA;
			}
			else if(CBL_ROP_LEVEL_1 == Host_ROP_Level){
				Host_ROP_Level = 0x55;
			}
			ROP_Level_Status = Change_ROP_Level(Host_ROP_Level);
		}
		Bootloader_Send_Data_To_Host((uint8_t *)&ROP_Level_Status, 1);
	}
	else{
#if (BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
		BL_Print_Message("CRC Verification Failed \r\n");
#endif
		Bootloader_Send_NACK();
	}
}
/*****************************************Static Functions Implementation End*****************************************/

