APP_TITLE	:= Mandelbrot
APP_AUTHOR	:= Luca Sasselli
APP_VERSION	:= 1.0
APP_ICON	:= icon.jpg

TARGET  := nx-mandelbrot

SRCDIRS	:= src
INCDIRS := include
ROMFS	:= res

SOURCES := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.cpp))
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

INCLUDES := $(foreach dir,$(INCDIRS),-I$(dir))

MODE ?= pc

.PHONY: all clean
all:

ifeq ($(MODE),pc)

#---------------------------------------------------------------------------
# BUILD PC
#---------------------------------------------------------------------------

all: $(TARGET)

INCLUDES += -Iglad/include

LIBS := -lGLU -lGL -lm -lglfw -ldl

$(TARGET): $(SOURCES) glad/src/glad.c
	$(CXX) -o $@ $(CXXFLAGS) $(INCLUDES) $^ $(LIBS)

else

#---------------------------------------------------------------------------
# BUILD SWITCH
#---------------------------------------------------------------------------

all: $(TARGET).nro

# Check devkitpro
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

include $(DEVKITPRO)/devkitA64/base_tools

PORTLIBS := $(PORTLIBS_PATH)/switch
PATH	 := $(PORTLIBS)/bin:$(PATH)

ARCH :=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CXXFLAGS  += -g -Wall -O2 -ffunction-sections $(ARCH)
CXXFLAGS  += -D__SWITCH__
CXXFLAGS  += -fno-rtti -fno-exceptions

INCLUDES  += -I$(DEVKITPRO)/libnx/include
INCLUDES  += -I$(PORTLIBS)/include/

LIBS := -L$(DEVKITPRO)/libnx/lib -L$(PORTLIBS)/lib -lglad -lglfw3 -lEGL -lglapi -ldrm_nouveau -lnx -lm

LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs

$(TARGET).elf : $(SOURCES)
	$(CXX) -o $@ $(CXXFLAGS) $(INCLUDES) $^ $(LIBS) $(LDFLAGS)

endif

NROFLAGS   := --nacp=$(TARGET).nacp --icon=$(APP_ICON) --romfsdir=$(ROMFS)
NACPFLAGS  := --titleid=$(APP_TITLEID)

%.nro : %.elf %.nacp
	elf2nro $< $@ $(NROFLAGS)

%.nacp:
	nacptool --create "$(APP_TITLE)" "$(APP_AUTHOR)" "$(APP_VERSION)" $@

clean:
	@rm -rf $(TARGET) $(TARGET).*