################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/backup.c \
../Core/Src/device.c \
../Core/Src/esp32_at.c \
../Core/Src/led.c \
../Core/Src/main.c \
../Core/Src/main_proc.c \
../Core/Src/operate.c \
../Core/Src/real_time.c \
../Core/Src/sd_card.c \
../Core/Src/server_api.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/web_page.c 

OBJS += \
./Core/Src/backup.o \
./Core/Src/device.o \
./Core/Src/esp32_at.o \
./Core/Src/led.o \
./Core/Src/main.o \
./Core/Src/main_proc.o \
./Core/Src/operate.o \
./Core/Src/real_time.o \
./Core/Src/sd_card.o \
./Core/Src/server_api.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/web_page.o 

C_DEPS += \
./Core/Src/backup.d \
./Core/Src/device.d \
./Core/Src/esp32_at.d \
./Core/Src/led.d \
./Core/Src/main.d \
./Core/Src/main_proc.d \
./Core/Src/operate.d \
./Core/Src/real_time.d \
./Core/Src/sd_card.d \
./Core/Src/server_api.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/web_page.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/backup.cyclo ./Core/Src/backup.d ./Core/Src/backup.o ./Core/Src/backup.su ./Core/Src/device.cyclo ./Core/Src/device.d ./Core/Src/device.o ./Core/Src/device.su ./Core/Src/esp32_at.cyclo ./Core/Src/esp32_at.d ./Core/Src/esp32_at.o ./Core/Src/esp32_at.su ./Core/Src/led.cyclo ./Core/Src/led.d ./Core/Src/led.o ./Core/Src/led.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/main_proc.cyclo ./Core/Src/main_proc.d ./Core/Src/main_proc.o ./Core/Src/main_proc.su ./Core/Src/operate.cyclo ./Core/Src/operate.d ./Core/Src/operate.o ./Core/Src/operate.su ./Core/Src/real_time.cyclo ./Core/Src/real_time.d ./Core/Src/real_time.o ./Core/Src/real_time.su ./Core/Src/sd_card.cyclo ./Core/Src/sd_card.d ./Core/Src/sd_card.o ./Core/Src/sd_card.su ./Core/Src/server_api.cyclo ./Core/Src/server_api.d ./Core/Src/server_api.o ./Core/Src/server_api.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/web_page.cyclo ./Core/Src/web_page.d ./Core/Src/web_page.o ./Core/Src/web_page.su

.PHONY: clean-Core-2f-Src

