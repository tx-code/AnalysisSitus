# Qt is searched manually first (just determine root)
message (STATUS "Processing Qt 3-rd party")

if (NOT DEFINED ${3RDPARTY_QT_DIR} AND ${3RDPARTY_QT_DIR} STREQUAL "")
  ASITUS_FIND_PRODUCT_DIR ("${3RDPARTY_DIR}" Qt 3RDPARTY_QT_DIR_NAME)
  
  if (NOT DEFINED ${3RDPARTY_QT_DIR_NAME} AND ${3RDPARTY_QT_DIR_NAME} STREQUAL "")
    set (3RDPARTY_QT_DIR "" CACHE PATH "" FORCE)
    message (ERROR "... Qt root directory was not found")
  endif()

  # Combine directory name with absolute path and show in GUI
  set (3RDPARTY_QT_DIR "${3RDPARTY_DIR}/${3RDPARTY_QT_DIR_NAME}" CACHE PATH "The directory containing Qt" FORCE)
  message (STATUS "... Qt root directory: ${3RDPARTY_QT_DIR}")
endif()

# Now set CMAKE_PREFIX_PATH to point to local Qt installation.
# Without this setting find_package() will not work
set(CMAKE_PREFIX_PATH ${3RDPARTY_QT_DIR})

# Now we can apply standard CMake finder for Qt5. We do this mostly
# to have qt5_wrap_cpp() function available
find_package(Qt5 REQUIRED COMPONENTS Widgets Core)
#
if (NOT WIN32)
  find_package(Qt5 REQUIRED COMPONENTS X11Extras)
  mark_as_advanced (Qt5X11Extras_DIR)
endif()
#
message (STATUS "... Qt cmake configuration at ${Qt5_DIR}")
find_program(QMAKE_EXECUTABLE NAMES qmake HINTS ${QTDIR} ENV QTDIR PATH_SUFFIXES bin)
execute_process(COMMAND ${QMAKE_EXECUTABLE} -query QT_VERSION OUTPUT_VARIABLE QT_VERSION)
string(REGEX REPLACE "\n$" "" QT_VERSION "${QT_VERSION}")
message (STATUS "... Qt version: ${QT_VERSION}")

# Hide specific paths
mark_as_advanced (Qt5Gui_DIR)
mark_as_advanced (Qt5Widgets_DIR)
mark_as_advanced (Qt5Network_DIR)
mark_as_advanced (Qt5Core_DIR)
mark_as_advanced (Qt5_DIR)

#--------------------------------------------------------------------------
# Installation
#--------------------------------------------------------------------------

if (NOT BUILD_ALGO_ONLY)
  if (WIN32)
    message (STATUS "... Qt libraries: ${3RDPARTY_QT_DIR}")

    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Core.dll    CONFIGURATIONS Release DESTINATION bin)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Gui.dll     CONFIGURATIONS Release DESTINATION bin)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Widgets.dll CONFIGURATIONS Release DESTINATION bin)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Svg.dll     CONFIGURATIONS Release DESTINATION bin)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Network.dll CONFIGURATIONS Release DESTINATION bin)

    if (USE_OCCT_INSPECTOR)
      install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Xml.dll   CONFIGURATIONS Release DESTINATION bin)
    endif()

    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Core.dll     CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Gui.dll      CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Widgets.dll  CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Svg.dll      CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Network.dll  CONFIGURATIONS RelWithDebInfo DESTINATION bini)

    if (USE_OCCT_INSPECTOR)
      install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Xmld.dll   CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    endif()

    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Cored.dll    CONFIGURATIONS Debug DESTINATION bind)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Guid.dll     CONFIGURATIONS Debug DESTINATION bind)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Widgetsd.dll CONFIGURATIONS Debug DESTINATION bind)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Svgd.dll     CONFIGURATIONS Debug DESTINATION bind)
    install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Networkd.dll CONFIGURATIONS Debug DESTINATION bind)

    if (USE_OCCT_INSPECTOR)
      install (FILES ${3RDPARTY_QT_DIR}/bin/Qt5Xmld.dll   CONFIGURATIONS Debug DESTINATION bind)
    endif()

    install (DIRECTORY ${3RDPARTY_QT_DIR}/plugins/imageformats/   CONFIGURATIONS Release DESTINATION bin/imageformats/)
    install (DIRECTORY ${3RDPARTY_QT_DIR}/plugins/platforms/      CONFIGURATIONS Release DESTINATION bin/platforms/)
    install (DIRECTORY ${3RDPARTY_QT_DIR}/qml/Qt/                 CONFIGURATIONS Release DESTINATION bin/Qt/)
    install (DIRECTORY ${3RDPARTY_QT_DIR}/qml/QtGraphicalEffects/ CONFIGURATIONS Release DESTINATION bin/QtGraphicalEffects/)

    install (DIRECTORY ${3RDPARTY_QT_DIR}/plugins/imageformats/   CONFIGURATIONS RelWithDebInfo DESTINATION bini/imageformats/)
    install (DIRECTORY ${3RDPARTY_QT_DIR}/plugins/platforms/      CONFIGURATIONS RelWithDebInfo DESTINATION bini/platforms/)
    install (DIRECTORY ${3RDPARTY_QT_DIR}/qml/Qt/                 CONFIGURATIONS RelWithDebInfo DESTINATION bini/Qt/)
    install (DIRECTORY ${3RDPARTY_QT_DIR}/qml/QtGraphicalEffects/ CONFIGURATIONS RelWithDebInfo DESTINATION bini/QtGraphicalEffects/)

    install (DIRECTORY ${3RDPARTY_QT_DIR}/plugins/imageformats/   CONFIGURATIONS Debug DESTINATION bind/imageformats/)
    install (DIRECTORY ${3RDPARTY_QT_DIR}/plugins/platforms/      CONFIGURATIONS Debug DESTINATION bind/platforms/)
    install (DIRECTORY ${3RDPARTY_QT_DIR}/qml/Qt/                 CONFIGURATIONS Debug DESTINATION bind/Qt/)
    install (DIRECTORY ${3RDPARTY_QT_DIR}/qml/QtGraphicalEffects/ CONFIGURATIONS Debug DESTINATION bind/QtGraphicalEffects/)
  endif()
endif()
