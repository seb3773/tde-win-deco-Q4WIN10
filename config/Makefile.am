INCLUDES                     = $(all_includes)

kde_module_LTLIBRARIES       = twin_q4win10_config.la

twin_q4win10_config_la_SOURCES = config.cpp configdialog.ui
twin_q4win10_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
twin_q4win10_config_la_LIBADD  = $(LIB_TDEUI)

METASOURCES                  = AUTO
noinst_HEADERS               = config.h
DISTCLEANFILES               = $(METASOURCES)
