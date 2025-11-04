-include config.mk

LOCAL_INCLUDES := -I.
CXXFLAGS += $(LOCAL_INCLUDES)

# -------------------------------
# Fallbacks if ./configure hasn't been run
# -------------------------------
CXX            ?= g++
CC             ?= cc
CXXSTD         ?= --std=c++23
BASE_CXXFLAGS  ?= -w -fdiagnostics-color=always -MMD -MP
OPTFLAGS       ?= -O2 -g
PKG_CFLAGS     ?= -I/usr/include/libxml2
PKG_LIBS       ?= -lxml2 -lcurl -lsqlite3 -ltar -lX11 -lcrypto -lssl -lmagic -lyara
SANITIZER      ?=
BUILD_TYPE     ?= Release

# Final flags (only used if config.mk didn't set them)
CXXFLAGS ?= $(OPTFLAGS) $(BASE_CXXFLAGS) $(CXXSTD) $(PKG_CFLAGS) $(SANITIZER) $(LOCAL_INCLUDES)
LDFLAGS  ?= $(PKG_LIBS) $(SANITIZER)

# -------------------------------
# Installation paths / helpers
# -------------------------------
prefix       ?= $(PREFIX)
prefix       ?= /usr/local
exec_prefix  ?= $(prefix)
bindir       ?= $(exec_prefix)/bin
datadir      ?= $(prefix)/share
mandir       ?= $(datadir)/man

# Packaging root (leave empty for normal installs; set when building packages)
DESTDIR ?=

# GNU-ish install helpers
INSTALL         ?= install
INSTALL_PROGRAM ?= $(INSTALL) -m 0755
INSTALL_DATA    ?= $(INSTALL) -m 0644
INSTALL_DIR     ?= $(INSTALL) -d

# Name to install as (keep the runtime name stable)
INSTALL_AS := grconsole

# -------------------------------
BIN_BASE := grconsole
ifeq ($(BUILD_TYPE),Debug)
  BIN_NAME := $(BIN_BASE)_debug
else
  BIN_NAME := $(BIN_BASE)
endif

# -------------------------------
# Sources / Objects
# -------------------------------
SOURCE	= coverage/coverage.cc \
	coverage/lcov.cc \
	crypto/secrets.cc \
	fuzzer/fuzzer.cc \
	fuzzer/fuzzerPool.cc \
	fuzzer/engines/afl.cc \
	fuzzer/engines/uli.cc \
	github/API.cc \
	global.cc \
	graph/dot.cc \
	graph/node.cc \
	html/html.cc \
	interface/rest.cc \
	llm/llm.cc \
	llm/openai.cc \
	modules/autobuild.cc \
	modules/autoconfig.cc \
	modules/experimental.cc \
	modules/monitor.cc \
	modules/plunger.cc \
	modules/triage.cc \
	mongoose/mongoose.c \
	network/HTTP.cc \
	ossfuzz/ossfuzz.cc \
	utils/error.cc \
	utils/filesys.cc \
	utils/process.cc \
	utils/tar.cc \
	utils/utils.cc \
	utils/x11.cc \
	validator/yara.cc \
	yaml/yaml.cc \
	campaign.cc \
	data.cc \
	db.cc \
	docker.cc \
	init.cc \
	progress.cc

OBJS = ${SOURCE:.cc=.o}
OBJS := ${OBJS:.c=.o}
DEPS := $(OBJS:.o=.d)

# -------------------------------
# Default target
# -------------------------------
.PHONY: all clean debug release install install-strip uninstall
all: $(BIN_NAME)

$(BIN_BASE) $(BIN_BASE)_debug: interface/grconsole.cc $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $< -o $@ $(LDFLAGS)
	@echo "Built $@"

debug:
	@$(MAKE) BUILD_TYPE=Debug

release:
	@$(MAKE) BUILD_TYPE=Release

updater: interface/updater.cc
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# -------------------------------
# Installation
# -------------------------------
install: $(BIN_NAME)
	$(INSTALL_DIR) "$(DESTDIR)$(bindir)"
	$(INSTALL_PROGRAM) "$(BIN_NAME)" "$(DESTDIR)$(bindir)/$(INSTALL_AS)"
	@echo "Installed $(BIN_NAME) -> $(DESTDIR)$(bindir)/$(INSTALL_AS)"

install-strip: $(BIN_NAME)
	$(INSTALL_DIR) "$(DESTDIR)$(bindir)"
	strip "$(BIN_NAME)"
	$(INSTALL_PROGRAM) "$(BIN_NAME)" "$(DESTDIR)$(bindir)/$(INSTALL_AS)"
	@echo "Installed (stripped) $(BIN_NAME) -> $(DESTDIR)$(bindir)/$(INSTALL_AS)"

uninstall:
	rm -f "$(DESTDIR)$(bindir)/$(INSTALL_AS)"
	@echo "Uninstalled $(DESTDIR)$(bindir)/$(INSTALL_AS)"

# Optionally, manpages or data (add files as needed):
#	$(INSTALL_DIR) "$(DESTDIR)$(mandir)/man1"
#	$(INSTALL_DATA) docs/grconsole.1 "$(DESTDIR)$(mandir)/man1/grconsole.1"

# -------------------------------
# Compilation rules
# -------------------------------
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC)  $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

# -------------------------------
clean:
	rm -f $(OBJS) $(DEPS) webservice grconsole grconsole_debug updater config.mk

