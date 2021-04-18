################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/Third_Party/LwIP/system/OS/subdir.mk
-include Middlewares/Third_Party/LwIP/src/netif/ppp/subdir.mk
-include Middlewares/Third_Party/LwIP/src/netif/subdir.mk
-include Middlewares/Third_Party/LwIP/src/core/ipv6/subdir.mk
-include Middlewares/Third_Party/LwIP/src/core/ipv4/subdir.mk
-include Middlewares/Third_Party/LwIP/src/core/subdir.mk
-include Middlewares/Third_Party/LwIP/src/apps/mqtt/subdir.mk
-include Middlewares/Third_Party/LwIP/src/apps/httpd/subdir.mk
-include Middlewares/Third_Party/LwIP/src/api/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include LWIP/Target/subdir.mk
-include LWIP/App/subdir.mk
-include Drivers/STM32F2xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := NUCLEO_F207
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX := 
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME).$(BUILD_ARTIFACT_EXTENSION)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
NUCLEO_F207.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
NUCLEO_F207.list \

OBJCOPY_BIN += \
NUCLEO_F207.bin \


# All Target
all: main-build

# Main-build Target
main-build: NUCLEO_F207.elf secondary-outputs

# Tool invocations
NUCLEO_F207.elf: $(OBJS) $(USER_OBJS) C:\Users\Olek.000\STM32CubeIDE\workspace_1.3.0\NUCLEO_F207\STM32F207ZGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "NUCLEO_F207.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"C:\Users\Olek.000\STM32CubeIDE\workspace_1.3.0\NUCLEO_F207\STM32F207ZGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="NUCLEO_F207.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

NUCLEO_F207.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "NUCLEO_F207.list"
	@echo 'Finished building: $@'
	@echo ' '

NUCLEO_F207.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "NUCLEO_F207.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents fail-specified-linker-script-missing warn-no-linker-script-specified
.SECONDARY:

-include ../makefile.targets
