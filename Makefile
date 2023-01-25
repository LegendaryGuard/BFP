#
# edawn Makefile
#
# GNU Make required
#

COMPILE_PLATFORM=$(shell uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
COMPILE_ARCH=$(shell uname -m | sed -e 's/i.86/x86/' | sed -e 's/^arm.*/arm/')

ifeq ($(COMPILE_PLATFORM),mingw32)
  ifeq ($(COMPILE_ARCH),i386)
    COMPILE_ARCH=x86
  endif
endif

#############################################################################
#
# If you require a different configuration from the defaults below, create a
# new file named "Makefile.local" in the same directory as this file and define
# your parameters there. This allows you to change configuration without
# causing problems with keeping up to date with the repository.
#
#############################################################################
-include Makefile.local

ifeq ($(COMPILE_PLATFORM),cygwin)
  PLATFORM=mingw32
endif

ifndef PLATFORM
PLATFORM=$(COMPILE_PLATFORM)
endif
export PLATFORM

ifeq ($(PLATFORM),mingw32)
  MINGW=1
endif
ifeq ($(PLATFORM),mingw64)
  MINGW=1
endif

ifeq ($(COMPILE_ARCH),i86pc)
  COMPILE_ARCH=x86
endif

ifeq ($(COMPILE_ARCH),amd64)
  COMPILE_ARCH=x86_64
endif
ifeq ($(COMPILE_ARCH),x64)
  COMPILE_ARCH=x86_64
endif

ifndef ARCH
ARCH=$(COMPILE_ARCH)
endif
export ARCH

ifneq ($(PLATFORM),$(COMPILE_PLATFORM))
  CROSS_COMPILING=1
else
  CROSS_COMPILING=0

  ifneq ($(ARCH),$(COMPILE_ARCH))
    CROSS_COMPILING=1
  endif
endif
export CROSS_COMPILING

#ifndef MOD_CFLAGS
#MOD_CFLAGS= -std=c99
#endif

ifndef BUILD_DIR
BUILD_DIR=build
endif

ifndef TEMPDIR
TEMPDIR=/tmp
endif

ifndef MOUNT_DIR
MOUNT_DIR=source
endif

ifndef DEBUG_CFLAGS
DEBUG_CFLAGS=-g -O0
endif

#############################################################################

BD=$(BUILD_DIR)/debug-$(PLATFORM)-$(ARCH)
BR=$(BUILD_DIR)/release-$(PLATFORM)-$(ARCH)

QADIR=$(MOUNT_DIR)/game
CGDIR=$(MOUNT_DIR)/cgame
UIDIR=$(MOUNT_DIR)/q3_ui

bin_path=$(shell which $(1) 2> /dev/null)

#############################################################################
# SETUP AND BUILD -- LINUX
#############################################################################

## Defaults
LIB=lib

INSTALL=install
MKDIR=mkdir -p

ifeq ($(PLATFORM),linux)

  ifeq ($(ARCH),x86_64)
    LIB=lib64
  endif

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes -pipe

  OPTIMIZE = -O2 -fvisibility=hidden -fomit-frame-pointer -ffast-math

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC -fvisibility=hidden
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  LIBS= -lm

  ifeq ($(ARCH),x86)
    BASE_CFLAGS += -m32
  endif

endif #Linux


ifdef MINGW

  ifeq ($(CROSS_COMPILING),1)
    # If CC is already set to something generic, we probably want to use
    # something more specific
    ifneq ($(findstring $(strip $(CC)),cc gcc),)
      CC=
    endif

    # We need to figure out the correct gcc and windres
    ifeq ($(ARCH),x86_64)
      MINGW_PREFIXES=x86_64-w64-mingw32 amd64-mingw32msvc
      STRIP=x86_64-w64-mingw32-strip
    endif
    ifeq ($(ARCH),x86)
      MINGW_PREFIXES=i686-w64-mingw32 i586-mingw32msvc i686-pc-mingw32
    endif

    ifndef CC
      CC=$(firstword $(strip $(foreach MINGW_PREFIX, $(MINGW_PREFIXES), \
         $(call bin_path, $(MINGW_PREFIX)-gcc))))
    endif

#   STRIP=$(MINGW_PREFIX)-strip -g

    ifndef WINDRES
      WINDRES=$(firstword $(strip $(foreach MINGW_PREFIX, $(MINGW_PREFIXES), \
         $(call bin_path, $(MINGW_PREFIX)-windres))))
    endif
  else
    # Some MinGW installations define CC to cc, but don't actually provide cc,
    # so check that CC points to a real binary and use gcc if it doesn't
    ifeq ($(call bin_path, $(CC)),)
      CC=gcc
    endif

  endif

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes -pipe

  OPTIMIZE = -O2 -fvisibility=hidden -fomit-frame-pointer -ffast-math

  SHLIBEXT=dll
  SHLIBCFLAGS=-fPIC -fvisibility=hidden
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  LIBS= -lm

  ifeq ($(ARCH),x86)
    BASE_CFLAGS += -m32
  endif

endif

# BUILD_DLL option, if user wants to compile and get .dll files
ifndef BUILD_DLL
  BUILD_DLL    =
endif

ifeq ($(BUILD_DLL), 1)
  BD=$(BUILD_DIR)/debug-$(ARCH)-windows
  BR=$(BUILD_DIR)/release-$(ARCH)-windows

  ifeq ($(CROSS_COMPILING),1)
    # If CC is already set to something generic, we probably want to use
    # something more specific
    ifneq ($(findstring $(strip $(CC)),cc gcc),)
      CC=
    endif
  else
    ifeq ($(call bin_path, $(CC)),)
      CC=gcc
    endif
  endif

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes -pipe

  OPTIMIZE = -O2 -fvisibility=hidden -fomit-frame-pointer -ffast-math

  SHLIBEXT=dll
  SHLIBCFLAGS=-fPIC -fvisibility=hidden
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  LIBS= -lm

  ifeq ($(ARCH),x86)
    BASE_CFLAGS += -m32
  endif

endif # BUILD_DLL

# BUILD_SO option, if user wants to compile and get .so files
ifndef BUILD_SO
  BUILD_SO    =
endif

ifeq ($(BUILD_SO), 1)
  BD=$(BUILD_DIR)/debug-$(ARCH)-linux
  BR=$(BUILD_DIR)/release-$(ARCH)-linux
  ifeq ($(ARCH),x86_64)
    LIB=lib64
  endif

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes -pipe

  OPTIMIZE = -O2 -fvisibility=hidden -fomit-frame-pointer -ffast-math

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC -fvisibility=hidden
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  LIBS= -lm

  ifeq ($(ARCH),x86)
    BASE_CFLAGS += -m32
  endif

endif # BUILD_SO

TARGETS =

ifndef FULLBINEXT
  FULLBINEXT=.$(ARCH)$(BINEXT)
endif

ifndef SHLIBNAME
  SHLIBNAME=$(ARCH).$(SHLIBEXT)
endif

TARGETS += \
  $(B)/cgame$(SHLIBNAME) \
  $(B)/qagame$(SHLIBNAME) \
  $(B)/ui$(SHLIBNAME)

ifeq ("$(CC)", $(findstring "$(CC)", "clang" "clang++"))
  BASE_CFLAGS += -Qunused-arguments
endif

ifdef DEFAULT_BASEDIR
  BASE_CFLAGS += -DDEFAULT_BASEDIR=\\\"$(DEFAULT_BASEDIR)\\\"
endif

ifeq ($(NO_STRIP),1)
  STRIP_FLAG =
else
  STRIP_FLAG = -s
endif

BASE_CFLAGS += -Wformat=2 -Wno-format-zero-length -Wformat-security -Wno-format-nonliteral
BASE_CFLAGS += -Wstrict-aliasing=2 -Wmissing-format-attribute
BASE_CFLAGS += -Wdisabled-optimization
BASE_CFLAGS += -Werror-implicit-function-declaration

ifeq ($(V),1)
echo_cmd=@:
Q=
else
echo_cmd=@echo
Q=@
endif

define DO_CC
$(echo_cmd) "CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_SHLIB_CC
$(echo_cmd) "SHLIB_CC $<"
$(Q)$(CC) $(MOD_CFLAGS) $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_GAME_CC
$(echo_cmd) "GAME_CC $<"
$(Q)$(CC) $(MOD_CFLAGS) -DQAGAME $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_CGAME_CC
$(echo_cmd) "CGAME_CC $<"
$(Q)$(CC) $(MOD_CFLAGS) -DCGAME $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_UI_CC
$(echo_cmd) "UI_CC $<"
$(Q)$(CC) $(MOD_CFLAGS) -DUI $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

#############################################################################
# MAIN TARGETS
#############################################################################

default: release

all: debug release

debug:
	@$(MAKE) targets B=$(BD) CFLAGS="$(CFLAGS) $(BASE_CFLAGS)" \
	  OPTIMIZE="$(DEBUG_CFLAGS)" V=$(V)

release:
	@$(MAKE) targets B=$(BR) CFLAGS="$(CFLAGS) $(BASE_CFLAGS)" \
	  OPTIMIZE="-DNDEBUG $(OPTIMIZE)" V=$(V)

# Create the build directories, check libraries and print out
# an informational message, then start building
targets: makedirs
	@echo ""
	@echo "Building $(CLIENTBIN) in $(B):"
	@echo "  PLATFORM: $(PLATFORM)"
	@echo "  ARCH: $(ARCH)"
	@echo "  COMPILE_PLATFORM: $(COMPILE_PLATFORM)"
	@echo "  COMPILE_ARCH: $(COMPILE_ARCH)"
	@echo "  CC: $(CC)"
	@echo ""
	@echo "  CFLAGS:"
	-@for i in $(CFLAGS); \
	do \
		echo "    $$i"; \
	done
	-@for i in $(OPTIMIZE); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo "  LDFLAGS:"
	-@for i in $(LDFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo "  LIBS:"
	-@for i in $(LIBS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo "  Output:"
	-@for i in $(TARGETS); \
	do \
		echo "    $$i"; \
	done
	@echo ""

ifneq ($(TARGETS),)
	@$(MAKE) $(TARGETS) V=$(V)
endif

makedirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/cgame ];then $(MKDIR) $(B)/cgame;fi
	@if [ ! -d $(B)/game ];then $(MKDIR) $(B)/game;fi
	@if [ ! -d $(B)/ui ];then $(MKDIR) $(B)/ui;fi

#############################################################################
## BASEQ3 CGAME
#############################################################################
# $(B)/cgame/cg_particles.o \ 

CGOBJ_ = \
  $(B)/cgame/cg_main.o \
  $(B)/cgame/bg_lib.o \
  $(B)/cgame/bg_misc.o \
  $(B)/cgame/bg_pmove.o \
  $(B)/cgame/bg_slidemove.o \
  $(B)/cgame/cg_consolecmds.o \
  $(B)/cgame/cg_draw.o \
  $(B)/cgame/cg_drawtools.o \
  $(B)/cgame/cg_effects.o \
  $(B)/cgame/cg_ents.o \
  $(B)/cgame/cg_event.o \
  $(B)/cgame/cg_info.o \
  $(B)/cgame/cg_localents.o \
  $(B)/cgame/cg_marks.o \
  $(B)/cgame/cg_players.o \
  $(B)/cgame/cg_playerstate.o \
  $(B)/cgame/cg_predict.o \
  $(B)/cgame/cg_scoreboard.o \
  $(B)/cgame/cg_servercmds.o \
  $(B)/cgame/cg_snapshot.o \
  $(B)/cgame/cg_view.o \
  $(B)/cgame/cg_weapons.o \
  \
  $(B)/game/q_math.o \
  $(B)/game/q_shared.o

CGOBJ = $(CGOBJ_) $(B)/cgame/cg_syscalls.o

$(B)/cgame$(SHLIBNAME): $(CGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(CGOBJ)

#############################################################################
## BASEQ3 GAME
#############################################################################

QAOBJ_ = \
  $(B)/game/g_main.o \
  $(B)/game/ai_chat.o \
  $(B)/game/ai_cmd.o \
  $(B)/game/ai_dmnet.o \
  $(B)/game/ai_dmq3.o \
  $(B)/game/ai_main.o \
  $(B)/game/ai_team.o \
  $(B)/game/ai_vcmd.o \
  $(B)/game/bg_lib.o \
  $(B)/game/bg_misc.o \
  $(B)/game/bg_pmove.o \
  $(B)/game/bg_slidemove.o \
  $(B)/game/g_active.o \
  $(B)/game/g_arenas.o \
  $(B)/game/g_bot.o \
  $(B)/game/g_client.o \
  $(B)/game/g_cmds.o \
  $(B)/game/g_combat.o \
  $(B)/game/g_items.o \
  $(B)/game/g_mem.o \
  $(B)/game/g_misc.o \
  $(B)/game/g_missile.o \
  $(B)/game/g_mover.o \
  $(B)/game/g_session.o \
  $(B)/game/g_spawn.o \
  $(B)/game/g_svcmds.o \
  $(B)/game/g_target.o \
  $(B)/game/g_team.o \
  $(B)/game/g_trigger.o \
  $(B)/game/g_utils.o \
  $(B)/game/g_weapon.o \
  \
  $(B)/game/q_math.o \
  $(B)/game/q_shared.o

QAOBJ = $(QAOBJ_) $(B)/game/g_syscalls.o

$(B)/qagame$(SHLIBNAME): $(QAOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(QAOBJ)

#############################################################################
## BASEQ3 UI
#############################################################################

UIOBJ_ = \
  $(B)/ui/ui_main.o \
  $(B)/ui/ui_addbots.o \
  $(B)/ui/ui_atoms.o \
  $(B)/ui/ui_cdkey.o \
  $(B)/ui/ui_cinematics.o \
  $(B)/ui/ui_confirm.o \
  $(B)/ui/ui_connect.o \
  $(B)/ui/ui_controls2.o \
  $(B)/ui/ui_credits.o \
  $(B)/ui/ui_demo2.o \
  $(B)/ui/ui_display.o \
  $(B)/ui/ui_gameinfo.o \
  $(B)/ui/ui_ingame.o \
  $(B)/ui/ui_menu.o \
  $(B)/ui/ui_mfield.o \
  $(B)/ui/ui_mods.o \
  $(B)/ui/ui_network.o \
  $(B)/ui/ui_options.o \
  $(B)/ui/ui_playermodel.o \
  $(B)/ui/ui_players.o \
  $(B)/ui/ui_playersettings.o \
  $(B)/ui/ui_preferences.o \
  $(B)/ui/ui_qmenu.o \
  $(B)/ui/ui_removebots.o \
  $(B)/ui/ui_serverinfo.o \
  $(B)/ui/ui_servers2.o \
  $(B)/ui/ui_setup.o \
  $(B)/ui/ui_sound.o \
  $(B)/ui/ui_sparena.o \
  $(B)/ui/ui_specifyserver.o \
  $(B)/ui/ui_splevel.o \
  $(B)/ui/ui_sppostgame.o \
  $(B)/ui/ui_spskill.o \
  $(B)/ui/ui_startserver.o \
  $(B)/ui/ui_team.o \
  $(B)/ui/ui_teamorders.o \
  $(B)/ui/ui_video.o \
  \
  $(B)/game/bg_lib.o \
  $(B)/game/bg_misc.o \
  $(B)/game/q_math.o \
  $(B)/game/q_shared.o

UIOBJ = $(UIOBJ_) $(B)/ui/ui_syscalls.o

$(B)/ui$(SHLIBNAME): $(UIOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(UIOBJ)

#############################################################################
## GAME MODULE RULES
#############################################################################

$(B)/cgame/bg_%.o: $(QADIR)/bg_%.c
	$(DO_CGAME_CC)

$(B)/cgame/q_%.o: $(QADIR)/q_%.c
	$(DO_CGAME_CC)

$(B)/cgame/%.o: $(CGDIR)/%.c
	$(DO_CGAME_CC)

$(B)/game/%.o: $(QADIR)/%.c
	$(DO_GAME_CC)

$(B)/ui/bg_%.o: $(QADIR)/bg_%.c
	$(DO_UI_CC)

$(B)/ui/%.o: $(UIDIR)/%.c
	$(DO_UI_CC)

#############################################################################
# MISC
#############################################################################

OBJ = $(QAOBJ) $(CGOBJ) $(UIOBJ)

clean: clean-debug clean-release

clean-debug:
	@$(MAKE) clean2 B=$(BD)

clean-release:
	@$(MAKE) clean2 B=$(BR)

clean2:
	@echo "CLEAN $(B)"
	@rm -f $(OBJ)
	@rm -f $(TARGETS)

distclean: clean
	@rm -rf $(BUILD_DIR)

#############################################################################
# DEPENDENCIES
#############################################################################

.PHONY: all clean clean2 clean-debug clean-release \
	debug default distclean makedirs \
	release targets \
	$(OBJ_D_FILES)
