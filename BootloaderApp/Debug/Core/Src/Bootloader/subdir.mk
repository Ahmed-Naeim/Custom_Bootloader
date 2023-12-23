################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Bootloader/bootloader.c 

OBJS += \
./Core/Src/Bootloader/bootloader.o 

C_DEPS += \
./Core/Src/Bootloader/bootloader.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Bootloader/%.o Core/Src/Bootloader/%.su Core/Src/Bootloader/%.cyclo: ../Core/Src/Bootloader/%.c Core/Src/Bootloader/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Bootloader

clean-Core-2f-Src-2f-Bootloader:
	-$(RM) ./Core/Src/Bootloader/bootloader.cyclo ./Core/Src/Bootloader/bootloader.d ./Core/Src/Bootloader/bootloader.o ./Core/Src/Bootloader/bootloader.su

.PHONY: clean-Core-2f-Src-2f-Bootloader

