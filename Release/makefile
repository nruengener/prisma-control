################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include util/subdir.mk
-include mpu6050/subdir.mk
-include kalman/subdir.mk
-include i2chw/subdir.mk
-include blmotor/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
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

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
PrismaControl.lss \

FLASH_IMAGE += \
PrismaControl.hex \

SIZEDUMMY += \
sizedummy \


# All Target
all: PrismaControl.elf secondary-outputs

# Tool invocations
PrismaControl.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,PrismaControl.map -Wl,-u,vfprintf -mmcu=atmega328p -o "PrismaControl.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

PrismaControl.lss: PrismaControl.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S PrismaControl.elf  >"PrismaControl.lss"
	@echo 'Finished building: $@'
	@echo ' '

PrismaControl.hex: PrismaControl.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex PrismaControl.elf  "PrismaControl.hex"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: PrismaControl.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p PrismaControl.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) PrismaControl.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
