# directories
OBJ_DIR = obj
OUTPUT_DIR = build
OUTPUT = build

ifneq ($(wildcard paths.mk),)
  include paths.mk
else
  $(info Copying file default.paths.mk into paths.mk.)
  $(info Please customize its variables appropriately for your computer.)
  $(shell cp default.paths.mk paths.mk)
  include paths.mk
endif

# Check to make sure that paths.mk defined all of the variables that we needed.
ifeq ($(wildcard $(ASF_PATH)/.),)
  $(info Please edit paths.mk to point the variable ASF_PATH to the root directory of the Microchip ASF3 XDK.)
  $(info The Microchip ASF3 XDK can be downloaded from)
  $(info www.microchip.com/en-us/development-tools-tools-and-software/libraries-code-examples-and-more/advanced-software-framework-for-sam-devices)
  $(info or)
  $(info www.microchip.com/mplab/avr-support/avr-and-sam-downloads-archive)
  $(error ASF_PATH not defined)
endif

################################
#  chip name                   #
################################
CHIP = SAMD21G18A

################################
#  Include directories         #
################################
# This sample makefile just has includes for structs for accessing peripheral MMIO. If you want to
# add include
INCLUDES=

# Include path for samd21.h and other component files
INCLUDES+= -I$(ASF_PATH)/sam0/utils/cmsis/samd21/include/

# Include path for system_samd21.h
INCLUDES+= -I$(ASF_PATH)/sam0/utils/cmsis/samd21/source/

# Include path for core_cm0plus.h, which defines mem-mapped structs for cortex-m0+ peripherals
# (e.g. NVIC and systick)
INCLUDES+= -I$(ASF_PATH)/thirdparty/CMSIS/Include/

# Uncomment this to include header files for ASF drivers.
# The header files for ASF drivers have the same filenames as the header files that define the MMIO
# structs. They're disambiguated because of extra path information.
# The MMIO struct for the TCC will be in sam0/utils/cmsis/samd21/include and is included with
#     #include "tcc.h"
# The ASF driver for the TCC will be in sam0/drivers/tcc/tcc.h and can be included with
#     #include "drivers/tcc/tcc.h"
# INCLUDES+= $(ASF_PATH)/sam0

# Project includes go here.
INCLUDES+= -I.
INCLUDES+= -I../common

################################
#  .a Libraries                #
################################
LIB_PATH=
LIB_PATH+= $(ASF_PATH)/thirdparty/CMSIS/Lib/GCC/
LIBS=
LIBS+= arm_cortexM0l_math


#################################
#  Sources                      #
#################################
C_SOURCES=
C_SOURCES+= $(ASF_PATH)/sam0/utils/cmsis/samd21/source/gcc/startup_samd21.c
C_SOURCES+= main.c
C_SOURCES+= ../common/game.c
C_SOURCES+= ../common/debounce.c
C_SOURCES+= ../common/audio_samples.c

ASM_SOURCES = $(shell find . -name "*.S" ! ! -iname ".*")

C_OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(C_SOURCES:.c=.c.o)))
ASM_OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(ASM_SOURCES:.S=.S.o)))

################################
#  Linker                      #
################################
# Note: a default linker script is included in the xdk, but you might want to make a copy so it can
# be customized.
LINKER_SCRIPT = ./samd21g18a_flash.ld

DEPENDENCIES_FILE = dependencies.d
#Tool prefix for cross-compilation
CROSS_COMPILE = arm-none-eabi-

#Compilation tools
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
SIZE = $(CROSS_COMPILE)size
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
GDB = $(CROSS_COMPILE)gdb
NM = $(CROSS_COMPILE)nm

####################
#    gcc flags     #
####################
# Warnings / errors
#CFLAGS += -Wall -Werror -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -Wno-missing-braces
CFLAGS += -Wno-override-init -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -Wno-missing-braces

# ARM stuff
CFLAGS += -march=armv6-m -mthumb -mno-thumb-interwork -mtune=cortex-m0plus

# Linker script stuff
CFLAGS += -ffunction-sections

OPTIMIZATION = -O3
CFLAGS += --std=gnu99 $(OPTIMIZATION) -g

# Define the microcontroller name
CFLAGS += -D __$(CHIP)__

#includes
CFLAGS += $(INCLUDES)

#####################
#  assembler flags  #
#####################
ASFLAGS += -mfloat-abi=soft -march=armv6-m -mcpu=cortex-m0plus -Wall -g $(OPTIMIZATION) -mthumb
ASFLAGS += -mlong-calls
ASFLAGS += $(INCLUDES) -D__ASSEMBLY__ -D $(CHIP) -D $(FAMILY)

#####################
#   linker flags    #
#####################
LDFLAGS += -Wl,-nostdlib -Wl,-lgcc
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--unresolved-symbols=report-all -Wl,--warn-common
LDFLAGS += -Wl,--warn-section-align
LDFLAGS += -mcpu=cortex-m0plus -mthumb

LDFLAGS += -Wl,-V

all: directories dependencies
	@$(MAKE) $(OUTPUT_DIR)/$(OUTPUT).elf

$(OUTPUT_DIR)/$(OUTPUT).elf: $(C_OBJECTS) $(ASM_OBJECTS)
	@echo "[$@]"
	@$(CC) $(LDFLAGS) $(LD_OPTIONAL) --specs=nano.specs -T$(LINKER_SCRIPT) -Wl,-Map,$(OUTPUT_DIR)/$(OUTPUT).map -o $(OUTPUT_DIR)/$(OUTPUT).elf $^
	@$(NM) $(OUTPUT_DIR)/$(OUTPUT).elf > $(OUTPUT_DIR)/$(OUTPUT).elf.txt
	@$(SIZE) $^ $(OUTPUT_DIR)/$(OUTPUT).elf

directories:
	@mkdir -p $(OUTPUT_DIR);
	@mkdir -p $(OBJ_DIR);

dependencies:
	@$(CC) -MM $(C_SOURCES) $(CFLAGS) | sed 's|\([a-zA-Z0-9_-]*\)\.o|$(OBJ_DIR)/\1.c.o|' > $(DEPENDENCIES_FILE)

$(OBJ_DIR)/%.c.o:
	@echo "[$<]"
	@$(CC) $(CFLAGS) -S -o $(OBJ_DIR)/$(notdir $@).s $<
	@$(CC) $(CFLAGS) -c -o $(OBJ_DIR)/$(notdir $@)  $<

$(OBJ_DIR)/%.S.o:
	@echo "[$<]"
	@$(CC) $(ASFLAGS) -c -o $(OBJ_DIR)/$(notdir $@) $<

-include $(DEPENDENCIES_FILE)

clean:
	@echo removing all build files
	@find . -name "*.o" -exec rm -f {} \;
	@find . -name "*.o.*" -exec rm -f {} \;
	@find . -name "*.elf" -exec rm -f {} \;
	@find . -name "*.out" -exec rm -f {} \;
	@find . -name "*.map" -exec rm -f {} \;
	@rm -rf $(OUTPUT_DIR)
	@rm -rf $(OBJ_DIR)
	@echo done

.PHONY: all directories dependencies clean
