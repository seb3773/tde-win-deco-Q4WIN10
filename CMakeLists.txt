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

add_subdirectory( config )

add_definitions( -DQT_PLUGIN -D_DEFAULT_SOURCE -DNDEBUG -O2 -g -W -Wall -Wchar-subscripts -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fvisibility=hidden -fvisibility-inlines-hidden -fdata-sections -ffunction-sections -fomit-frame-pointer -ffast-math -fmerge-all-constants -flto )
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -Wl,--as-needed -flto -O2")

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/twin/lib
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_create_translated_desktop(
  SOURCE q4win10.desktop
  DESTINATION ${DATA_INSTALL_DIR}/twin
  PO_DIR twin-clients-desktops
)


##### twin_plastik (module) ####################

tde_add_kpart( twin3_q4win10 AUTOMOC
  SOURCES q4win10.cpp q4win10client.cpp q4win10button.cpp
  LINK tdecorations-shared tdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

add_custom_command(TARGET twin3_q4win10-module POST_BUILD
  COMMAND sstrip ${CMAKE_CURRENT_BINARY_DIR}/twin3_q4win10.so
  COMMENT "Super-Stripping (sstrip) twin3_q4win10.so"
)
