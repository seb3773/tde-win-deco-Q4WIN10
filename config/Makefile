# Q4WIN10 Config Plugin - Standalone Makefile

# Paths
TDEBASE := $(shell cd ../../../../ && pwd)
TWIN_LIB := $(TDEBASE)/twin/lib
TDE_PREFIX := /opt/trinity
TDE_INCLUDE := $(TDE_PREFIX)/include
TDE_LIB := $(TDE_PREFIX)/lib
TQT_INCLUDE := /usr/include/tqt3
PLUGIN_DIR := $(TDE_PREFIX)/lib/trinity

# Tools
CXX := g++
MOC := $(shell which tmoc moc-tqt 2>/dev/null | head -n 1)
UIC := $(shell which uic-tqt 2>/dev/null | head -n 1)

CXXFLAGS := -fPIC \
    -I. -I.. -I$(TWIN_LIB) \
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

# Sources and generated files
SOURCES := config.cpp configdialog.cpp
MOCS := config.moc configdialog.moc
UI_HEADER := configdialog.h
UI_SOURCE := configdialog.cpp

TARGET := twin_q4win10_config.so

.PHONY: all clean install

all: $(TARGET)

# MOC generation
%.moc: %.h
	$(MOC) $< -o $@

# UI generation
$(UI_HEADER): configdialog.ui
	$(UIC) $< -o $@

$(UI_SOURCE): configdialog.ui $(UI_HEADER)
	$(UIC) -impl configdialog.h configdialog.ui -o configdialog.cpp

$(TARGET): $(UI_HEADER) $(UI_SOURCE) $(MOCS) $(SOURCES)
	@$(CXX) $(CXXFLAGS) $(SOURCES) -o $@ $(LDFLAGS)
	if command -v sstrip >/dev/null 2>&1; then sstrip $@ 2>/dev/null || true; else strip --strip-all $@; fi

install: all
	install -d $(DESTDIR)$(PLUGIN_DIR)
	install -m 755 $(TARGET) $(DESTDIR)$(PLUGIN_DIR)/

clean:
	rm -f $(TARGET) $(MOCS) $(UI_HEADER) $(UI_SOURCE)
	rm -f *.o
