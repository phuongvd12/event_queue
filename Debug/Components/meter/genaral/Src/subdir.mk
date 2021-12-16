################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Components/meter/genaral/Src/Init_All_Meter.c \
../Components/meter/genaral/Src/myuart.c \
../Components/meter/genaral/Src/pushdata.c \
../Components/meter/genaral/Src/variable.c 

OBJS += \
./Components/meter/genaral/Src/Init_All_Meter.o \
./Components/meter/genaral/Src/myuart.o \
./Components/meter/genaral/Src/pushdata.o \
./Components/meter/genaral/Src/variable.o 

C_DEPS += \
./Components/meter/genaral/Src/Init_All_Meter.d \
./Components/meter/genaral/Src/myuart.d \
./Components/meter/genaral/Src/pushdata.d \
./Components/meter/genaral/Src/variable.d 


# Each subdirectory must supply rules for building sources it contributes
Components/meter/genaral/Src/%.o: ../Components/meter/genaral/Src/%.c Components/meter/genaral/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L452xx -c -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/sub_event" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/10.Genius/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/11.Start/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/12.Meter103/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/3.Gelex/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/4.Landis/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/CPC_Lib/inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/1.Elster/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/flash_ext" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter/genaral/Inc" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/App" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/meter" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/queue" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/Components/sim_module" -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

