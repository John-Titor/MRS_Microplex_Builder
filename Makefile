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
export CW_INSTALL_DIR

export CURDIR	:= $(dir $(lastword $(MAKEFILE_LIST)))
ifneq ($(words $(CURDIR)),1)
$(error spaces not permitted in parent directory names)
endif
ifeq ($(VERBOSE),)
export V	 = @
endif

export BUILDTOP	?= $(CURDIR)build
export SRCDIR	:= $(CURDIR)
export MCU	:= $(BUILDTOP)/MCU
export RSRCDIR	:= $(CURDIR)resources

export WINE	:= /opt/homebrew/bin/wine
export WINEDEBUG = -all
export CC	:= $(WINE) $(MCU)/prog/chc08.exe
export LD	:= $(WINE) $(MCU)/prog/linker.exe

# library sources
export LIB_SRCS	:= $(wildcard lib/*.c) \
               	   $(wildcard lib/HAL/*.c)

# needed sources from CW MCU
export MCU_SRCS	:= $(MCU)/lib/hc08c/src/start08.c \
		   $(MCU)/lib/hc08c/device/src/mc9s08dz60.c

# needed libraries from CW MCU
export LIBS	:= $(MCU)/lib/hc08c/lib/ansiis.lib


export GIT_VERS	:= $(shell git describe --always --dirty)

BUILTIN_TARGETS	:= clean reformat doc
TARGETS		:= $(filter-out $(BUILTIN_TARGETS),$(MAKECMDGOALS))

.PHONY: $(TARGETS) $(BUILTIN_TARGETS)
.SECONDARY:

$(TARGETS):
	make -f build.mk $@

all:
	@echo "Must supply one or more app names as targets to build"

clean:
	rm -rf $(BUILDTOP)

doc:
	doxygen

FORMAT_SRCS	 = $(shell find $(CURDIR)lib $(CURDIR)src $(CURDIR)include -name "*.[ch]")
REFORMAT_OPTS	 = --style=1tbs \
		   --attach-closing-while \
		   --indent=spaces=4 \
		   --indent-preproc-block \
		   --indent-preproc-define \
		   --indent-cases \
		   --min-conditional-indent=0 \
		   --break-blocks \
		   --pad-oper \
		   --pad-header \
		   --unpad-paren \
		   --add-brackets \
		   --convert-tabs \
		   --align-pointer=name \
		   --keep-one-line-blocks \
		   --break-return-type \
		   --attach-return-type-decl \
		   --convert-tabs\
		   --max-code-length=100 \
		   --break-after-logical \
		   --formatted \
		   --suffix=none \

reformat:
	astyle $(REFORMAT_OPTS)	$(FORMAT_SRCS)

# Make sure that the link to the CW MCU tools are in place before attempting
# to build anything
$(TARGETS): $(MCU)
$(MCU):
	@mkdir -p $(@D)
	@test -d $(CW_INSTALL_DIR) || (echo Must set CW_INSTALL_DIR; exit 1)
	$(V)ln -sf $(abspath $(CW_INSTALL_DIR)) $@

-include $(OBJS:.o=.d)
