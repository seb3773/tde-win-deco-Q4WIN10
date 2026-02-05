# Q4WIN10 Window Decoration - Standalone Makefile

# Paths
TDEBASE := $(shell cd ../../../ && pwd)
TWIN_LIB := $(TDEBASE)/twin/lib
TDE_PREFIX := /opt/trinity
TDE_INCLUDE := $(TDE_PREFIX)/include
TDE_LIB := $(TDE_PREFIX)/lib
TQT_INCLUDE := /usr/include/tqt3
PLUGIN_DIR := $(TDE_PREFIX)/lib/trinity
DESKTOP_DIR := $(TDE_PREFIX)/share/apps/twin

# Tools
CXX := g++
MOC := $(shell which tmoc moc-tqt 2>/dev/null | head -n 1)
UIC := $(shell which uic-tqt 2>/dev/null | head -n 1)

CXXFLAGS := -fPIC \
    -I. -I$(TWIN_LIB) \
    -I$(TDE_INCLUDE) \
    -I$(TQT_INCLUDE) \
    -I$(TQT_INCLUDE)/../tqt \
    -include tqt.h \
    -DTQT_NO_ASCII_CAST -DTQT_NO_STL -DTQT_NO_COMPAT \
    -DTQT_NO_TRANSLATION -DTQT_THREAD_SUPPORT -D_REENTRANT \
    -DQT_PLUGIN -D_DEFAULT_SOURCE -DNDEBUG \
    -O2 \
    -fdata-sections -ffunction-sections -fomit-frame-pointer \
    -ffast-math -fmerge-all-constants -flto

LDFLAGS := -shared -Wl,--gc-sections -Wl,--as-needed -flto -O2 \
    -L$(TDE_LIB) -L$(TDEBASE)/build/twin/lib \
    -ltdecorations -ltdeui -ltdecore -ltdefx -ltqt-mt

# Sources
MAIN_SRCS := q4win10.cpp q4win10client.cpp q4win10button.cpp
CONFIG_SRCS := config/config.cpp config/configdialog.cpp

# Generated files
MAIN_MOCS := q4win10.moc q4win10button.moc
CONFIG_MOCS := config/config.moc config/configdialog.moc
UI_HEADER := config/configdialog.h
UI_SOURCE := config/configdialog.cpp

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

config/%.moc: config/%.h
	$(MOC) $< -o $@

# UI generation
$(UI_HEADER): config/configdialog.ui
	$(UIC) $< -o $@

$(UI_SOURCE): config/configdialog.ui $(UI_HEADER)
	$(UIC) -impl $(UI_HEADER) $< -o $@

# Main decoration plugin
$(MAIN_TARGET): $(MAIN_MOCS) $(MAIN_SRCS)
	@$(CXX) $(CXXFLAGS) $(MAIN_SRCS) -o $@ $(LDFLAGS)
	@if command -v sstrip >/dev/null 2>&1; then sstrip $@ 2>/dev/null || true; else strip --strip-all $@; fi

# Config plugin
$(CONFIG_TARGET): $(UI_HEADER) $(UI_SOURCE) $(CONFIG_MOCS) $(CONFIG_SRCS)
	@$(CXX) $(CXXFLAGS) -Iconfig $(CONFIG_SRCS) -o $@ $(LDFLAGS)
	@if command -v sstrip >/dev/null 2>&1; then sstrip $@ 2>/dev/null || true; else strip --strip-all $@; fi

install: all
	install -d $(DESTDIR)$(PLUGIN_DIR)
	install -d $(DESTDIR)$(DESKTOP_DIR)
	install -m 755 $(MAIN_TARGET) $(DESTDIR)$(PLUGIN_DIR)/
	install -m 755 $(CONFIG_TARGET) $(DESTDIR)$(PLUGIN_DIR)/
	install -m 644 q4win10.desktop $(DESTDIR)$(DESKTOP_DIR)/
	@echo "Installed plugins to $(DESTDIR)$(PLUGIN_DIR)/"
	@echo "Installed .desktop to $(DESTDIR)$(DESKTOP_DIR)/"
	@echo "Run: tdebuildsyscoca && dcop twin default restart"

clean:
	rm -f $(MAIN_TARGET) $(CONFIG_TARGET)
	rm -f $(MAIN_MOCS) $(CONFIG_MOCS)
	rm -f $(UI_HEADER) $(UI_SOURCE)
	rm -f *.o config/*.o
