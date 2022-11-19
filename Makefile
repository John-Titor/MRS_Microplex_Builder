#
# Build an app for an MRS Microplex 7 module using the CodeWarrior 
# MCU binaries.
#

#
# Set this to where Codewarrior MCU was installed, or pass
# it on the commandline.
#
#CW_INSTALL_DIR	?= ~/.wine/drive_c/Freescale/CW\ MCU\ v11.1/MCU
CW_INSTALL_DIR	?= ../MCU

_CWD		:= $(dir $(lastword $(MAKEFILE_LIST)))
ifneq ($(words $(_CWD)),1)
$(error spaces not permitted in parent directory names)
endif
ifeq ($(VERBOSE),)
V		 = @
endif

BUILDDIR	?= $(_CWD)build
export SRCDIR	 = $(_CWD)
export MCU	 = $(BUILDDIR)/MCU
export RSRCDIR	 = $(_CWD)resources

WINE		:= /opt/homebrew/bin/wine
CC		 = $(WINE) $(MCU)/prog/chc08.exe
LD		 = $(WINE) $(MCU)/prog/linker.exe

GLOBAL_DEPS	:= $(MAKEFILE_LIST) \
		   $(wildcard $(RSRCDIR)/*) \
		   $(MCU)

# look for application sources, fall back to the test app
-include $(_CWD)/app/app.mk
APP_SRCS	?= $(wildcard test/*.c)

# library sources
LIB_SRCS	:= $(wildcard lib/*.c) \
		   $(wildcard lib/HAL/*.c)

# needed sources from CW MCU
MCU_SRCS	 = $(MCU)/lib/hc08c/src/start08.c \
		   $(MCU)/lib/hc08c/device/src/mc9s08dz60.c

FORMAT_SRCS	 = $(shell find $(_CWD)lib $(_CWD)src $(_CWD)include -name "*.[ch]")
APP_FORMAT_SRCS	 = $(shell find $(_CWD)app -name "*.[ch]")

# export this so the compiler can pick it up
export GIT_VERS	:= $(shell git describe --always --dirty)

# export these so the linker can pick it up from its config
export OBJS	:= $(patsubst %.c,$(BUILDDIR)/%.o,$(APP_SRCS) $(LIB_SRCS)) \
		   $(patsubst $(MCU)/lib/%.c,$(BUILDDIR)/mcu_lib/%.o,$(MCU_SRCS))
export LIBS	 = $(MCU)/lib/hc08c/lib/ansiis.lib

# make the linker put the map file somewhere sensible
export TEXTPATH	 = $(BUILDDIR)

# suppress generation of error files
export ERRORFILE = /dev/null

# convert \ to / to enable parsing filenames out of error messages 
CONVERT_SLASHES	 = | tr \\ /

# make Wine wineserver quieter - turn this off when debugging Wine problems
export WINEDEBUG = -all

.PHONY: all clean reformat doc
.SECONDARY:

all:	$(BUILDDIR)/microplex.elf

clean:
	rm -rf $(BUILDDIR)

doc:
	doxygen

REFORMAT_OPTS	 = --style=kr \
		   --indent=spaces=4 \
		   --indent-cases \
		   --indent-preprocessor \
		   --break-blocks \
		   --pad-oper \
		   --pad-header \
		   --unpad-paren \
		   --add-brackets \
		   --convert-tabs \
		   --align-pointer=name \
		   --keep-one-line-blocks \
		   --formatted \
		   --suffix=none \

reformat:
	astyle $(REFORMAT_OPTS)	$(APP_FORMAT_SRCS)

reformat-all:
	astyle $(REFORMAT_OPTS)	$(APP_FORMAT_SRCS) $(FORMAT_SRCS)

# build an application / generate s-records
$(BUILDDIR)/%.elf: $(OBJS) $(GLOBAL_DEPS)
	@mkdir -p $(@D)
	@echo ==== LINK $(notdir $@)
	$(V)$(LD) -ArgFile$(_CWD)/resources/link.args -O$@

# build an object file from a source file in ./src
$(BUILDDIR)/%.o: %.c $(GLOBAL_DEPS)
	@mkdir -p $(@D)
	@echo ==== COMPILE $<
	$(V)$(CC) -ArgFile$(_CWD)/resources/compile.args 	\
	 	-ObjN=$@ $(abspath $<) 				\
		-Lm=$(@:%.o=%.d) 				\
		$(CONVERT_SLASHES)

# build an object file from a source file supplied by CW MCU
$(BUILDDIR)/mcu_lib/%.o: $(MCU)/lib/%.c $(GLOBAL_DEPS)
	@mkdir -p $(@D)
	@echo ==== COMPILE $<
	$(V)$(CC) -ArgFile$(_CWD)/resources/compile.args 	\
		-ObjN=$@ $< 					\
		-Lm=$(@:%.o=%.d)				\
		$(CONVERT_SLASHES)

$(MCU_SRCS): $(MCU)

# link to the CW MCU tools
$(MCU):
	@mkdir -p $(@D)
	@test -d $(CW_INSTALL_DIR) || (echo Must set CW_INSTALL_DIR; exit 1)
	$(V)ln -sf $(abspath $(CW_INSTALL_DIR)) $@

-include $(OBJS:.o=.d)
