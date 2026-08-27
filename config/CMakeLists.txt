#################################################
#
#  (C) 2026 Q4OS / TDE Developers
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${TDE_LIBRARY_DIRS}
)

##### twin_q4win10_config (module) ##############

tde_add_kpart( twin_q4win10_config AUTOMOC
  SOURCES
    config.cpp
    configdialog.ui
  LINK
    tdeui-shared
    tdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

if( SSTRIP_EXECUTABLE AND Q4WIN10_AGGRESSIVE_FLAGS )
  add_custom_command( TARGET twin_q4win10_config-module POST_BUILD
    COMMAND ${SSTRIP_EXECUTABLE} $<TARGET_FILE:twin_q4win10_config-module>
    COMMENT "Aggressively stripping twin_q4win10_config.so with sstrip"
  )
endif()
