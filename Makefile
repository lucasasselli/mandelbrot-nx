ifeq ($(strip $(MODE)),)
$(error "Please set MODE in your environment.")
endif

TARGET      := nx-mandelbrot

SRCDIRS		:= src
INCDIRS     := include
ROMFS		:= res

SOURCES := $(wildcard $(SRCDIRS)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

CFLAGS  := $(foreach dir,$(INCDIRS),-I$(dir))

.PHONY: all clean
all:

ifeq ($(MODE),pc)

#---------------------------------------------------------------------------
# BUILD PC
#---------------------------------------------------------------------------

all: $(TARGET)

CFLAGS += -Iglad/include

LIBS   := -lGLU -lGL -lm -lglfw -ldl

$(TARGET): $(SOURCES) glad/src/glad.c
	$(CXX) -o $@ $(CFLAGS) $(CPPFLAGS) $? $(LIBS)

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

PORTLIBS	:= $(PORTLIBS_PATH)/switch
PATH	    := $(PORTLIBS)/bin:$(PATH)

APP_TITLE	:= Mandelbrot
APP_AUTHOR	:= Luca Sasselli
APP_VERSION	:= 1.0
APP_ICON	:=	$(DEVKITPRO)/libnx/default_icon.jpg

ARCH	:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS	+=	-g -Wall -O2 -ffunction-sections $(ARCH)
CFLAGS	+=	-D__SWITCH__
CFLAGS  += -I$(DEVKITPRO)/libnx/include
CFLAGS  += -I$(PORTLIBS)/include/

CXXFLAGS += -fno-rtti -fno-exceptions

LDFLAGS	=	-specs=$(DEVKITPRO)/libnx/switch.specs

LIBS	:= -L$(DEVKITPRO)/libnx/lib -L$(PORTLIBS)/lib -lglad -lglfw3 -lEGL -lglapi -ldrm_nouveau -lnx -lm

NROFLAGS += --nacp=$(TARGET).nacp --icon=$(APP_ICON) --romfsdir=$(ROMFS)
NACPFLAGS += --titleid=$(APP_TITLEID)

$(TARGET).elf : $(SOURCES)
	$(CXX) -o $@ $(CFLAGS) $(CPPFLAGS) $? $(LIBS) $(LDFLAGS)

endif

%.nro : %.elf %.nacp
	elf2nro $< $@ $(NROFLAGS)

%.nacp:
	nacptool --create "$(APP_TITLE)" "$(APP_AUTHOR)" "$(APP_VERSION)" $@

clean:
	@rm -rf $(TARGET).*