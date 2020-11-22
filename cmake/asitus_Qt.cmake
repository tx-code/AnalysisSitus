# Qt is searched manually first (just determine root)
message (STATUS "Processing Qt 3-rd party")

if (NOT DEFINED ${3RDPARTY_QT_DIR} AND ${3RDPARTY_QT_DIR} STREQUAL "")
  ASITUS_FIND_PRODUCT_DIR ("${3RDPARTY_DIR}" Qt 3RDPARTY_QT_DIR_NAME)
  
  if (NOT DEFINED ${3RDPARTY_QT_DIR_NAME} AND ${3RDPARTY_QT_DIR_NAME} STREQUAL "")
    message (ERROR "... Qt root directory was not found")
  endif()

  # Combine directory name with absolute path and show in GUI
  set (3RDPARTY_QT_DIR "${3RDPARTY_DIR}/${3RDPARTY_QT_DIR_NAME}" CACHE PATH "The directory containing Qt" FORCE)
  message (STATUS "... Qt root directory: ${3RDPARTY_QT_DIR}")
endif()

# Now set CMAKE_PREFIX_PATH to point to local Qt installation.
# Without this setting find_package() will not work
set(CMAKE_PREFIX_PATH ${3RDPARTY_QT_DIR})

find_program(QMAKE_EXECUTABLE NAMES qmake HINTS ${QTDIR} ENV QTDIR PATH_SUFFIXES bin)
execute_process(COMMAND ${QMAKE_EXECUTABLE} -query QT_VERSION OUTPUT_VARIABLE QT_VERSION)
string(REGEX REPLACE "\n$" "" QT_VERSION "${QT_VERSION}")

string(REPLACE "." ";" QT_VERSIONS_LIST ${QT_VERSION})
list(GET QT_VERSIONS_LIST 0 QT_MAJOR_VERSION)
set (QT_MAJOR_VERSION "${QT_MAJOR_VERSION}" CACHE STRING "Qt major version.")

message (STATUS "... Qt cmake configuration at ${3RDPARTY_QT_DIR}")
message (STATUS "... Qt version: ${QT_VERSION}")

# Now we can apply standard CMake finder for Qt5 (or later). We do this mostly
# to have qt5_wrap_cpp() function available
find_package(Qt${QT_MAJOR_VERSION} REQUIRED COMPONENTS Widgets Core)
#
if (NOT WIN32)
  find_package(Qt${QT_MAJOR_VERSION} REQUIRED COMPONENTS X11Extras)
  mark_as_advanced (Qt${QT_MAJOR_VERSION}X11Extras_DIR)
endif()
#

# Hide specific paths
mark_as_advanced (Qt${QT_MAJOR_VERSION}Gui_DIR)
mark_as_advanced (Qt${QT_MAJOR_VERSION}Widgets_DIR)
mark_as_advanced (Qt${QT_MAJOR_VERSION}Core_DIR)
mark_as_advanced (Qt${QT_MAJOR_VERSION}_DIR)

#--------------------------------------------------------------------------
# Installation
#--------------------------------------------------------------------------

if (WIN32)
  message (STATUS "... Qt libraries: ${3RDPARTY_QT_DIR}")

  install (FILES ${3RDPARTY_QT_DIR}/bin/Qt${QT_MAJOR_VERSION}Core.dll    DESTINATION bin)
  install (FILES ${3RDPARTY_QT_DIR}/bin/Qt${QT_MAJOR_VERSION}Gui.dll     DESTINATION bin)
  install (FILES ${3RDPARTY_QT_DIR}/bin/Qt${QT_MAJOR_VERSION}Widgets.dll DESTINATION bin)
  install (FILES ${3RDPARTY_QT_DIR}/bin/Qt${QT_MAJOR_VERSION}Svg.dll     DESTINATION bin)

  install (DIRECTORY ${3RDPARTY_QT_DIR}/plugins/imageformats/   DESTINATION bin/imageformats/)
  install (DIRECTORY ${3RDPARTY_QT_DIR}/plugins/platforms/      DESTINATION bin/platforms/)
  install (DIRECTORY ${3RDPARTY_QT_DIR}/qml/Qt/                 DESTINATION bin/Qt/)
  install (DIRECTORY ${3RDPARTY_QT_DIR}/qml/QtGraphicalEffects/ DESTINATION bin/QtGraphicalEffects/)
else()
  message (STATUS "... Qt libraries: ${Qt${QT_MAJOR_VERSION}_DIR}")

  install (FILES ${Qt5_DIR}/../../libQt${QT_MAJOR_VERSION}Core.so.${QT_VERSION}    DESTINATION bin)
  install (FILES ${Qt5_DIR}/../../libQt${QT_MAJOR_VERSION}Gui.so.${QT_VERSION}     DESTINATION bin)
  install (FILES ${Qt5_DIR}/../../libQt${QT_MAJOR_VERSION}Widgets.so.${QT_VERSION} DESTINATION bin)
  install (FILES ${Qt5_DIR}/../../libQt${QT_MAJOR_VERSION}Svg.so.${QT_VERSION}     DESTINATION bin)

  install (DIRECTORY ${Qt5_DIR}/../../qt${QT_MAJOR_VERSION}/plugins/imageformats   DESTINATION bin/imageformats/)
  install (DIRECTORY ${Qt5_DIR}/../../qt${QT_MAJOR_VERSION}/plugins/platforms      DESTINATION bin/platforms/)
  install (DIRECTORY ${Qt5_DIR}/../../qt${QT_MAJOR_VERSION}/qml/QtGraphicalEffects DESTINATION bin/QtGraphicalEffects/)
endif()
