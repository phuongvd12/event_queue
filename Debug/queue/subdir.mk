################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../queue/queue_p.c 

OBJS += \
./queue/queue_p.o 

C_DEPS += \
./queue/queue_p.d 


# Each subdirectory must supply rules for building sources it contributes
queue/%.o: ../queue/%.c queue/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L452xx -c -I../Core/Inc -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/queue" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/meter" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/sim_module" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/App" -I"D:/Embedded/DCU/2021/PhuongNov21/event_queue/EventDriven" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

