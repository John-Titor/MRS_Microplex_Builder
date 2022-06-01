#
# Build an app for an MRS Microplex 7 module using the CodeWarrior MCU binaries.
#

#
# Set this to where Codewarrior MCU was installed, or pass
# it on the commandline.
#
CW_INSTALL_DIR	?= ~/.wine/drive_c/Freescale/CW\ MCU\ v11.1/MCU

_CWD		:= $(dir $(lastword $(MAKEFILE_LIST)))
ifneq ($(words $(_CWD)),1)
$(error spaces not permitted in parent directory names)
endif
ifeq ($(VERBOSE),)
V		 = @
endif

OBJDIR		?= $(_CWD)obj
export SRCDIR	 = $(_CWD)
export MCU	 = $(OBJDIR)/MCU
export RSRCDIR	 = $(_CWD)resources

CC		 = $(MCU)/prog/chc08.exe
LD		 = $(MCU)/prog/linker.exe

GLOBAL_DEPS	:= $(MAKEFILE_LIST) \
		   $(wildcard $(RSRCDIR)/*) \
		   $(MCU)

# application sources
APP_SRCS	:= $(wildcard src/*.c)

# library sources
LIB_SRCS	:= $(wildcard lib/*.c)

# needed sources from CW MCU
MCU_SRCS	 = $(MCU)/lib/hc08c/src/start08.c \
		   $(MCU)/lib/hc08c/device/src/mc9s08dz60.c

# export these so the linker can pick it up from its config
export OBJS	:= $(patsubst %.c,$(OBJDIR)/%.obj,$(APP_SRCS) $(LIB_SRCS)) \
		   $(patsubst $(MCU)/lib/%.c,$(OBJDIR)/mcu_lib/%.obj,$(MCU_SRCS))
export LIBS	 = $(MCU)/lib/hc08c/lib/ansiis.lib

# make the linker put its artifacts somewhere sensible
export TEXTPATH	 = $(OBJDIR)
export ERRORFILE = $(OBJDIR)/%n_link_errors.txt

.PHONY: all clean

all:	$(OBJDIR)/microplex.elf

clean:
	rm -rf $(OBJDIR)

# build an application / generate s-records
$(OBJDIR)/%.elf: $(OBJS) $(GLOBAL_DEPS)
	@mkdir -p $(@D)
	@echo ==== LINK $(notdir $@)
	$(V)wine $(LD) -ArgFile$(_CWD)/resources/link.args -O$@

# build an object file from a source file in ./src
$(OBJDIR)/%.obj: %.c $(GLOBAL_DEPS)
	@mkdir -p $(@D)
	@echo ==== COMPILE $(notdir $@)
	$(V)wine $(CC) -ArgFile$(_CWD)/resources/compile.args -ObjN=$@ $< -Lm=$(@:%.obj=%.d)

# build an object file from a source file supplied by CW MCU
$(OBJDIR)/mcu_lib/%.obj: $(MCU)/lib/%.c $(GLOBAL_DEPS)
	@mkdir -p $(@D)
	@echo ==== COMPILE $(notdir $@)
	$(V)wine $(CC) -ArgFile$(_CWD)/resources/compile.args -ObjN=$@ $< -Lm=$(@:%.obj=%.d)

$(MCU_SRCS): $(MCU)

# link to the CW MCU tools
$(MCU):
	@mkdir -p $(@D)
	@test -d $(CW_INSTALL_DIR) || (echo Must set CW_INSTALL_DIR; exit 1)
	$(V)ln -sf $(CW_INSTALL_DIR) $@

-include $(OBJS:.obj=.d)
