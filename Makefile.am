AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = -I$(srcdir)/../../lib $(all_includes)

twindir = $(kde_datadir)/twin/
twin_DATA = q4win10.desktop

kde_module_LTLIBRARIES = twin3_q4win10.la
twin3_q4win10_la_SOURCES = q4win10.cpp q4win10client.cpp q4win10button.cpp
twin3_q4win10_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
twin3_q4win10_la_LIBADD = $(LIB_TDEUI) ../../lib/libtdecorations.la
twin3_q4win10_la_METASOURCES = AUTO

DISTCLEANFILES = $(twin3_q4win10_la_METASOURCES)
