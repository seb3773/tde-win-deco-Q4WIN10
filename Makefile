# Q4WIN10 Window Decoration - Standalone Makefile
# (C) 2026 Q4OS / TDE Developers

# Paths
TDE_PREFIX ?= /opt/trinity
TDE_INCLUDE ?= $(TDE_PREFIX)/include
TDE_LIB ?= $(TDE_PREFIX)/lib
TQT_INCLUDE ?= /usr/include/tqt3
PLUGIN_DIR ?= $(TDE_PREFIX)/lib/trinity
DESKTOP_DIR ?= $(TDE_PREFIX)/share/apps/twin

# Tools
CXX ?= g++
MOC := $(shell which tmoc moc-tqt 2>/dev/null | head -n 1)
UIC := $(shell which uic-tqt 2>/dev/null | head -n 1)
SSTRIP := $(shell which sstrip 2>/dev/null)

# Aggressive Optimization Flags (matching TDE Classic-X Applet)
CXXFLAGS := -fPIC \
    -I. \
    -I$(TDE_INCLUDE) \
    -I$(TQT_INCLUDE) \
    -I$(TQT_INCLUDE)/../tqt \
    -include tqt.h \
    -DTQT_NO_ASCII_CAST -DTQT_NO_STL -DTQT_NO_COMPAT \
    -DTQT_NO_TRANSLATION -DTQT_THREAD_SUPPORT -D_REENTRANT \
    -DQT_PLUGIN -D_DEFAULT_SOURCE -DNDEBUG \
    -O2 -flto=auto -fstrict-aliasing \
    -ffunction-sections -fdata-sections \
    -fno-ident -fno-plt -fomit-frame-pointer \
    -fno-stack-protector -fno-math-errno \
    -fvisibility=hidden -fvisibility-inlines-hidden \
    -fmerge-all-constants -fno-asynchronous-unwind-tables \
    -std=c++03 -fno-threadsafe-statics -fno-exceptions \
    -Wno-deprecated-declarations -fpermissive

LDFLAGS := -shared -Wl,-O1 -Wl,-Bsymbolic -Wl,--gc-sections -Wl,--as-needed \
    -Wl,--build-id=none -Wl,-z,norelro -flto=auto -s -Wl,--no-undefined \
    -L$(TDE_LIB) \
    -ltdecorations -ltdeui -ltdecore -ltdefx -ltqt-mt -lX11

# Sources
MAIN_SRCS := q4win10.cpp q4win10client.cpp q4win10button.cpp
CONFIG_SRCS := config/config.cpp config/configdialog.cpp config/moc_configdialog.cpp

# Generated files
MAIN_MOCS := q4win10.moc q4win10button.moc
CONFIG_MOCS := config/config.moc
UI_HEADER := config/configdialog.h
UI_SOURCE := config/configdialog.cpp
UI_MOC := config/moc_configdialog.cpp

# Targets
MAIN_TARGET := twin3_q4win10.so
CONFIG_TARGET := config/twin_q4win10_config.so

.PHONY: all clean install

all: $(MAIN_TARGET) $(CONFIG_TARGET)
	@echo "Build complete!"
	@ls -lh $(MAIN_TARGET) $(CONFIG_TARGET)

# MOC generation
%.moc: %.h
	$(MOC) $< -o $@

config/config.moc: config/config.h
	$(MOC) $< -o $@

$(UI_MOC): $(UI_HEADER)
	$(MOC) $< -o $@

# UI generation
$(UI_HEADER): config/configdialog.ui
	$(UIC) $< -o $@

$(UI_SOURCE): config/configdialog.ui $(UI_HEADER)
	$(UIC) -impl $(UI_HEADER) $< -o $@

# Main decoration plugin
$(MAIN_TARGET): $(MAIN_MOCS) $(MAIN_SRCS)
	@$(CXX) $(CXXFLAGS) $(MAIN_SRCS) -o $@ $(LDFLAGS)
	@if [ -n "$(SSTRIP)" ]; then $(SSTRIP) $@ 2>/dev/null || true; else strip --strip-all $@ 2>/dev/null || true; fi

# Logo header generation
config/q4win10_logo.h: tdewin_128_16.png convert_images.py
	python3 ./convert_images.py

# Config plugin
$(CONFIG_TARGET): config/q4win10_logo.h $(UI_HEADER) $(UI_SOURCE) $(UI_MOC) $(CONFIG_MOCS) $(CONFIG_SRCS)
	@$(CXX) $(CXXFLAGS) -Iconfig $(CONFIG_SRCS) -o $@ $(LDFLAGS)
	@if [ -n "$(SSTRIP)" ]; then $(SSTRIP) $@ 2>/dev/null || true; else strip --strip-all $@ 2>/dev/null || true; fi

install: all
	install -d $(DESTDIR)$(PLUGIN_DIR)
	install -d $(DESTDIR)$(DESKTOP_DIR)
	install -m 755 $(MAIN_TARGET) $(DESTDIR)$(PLUGIN_DIR)/
	install -m 755 $(CONFIG_TARGET) $(DESTDIR)$(PLUGIN_DIR)/
	install -m 644 q4win10.desktop $(DESTDIR)$(DESKTOP_DIR)/
	@echo "Installed plugins to $(DESTDIR)$(PLUGIN_DIR)/"
	@echo "Installed .desktop to $(DESTDIR)$(DESKTOP_DIR)/"
	@echo "Run: tdebuildsycoca"

clean:
	rm -f $(MAIN_TARGET) $(CONFIG_TARGET)
	rm -f $(MAIN_MOCS) $(CONFIG_MOCS)
	rm -f $(UI_HEADER) $(UI_SOURCE) $(UI_MOC)
	rm -f *.o config/*.o
