COMPILE_PLATFORM=$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'|sed -e 's/\//_/g')
COMPILE_ARCH=$(shell uname -m | sed -e s/i.86/x86/)

ifeq ($(COMPILE_PLATFORM),sunos)
	COMPILE_ARCH=$(shell uname -p | sed -e s/i.86/x86/)
endif
ifeq ($(COMPILE_PLATFORM),darwin)
	COMPILE_ARCH=$(shell uname -p | sed -e s/i.86/x86/)
endif

ifeq ($(COMPILE_PLATFORM),mingw32)
ifeq ($(COMPILE_ARCH),x86_64)
	COMPILE_ARCH=x64
endif
endif


ifndef PLATFORM
PLATFORM=$(COMPILE_PLATFORM)
endif
export PLATFORM

ifeq ($(COMPILE_ARCH),powerpc)
	COMPILE_ARCH=ppc
endif
ifeq ($(COMPILE_ARCH),powerpc64)
	COMPILE_ARCH=ppc64
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

ifndef VERSION
VERSION=1.0
endif

ifndef CLIENTBIN
CLIENTBIN=lecista
endif

ifndef MOUNT_DIR
MOUNT_DIR=src
endif

ifndef BUILD_DIR
BUILD_DIR=build
endif

ifndef TEMPDIR
TEMPDIR=/tmp
endif

ifndef DEBUG_CXXFLAGS
DEBUG_CXXFLAGS=-g -O0 -DDEBUG
endif

BD=$(BUILD_DIR)/debug-$(PLATFORM)-$(ARCH)
BR=$(BUILD_DIR)/release-$(PLATFORM)-$(ARCH)
LDIR=$(MOUNT_DIR)/library
LIBSDIR=$(MOUNT_DIR)/libs
TEMPDIR=/tmp

bin_path=$(shell which $(1) 2> /dev/null)

# Add git version info
USE_GIT=
ifeq ($(wildcard .git),.git)
	GIT_REV=$(shell git show -s --pretty=format:%h-%ad --date=short)
ifneq ($(GIT_REV),)
	VERSION:=$(VERSION)_GIT_$(GIT_REV)
	USE_GIT=1
endif
endif

BASE_CXXFLAGS = -Wall -fno-strict-aliasing
DO_CXXFLAGS = -std=c++0x

#############################################################################
# SETUP AND BUILD -- LINUX
#############################################################################

ifneq (,$(findstring "$(PLATFORM)", "linux" "gnu_kfreebsd" "kfreebsd-gnu"))
	BASE_CXXFLAGS += -pipe
	CXX = clang++
else # ifeq Linux

#############################################################################
# SETUP AND BUILD -- MINGW32
#############################################################################

ifeq ($(PLATFORM),mingw32)

# Some MinGW installations define CC to cc, but don't actually provide cc,
# so explicitly use gcc instead (which is the only option anyway)
ifeq ($(call bin_path, $(CXX)),)
	CXX=g++
endif

	SHLIBEXT=dll
	SHLIBCXXFLAGS=
	SHLIBLDFLAGS=-shared $(LDFLAGS)

	BINEXT=.exe

ifeq ($(ARCH),x86)
# build 32bit
	BASE_CXXFLAGS += -m32
else
	BASE_CXXFLAGS += -m64
endif
	CLIENT_LIBS += -lmingw32
else # ifeq mingw32

#############################################################################
# SETUP AND BUILD -- GENERIC
#############################################################################
	BASE_CXXFLAGS=
	OPTIMIZE = -O3

	SHLIBEXT=so
	SHLIBCXXFLAGS=-fPIC
	SHLIBLDFLAGS=-shared

endif #Linux
endif #mingw32

ifndef FULLBINEXT
	FULLBINEXT=.$(ARCH)$(BINEXT)
endif

MKDIR = mkdir
TARGETS=$(B)/$(CLIENTBIN)$(FULLBINEXT)
ifeq ($(V),1)
echo_cmd=@:
Q=
else
echo_cmd=@echo
Q=@
endif

define DO_CXX
$(echo_cmd) "CXX $<"
$(Q)$(CXX) $(CXXFLAGS) $(DO_CXXFLAGS) -o $@ -c $<
endef


## LIBS
LIBS += -lboost_system \
	-lboost_filesystem \
	-lboost_thread \
	-lboost_date_time \
	-lboost_chrono \
	-lcrypto \
	-lleveldb \
	-lpthread

#############################################################################
# MAIN TARGETS
#############################################################################

default: release
all: debug release

debug:
	@$(MAKE) targets B=$(BD) CXXFLAGS="$(CXXFLAGS) $(BASE_CXXFLAGS) $(DEBUG_CXXFLAGS)" \
	OPTIMIZE="$(DEBUG_CXXFLAGS)"

release:
	@$(MAKE) targets B=$(BR) CXXFLAGS="$(CXXFLAGS) $(BASE_CXXFLAGS)" \
	OPTIMIZE="$(OPTIMIZE)"

# Create the build directories, check libraries and print out
# an informational message, then start building

targets: makedirs
	@echo ""
	@echo "Building $(CLIENTBIN) in $(B):"
	@echo " PLATFORM: $(PLATFORM)"
	@echo " ARCH: $(ARCH)"
	@echo " VERSION: $(VERSION)"
	@echo " COMPILE_PLATFORM: $(COMPILE_PLATFORM)"
	@echo " COMPILE_ARCH: $(COMPILE_ARCH)"
	@echo " CXX: $(CXX)"
	@echo ""
	@echo " CXXFLAGS:"
	-@for i in $(CXXFLAGS); \
	do \
	echo " $$i"; \
	done
	-@for i in $(OPTIMIZE); \
	do \
	echo " $$i"; \
	done
	@echo ""
	@echo " LDFLAGS:"
	-@for i in $(LDFLAGS); \
	do \
	echo " $$i"; \
	done
	@echo ""
	@echo " LIBS:"
	-@for i in $(LIBS); \
	do \
	echo " $$i"; \
	done
	@echo ""
	@echo " Output:"
	-@for i in $(TARGETS); \
	do \
	echo " $$i"; \
	done
	@echo ""

ifneq ($(TARGETS),)
	@$(MAKE) $(TARGETS) V=$(V)
endif

makedirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/library ];then $(MKDIR) $(B)/library;fi
	@if [ ! -d $(B)/library/filesystem ];then $(MKDIR) $(B)/library/filesystem;fi
	@if [ ! -d $(B)/library/network ];then $(MKDIR) $(B)/library/network;fi

#############################################################################
# LIBRARY
#############################################################################

LOBJ = \
	$(B)/library/filesystem/Config.o \
	$(B)/library/filesystem/DirectoryExplorer.o \
	$(B)/library/filesystem/Hash.o \
	$(B)/library/filesystem/Hasher.o \
	$(B)/library/filesystem/HashDatabase.o \
	$(B)/library/filesystem/HashTree.o
	#$(B)/library/filesystem/main.o

LOBJ = \
	$(B)/library/network/main.o \
	$(B)/library/network/IOHandler.o \
	$(B)/library/network/MulticastNetwork.o \
	$(B)/library/network/MulticastHandler.o \
	$(B)/library/network/MulticastGateway.o \
	$(B)/library/network/GatewayElection.o

$(B)/$(CLIENTBIN)$(FULLBINEXT): $(LOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(LOBJ) $(LIBS)

#############################################################################
## LIBRARY RULES
#############################################################################

$(B)/library/%.o: $(LDIR)/%.cpp
	$(DO_CXX)

$(B)/library/filesystem/%.o: $(LDIR)/filesystem/%.cpp
	$(DO_CXX)

$(B)/library/network/%.o: $(LDIR)/network/%.cpp
	$(DO_CXX)

#############################################################################
# MISC
#############################################################################

OBJ = $(LOBJ)

clean: clean-debug clean-release

clean-debug:
	@$(MAKE) clean2 B=$(BD)

clean-release:
	@$(MAKE) clean2 B=$(BR)

clean2:
	@echo "CLEAN $(B)"
	@rm -f $(OBJ)

distclean: clean
	@rm -rf $(BUILD_DIR)

dist:
	git archive --format zip --output $(CLIENTBIN)-$(VERSION).zip HEAD

doc:
	doxygen Doxyfile

.PHONY: all clean clean2 clean-debug clean-release copyfiles \
debug default dist distclean makedirs \
release targets doc
