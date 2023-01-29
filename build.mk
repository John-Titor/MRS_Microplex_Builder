#
# Build an app for an MRS Microplex 7 module using the CodeWarrior 
# MCU binaries.
#
# Don't call this directly - use the top-level Makefile.

export APP	:= $(MAKECMDGOALS)
include $(APP)/app.mk

BUILDDIR	:= $(BUILDTOP)/$(APP)
GLOBAL_DEPS	:= $(MAKEFILE_LIST)				\
		   $(wildcard $(RSRCDIR)/*)			\
		   $(MCU)

# export preprocessor defines
export DEFINES	:= $(addprefix -D,$(APP_DEFINES)) \
		   -D__NO_FLOAT__

# export these so the linker can pick it up from its config
export OBJS	:= $(patsubst %.c,$(BUILDDIR)/%.o,$(APP_SRCS) $(LIB_SRCS)) \
		   $(patsubst $(MCU)/lib/%.c,$(BUILDDIR)/mcu_lib/%.o,$(MCU_SRCS))

# make the linker put the map file somewhere sensible
export TEXTPATH	 = $(BUILDDIR)

# suppress generation of error files
export ERRORFILE = /dev/null

# convert \ to / to enable parsing filenames out of error messages
CONVERT_SLASHES	 = | tr \\ /

APP_ELF		:= $(BUILDDIR)/$(APP).elf
.PHONY: $(APP)
.SECONDARY:
$(APP):	$(APP_ELF)

$(info BUILDDIR $(BUILDDIR))
$(info OBJS $(OBJS))
$(info APP_ELF $(APP_ELF))

# build an application / generate s-records
$(APP_ELF): $(OBJS) $(GLOBAL_DEPS)
	@mkdir -p $(@D)
	@echo ==== LINK $(notdir $@)
	$(V)$(LD) -ArgFile$(CURDIR)/resources/link.args -O$@

# build an object file from a source file in ./src
$(BUILDDIR)/%.o: %.c $(GLOBAL_DEPS)
	@mkdir -p $(@D)
	@rm -f $@
	@echo ==== COMPILE $<
	$(V)$(CC) -ArgFile$(CURDIR)/resources/compile.args	\
		-ObjN=$@ $(abspath $<)				\
		-Lm=$(@:%.o=%.d)				\
		$(CONVERT_SLASHES)

# build an object file from a source file supplied by CW MCU
$(BUILDDIR)/mcu_lib/%.o: $(MCU)/lib/%.c $(GLOBAL_DEPS)
	@mkdir -p $(@D)
	@rm -f $@
	@echo ==== COMPILE $<
	$(V)$(CC) -ArgFile$(CURDIR)/resources/compile.args	\
		-ObjN=$@ $<					\
		-Lm=$(@:%.o=%.d)				\
		$(CONVERT_SLASHES)

-include $(OBJS:.o=.d)
