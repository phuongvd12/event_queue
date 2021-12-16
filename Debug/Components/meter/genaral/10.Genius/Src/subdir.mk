################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Components/meter/genaral/10.Genius/Src/Genius_Init_Meter.c \
../Components/meter/genaral/10.Genius/Src/Genius_Read_Meter.c \
../Components/meter/genaral/10.Genius/Src/command_line_protocol.c \
../Components/meter/genaral/10.Genius/Src/crc_utility.c \
../Components/meter/genaral/10.Genius/Src/ieee754_utility.c 

OBJS += \
./Components/meter/genaral/10.Genius/Src/Genius_Init_Meter.o \
./Components/meter/genaral/10.Genius/Src/Genius_Read_Meter.o \
./Components/meter/genaral/10.Genius/Src/command_line_protocol.o \
./Components/meter/genaral/10.Genius/Src/crc_utility.o \
./Components/meter/genaral/10.Genius/Src/ieee754_utility.o 

C_DEPS += \
./Components/meter/genaral/10.Genius/Src/Genius_Init_Meter.d \
./Components/meter/genaral/10.Genius/Src/Genius_Read_Meter.d \
./Components/meter/genaral/10.Genius/Src/command_line_protocol.d \
./Components/meter/genaral/10.Genius/Src/crc_utility.d \
./Components/meter/genaral/10.Genius/Src/ieee754_utility.d 


# Each subdirectory must supply rules for building sources it contributes
Components/meter/genaral/10.Genius/Src/%.o: ../Components/meter/genaral/10.Genius/Src/%.c Components/meter/genaral/10.Genius/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L452xx -c -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/sub_event" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/10.Genius/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/11.Start/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/12.Meter103/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/3.Gelex/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/4.Landis/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/CPC_Lib/inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/1.Elster/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/flash_ext" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/App" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/queue" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/sim_module" -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

