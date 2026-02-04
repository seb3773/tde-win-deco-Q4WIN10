#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

add_definitions( -DQT_PLUGIN -D_DEFAULT_SOURCE -DNDEBUG -O2 -g -W -Wall -Wchar-subscripts -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fvisibility=hidden -fvisibility-inlines-hidden -fdata-sections -ffunction-sections -fomit-frame-pointer -ffast-math -fmerge-all-constants -flto )
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -Wl,--as-needed -flto -O2")

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### other data ################################

install( FILES  DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### twin_plastik_config (module) ##############

tde_add_kpart( twin_q4win10_config AUTOMOC
  SOURCES config.cpp configdialog.ui
  LINK tdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

add_custom_command(TARGET twin_q4win10_config-module POST_BUILD
  COMMAND sstrip ${CMAKE_CURRENT_BINARY_DIR}/twin_q4win10_config.so
  COMMENT "Super-Stripping (sstrip) twin_q4win10_config.so"
)
