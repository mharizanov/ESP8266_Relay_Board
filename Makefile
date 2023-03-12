# tnx to mamalala
# Changelog
# Changed the variables to include the header file directory
# Added global var for the XTENSA tool root
#
# This make file still needs some work.
#
#
# Output directors to store intermediate compiled files
# relative to the project directory
BUILD_BASE	= build
FW_BASE		= firmware


#SET THIS !
FLASH_SIZE ?= 1MB

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= $(HOME)/esp/xtensa-lx106-elf/bin
#XTENSA_TOOLS_ROOT ?= $(HOME)/.platformio/packages/toolchain-xtensa@1.40802.0/bin

AR = xtensa-lx106-elf-ar
CC = xtensa-lx106-elf-gcc
CXX = xtensa-lx106-elf-g++
CPP = xtensa-lx106-elf-cpp


# base directory of the ESP8266 SDK package, absolute
#SDK_BASE	?= $(HOME)/.platformio/packages/framework-esp8266-nonos-sdk
SDK_BASE        ?= $(HOME)/esp/ESP8266_NONOS_SDK
#Extra Tensilica includes from the ESS VM
#Not sure these are needed anymore
SDK_EXTRA_INCLUDES ?= $(SDK_BASE)/third_party/include
SDK_EXTRA_LIBS ?= /tmp/al.l

# Path to bootloader file
BOOTFILE	?= $(SDK_BASE/bin/boot_v1.7.bin)

APPGEN_TOOL	?= gen_appbin.py

OS 	:= $(shell uname)

ESP_FLASH_MAX       ?= 503808  # max bin file

ifeq ($(OS),Darwin)
	STATARGS = -f %z	
	FINDARGS = -E . -type f  -iregex '.*\.(html|css|js)'
else
	FINDARGS = . -type f -regex ".*/.*\.\(html\|css\|js\)"
	STATARGS = -c %s	
endif

ifeq ("$(FLASH_SIZE)","512KB")
# Winbond 25Q40 512KB flash, typ for esp-01 thru esp-11
ESP_SPI_SIZE        ?= 0       # 0->512KB (256KB+256KB)
ESP_FLASH_MODE      ?= 0       # 0->QIO
ESP_FLASH_FREQ_DIV  ?= 0       # 0->40Mhz
ET_FS               ?= 4m      # 4Mbit flash size in esptool flash command
ET_FF               ?= 40m     # 40Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x7E000 # where to flash blank.bin to erase wireless settings

else ifeq ("$(FLASH_SIZE)","1MB")
# ESP-01E
ESP_SPI_SIZE        ?= 2       # 2->1MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0       # 0->QIO
#ESP_FLASH_FREQ_DIV  ?= 15      # 15->80MHz
ESP_FLASH_FREQ_DIV  ?= 0      # 15->80MHz
ET_FS               ?= 8m      # 8Mbit flash size in esptool flash command
ET_FF               ?= 40m     # 40Mhz flash speed in esptool flash command
#ET_FF               ?= 80     # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0xFE000 # where to flash blank.bin to erase wireless settings

else ifeq ("$(FLASH_SIZE)","2MB")
# Manuf 0xA1 Chip 0x4015 found on wroom-02 modules
# Here we're using two partitions of approx 0.5MB because that's what's easily available in terms
# of linker scripts in the SDK. Ideally we'd use two partitions of approx 1MB, the remaining 2MB
# cannot be used for code (esp8266 limitation).
ESP_SPI_SIZE        ?= 4       # 6->4MB (1MB+1MB) or 4->4MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0       # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15      # 15->80Mhz
ET_FS               ?= 16m     # 16Mbit flash size in esptool flash command
ET_FF               ?= 80m     # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x1FE000 # where to flash blank.bin to erase wireless settings

else
# Winbond 25Q32 4MB flash, typ for esp-12
# Here we're using two partitions of approx 0.5MB because that's what's easily available in terms
# of linker scripts in the SDK. Ideally we'd use two partitions of approx 1MB, the remaining 2MB
# cannot be used for code (esp8266 limitation).
ESP_SPI_SIZE        ?= 4       # 6->4MB (1MB+1MB) or 4->4MB (512KB+512KB)
ESP_FLASH_MODE      ?= 0       # 0->QIO, 2->DIO
ESP_FLASH_FREQ_DIV  ?= 15      # 15->80Mhz
ET_FS               ?= 32m     # 32Mbit flash size in esptool flash command
ET_FF               ?= 80m     # 80Mhz flash speed in esptool flash command
ET_BLANK            ?= 0x3FE000 # where to flash blank.bin to erase wireless settings
endif


DEFINES += -DSPI_FLASH_SIZE_MAP=$(ESP_SPI_SIZE) -DCGIFLASH_DBG -DSYSLOGDBG


# name for the target project
TARGET		= relayboard

# which modules (subdirectories) of the project to include in compiling
#MODULES		= driver user lwip/api lwip/app lwip/core lwip/core/ipv4 lwip/netif
		  #xtensa-lx106-elf/include 
		  #xtensa-lx106-elf/include/sys 
MODULES		= src/user 
EXTRA_INCDIR	= include \
		sys \
		include/sys \
		. \
		lib/heatshrink/ \
		$(SDK_EXTRA_INCLUDES) 


# libraries used in this project, mainly provided by the SDK
#LIBS		= c gcc phy pp net80211 wpa main lwip crypto airkiss at driver espnow json mbedtls mesh pwm smartconfig ssl upgrade wpa2 wps 
LIBS		= c gcc phy pp net80211 wpa main lwip crypto airkiss at driver espnow json pwm smartconfig ssl upgrade wpa2 wps 

#-lairkiss -lat -lc -lcrypto -ldriver -lespnow -lgcc -ljson -llwip -lmain -lmbedtls -lmesh -lnet80211 -lphy -lpp -lpwm -lsmartconfig -lssl -lupgrade -lwpa -lwpa2 -lwps

# if GZIP_COMPRESSION is enabled then the static css, js, and html files will be compressed with gzip before added to the espfs image.
# This could speed up the downloading of these files, but might break compatibility with older web browsers not supporting gzip encoding
# because Accept-Encoding is simply ignored. Enable this option if you have large static files to serve (for e.g. JQuery, Twitter bootstrap)
GZIP_COMPRESSION = "yes"

# compiler flags using during compilation of source files
		  #-Wno-implicit-function-declaration -nostdlib  -ffunction-sections -fdata-sections -fno-builtin-printf \  
CFLAGS		= -Os -ggdb -std=c99  -Wpointer-arith -Wundef -Wall -Wl,-EL -fno-inline-functions \
		  -Wno-implicit-function-declaration -ffunction-sections -fdata-sections -fno-builtin-printf \
		-mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH \
		-Wno-address -Wno-unused-function -Wno-unused-but-set-variable \
		-DPLATFORMIO=60106 -DESP8266 -DARDUINO_ARCH_ESP8266 -DARDUINO_ESP8266_ESP01 \
		-DLWIP_OPEN_SRC -DTENSILICA -DICACHE_FLASH -DPIO_FRAMEWORK_ARDUINO_ESPRESSIF_SDK22x_190703 \
		-DF_CPU=80000000L -D__ets__ $(DEFINES) \
		-U__STRICT_ANSI__ 
		


# linker flags used to generate the main object file
#LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static -L$(SDK_EXTRA_LIBS)
LDFLAGS		=  -Wl,--no-check-sections -u call_user_start -Wl,-static -L$(SDK_EXTRA_LIBS)

# linker script used for the above linkier step
#LD_SCRIPT	= eagle.app.v6.ld
LD_SCRIPT1	= eagle.app.v6.new.1024.app1.ld
LD_SCRIPT2	= eagle.app.v6.new.1024.app2.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json
SDK_TOOLSDIR	= tools

# we create two different files for uploading into the flash
# these are the names and options to generate them
FW_FILE_3       = webpages.espfs

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
NM 		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-nm
OBJCP	 	:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy
OBJDP 		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objdump
ELF_SIZE	:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-size
COMPILE		:= gcc


####
#### no user configurable options below here
####
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))
SDK_TOOLS	:= $(addprefix $(SDK_BASE)/,$(SDK_TOOLSDIR))
APPGEN_TOOL	:= $(addprefix $(SDK_TOOLS)/,$(APPGEN_TOOL))


SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT1	:= $(addprefix $(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT1))
LD_SCRIPT2	:= $(addprefix $(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT2))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

FW_FILE_1	:= $(addprefix $(FW_BASE)/,$(FW_FILE_1).bin)
FW_FILE_2	:= $(addprefix $(FW_BASE)/,$(FW_FILE_2).bin)
USER1_OUT 	:= $(addprefix $(BUILD_BASE)/,$(TARGET).user1.out)
USER2_OUT 	:= $(addprefix $(BUILD_BASE)/,$(TARGET).user2.out)

PYTHON=python2

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

ifeq ($(GZIP_COMPRESSION),"yes")
CFLAGS		+= -DGZIP_COMPRESSION
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(FW_BASE)/user1.bin $(FW_BASE)/user2.bin

$(USER1_OUT): $(APP_AR) $(LD_SCRIPT1)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) -T$(LD_SCRIPT1) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
	@echo Dump  : $(OBJDP) -x $(USER1_OUT)
	@echo Disass: $(OBJDP) -d -l -x $(USER1_OUT)
#	$(Q) $(OBJDP) -x $(TARGET_OUT) | egrep espfs_img

$(USER2_OUT): $(APP_AR) $(LD_SCRIPT2)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) -T$(LD_SCRIPT2) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
#	$(Q) $(OBJDP) -x $(TARGET_OUT) | egrep espfs_img

$(FW_BASE):
	$(vecho) "FW $@"
	$(Q) mkdir -p $@

$(FW_BASE)/user1.bin: $(USER1_OUT) $(FW_BASE)
	$(Q) $(OBJCP) --only-section .text -O binary $(USER1_OUT) eagle.app.v6.text.bin
	$(Q) $(OBJCP) --only-section .data -O binary $(USER1_OUT) eagle.app.v6.data.bin
	$(Q) $(OBJCP) --only-section .rodata -O binary $(USER1_OUT) eagle.app.v6.rodata.bin
	$(Q) $(OBJCP) --only-section .irom0.text -O binary $(USER1_OUT) eagle.app.v6.irom0text.bin
	$(Q) $(ELF_SIZE) -A $(USER1_OUT) |grep -v " 0$$" |grep .
	$(Q) COMPILE=gcc PATH=$(XTENSA_TOOLS_ROOT):$(PATH) python $(APPGEN_TOOL) $(USER1_OUT) 2 $(ESP_FLASH_MODE) $(ESP_FLASH_FREQ_DIV) $(ESP_SPI_SIZE) 1 >/dev/null
	$(Q) rm -f eagle.app.v6.*.bin
	$(Q) mv eagle.app.flash.bin $@
	@echo "    user1.bin uses $$(stat $(STATARGS) $@) bytes of" $(ESP_FLASH_MAX) "available"
#	$(Q) if [ $$(stat -f '%z' $@) -gt $$(( $(ESP_FLASH_MAX) )) ]; then echo "$@ too big!"; false; fi
	$(Q) if [ $$(stat $(STATARGS) $@ ) -gt $$(( $(ESP_FLASH_MAX) )) ]; then echo "$@ too big!"; false; fi

$(FW_BASE)/user2.bin: $(USER2_OUT) $(FW_BASE)
	$(Q) $(OBJCP) --only-section .text -O binary $(USER2_OUT) eagle.app.v6.text.bin
	$(Q) $(OBJCP) --only-section .data -O binary $(USER2_OUT) eagle.app.v6.data.bin
	$(Q) $(OBJCP) --only-section .rodata -O binary $(USER2_OUT) eagle.app.v6.rodata.bin
	$(Q) $(OBJCP) --only-section .irom0.text -O binary $(USER2_OUT) eagle.app.v6.irom0text.bin
	$(Q) COMPILE=gcc PATH=$(XTENSA_TOOLS_ROOT):$(PATH) python $(APPGEN_TOOL) $(USER2_OUT) 2 $(ESP_FLASH_MODE) $(ESP_FLASH_FREQ_DIV) $(ESP_SPI_SIZE) 2 >/dev/null
	$(Q) rm -f eagle.app.v6.*.bin
	$(Q) mv eagle.app.flash.bin $@
#	$(Q) if [ $$(stat -f '%z' $@) -gt $$(( $(ESP_FLASH_MAX) )) ]; then echo "$@ too big!"; false; fi
	$(Q) if [ $$(stat $(STATARGS) $@ ) -gt $$(( $(ESP_FLASH_MAX) )) ]; then echo "$@ too big!"; false; fi

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^


$(FW_FILE_3): html/ html/config/ html/config/wifi/ html/control/ mkespfsimage/mkespfsimage
	$(vecho) "MKEFSIMAGE $@"
ifeq ($(GZIP_COMPRESSION),"yes")
	$(Q) rm -rf html_compressed;
		$(Q) cp -r html html_compressed;
#		$(Q) cd html_compressed; find . -type f -regex ".*/.*\.\(html\|css\|js\)" -exec sh -c "gzip -n {}; mv {}.gz {}" \;; cd ..;	
#		$(Q) cd html_compressed; find -E . -type f  -iregex '.*\.(html|css|js)'  -exec sh -c "gzip -n {}; mv {}.gz {}" \;; cd ..;	
		$(Q) cd html_compressed; find $(FINDARGS)  -exec sh -c "gzip -n {}; mv {}.gz {}" \;; cd ..;	
		$(Q) cd html_compressed; find .  | ../mkespfsimage/mkespfsimage > ../firmware/$(FW_FILE_3); cd ..;
else
		$(Q) cd html; find . | ../mkespfsimage/mkespfsimage > ../firmware/$(FW_FILE_3); cd ..
endif
#		$(Q) if [ $$(stat -f '%z' firmware/$(FW_FILE_3)) -gt $$(( 0x2E000 )) ]; then echo firmware/$(FW_FILE_3)" too big!"; false; fi
		$(Q) if [ $$(stat $(STATARGS) firmware/$(FW_FILE_3)) -gt $$(( 0x2E000 )) ]; then echo firmware/$(FW_FILE_3)" too big!"; false; fi


checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

mkespfsimage/mkespfsimage: mkespfsimage/
	make -C mkespfsimage

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) find $(BUILD_BASE) -type f | xargs rm -f

ifeq ($(GZIP_COMPRESSION),"yes") 
	$(Q) rm -rf html_compressed;
endif

	$(Q) rm -f $(FW_FILE_1)
	$(Q) rm -f $(FW_FILE_2)
	$(Q) rm -f $(FW_FILE_3)

	$(Q) rm -rf $(FW_BASE)

distclean: clean
	$(Q) make -C mkespfsimage clean

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
