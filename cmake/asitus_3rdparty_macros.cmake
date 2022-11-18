# Useful macro to work with 3-rd parties

# Include functions
include (${CMAKE_SOURCE_DIR}/cmake/asitus_functions.cmake)

#-------------------------------------------------------------------------------
# Name:    ASITUS_CHECK_PATH_FOR_CONSISTENCY
# Purpose: checks path on consistency
#-------------------------------------------------------------------------------
macro (ASITUS_CHECK_PATH_FOR_CONSISTENCY
       THE_ROOT_PATH_NAME
       THE_BEING_CHECKED_PATH_NAME
       THE_VAR_TYPE
       THE_MESSAGE_OF_BEING_CHECKED_PATH)

  set (THE_ROOT_PATH "${${THE_ROOT_PATH_NAME}}")
  set (THE_BEING_CHECKED_PATH "${${THE_BEING_CHECKED_PATH_NAME}}")

  if (THE_BEING_CHECKED_PATH OR EXISTS "${THE_BEING_CHECKED_PATH}")
    get_filename_component (THE_ROOT_PATH_ABS "${THE_ROOT_PATH}" ABSOLUTE)
    get_filename_component (THE_BEING_CHECKED_PATH_ABS "${THE_BEING_CHECKED_PATH}" ABSOLUTE)

    string (REGEX MATCH "${THE_ROOT_PATH_ABS}" DOES_PATH_CONTAIN "${THE_BEING_CHECKED_PATH_ABS}")

    if (NOT DOES_PATH_CONTAIN) # if cmake found the being checked path at different place from THE_ROOT_PATH_ABS
      set (${THE_BEING_CHECKED_PATH_NAME} "" CACHE ${THE_VAR_TYPE} "${THE_MESSAGE_OF_BEING_CHECKED_PATH}" FORCE)
    endif()
  else()
    set (${THE_BEING_CHECKED_PATH_NAME} "" CACHE ${THE_VAR_TYPE} "${THE_MESSAGE_OF_BEING_CHECKED_PATH}" FORCE)
  endif()

endmacro()

#-------------------------------------------------------------------------------
# Name:    ASITUS_THIRDPARTY_PRODUCT
# Purpose: handles a single 3-rd party product
#-------------------------------------------------------------------------------
macro (ASITUS_THIRDPARTY_PRODUCT
       PRODUCT_NAME
       SUBINC_NAME
       HEADER_NAME
       LIBRARY_NAME)

  message (STATUS "Processing ${PRODUCT_NAME} 3-rd party")

  # Determine platform specification
  # Variables are PLATFORM, COMPILER and COMPILER_BITNESS
  ASITUS_MAKE_PLATFORM_SHORT_NAME ()
  ASITUS_MAKE_COMPILER_SHORT_NAME ()
  ASITUS_MAKE_COMPILER_BITNESS ()

  # Root directories of products
  if (NOT DEFINED 3RDPARTY_${PRODUCT_NAME}_DIR)
    set (3RDPARTY_${PRODUCT_NAME}_DIR ""
         CACHE PATH
         "The directory containing ${PRODUCT_NAME}")
  endif()

  # Specify product folder in connection with 3RDPARTY_DIR
  if (3RDPARTY_DIR AND EXISTS "${3RDPARTY_DIR}")
    if (NOT 3RDPARTY_${PRODUCT_NAME}_DIR OR NOT EXISTS "${3RDPARTY_${PRODUCT_NAME}_DIR}")
      ASITUS_FIND_PRODUCT_DIR ("${3RDPARTY_DIR}" ${PRODUCT_NAME} ${PRODUCT_NAME}_DIR_NAME)
      if (${PRODUCT_NAME}_DIR_NAME)
        set (3RDPARTY_${PRODUCT_NAME}_DIR "${3RDPARTY_DIR}/${${PRODUCT_NAME}_DIR_NAME}" CACHE PATH "The directory containing ${PRODUCT_NAME}" FORCE)
        message (STATUS "... Directory for ${PRODUCT_NAME} 3-rd party is ${3RDPARTY_DIR}/${${PRODUCT_NAME}_DIR_NAME}")
      else()
        message (STATUS "... Warning: directory for ${PRODUCT_NAME} 3-rd party not found")
      endif()
    endif()
  endif()

  if (NOT DEFINED 3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR)
    set (3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR "" CACHE PATH "The path of ${HEADER_NAME}")
  endif()

  if (NOT DEFINED 3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR)
    set (3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR "" CACHE PATH "The directory containing ${PRODUCT_NAME} library")
  endif()

  if (WIN32)
    if (NOT DEFINED 3RDPARTY_${PRODUCT_NAME}_DLL_DIR)
      set (3RDPARTY_${PRODUCT_NAME}_DLL_DIR "" CACHE PATH "The directory containing ${PRODUCT_NAME} dynamic library (DLL)")
    endif()
  endif()

  #----------------------------------------------------------------------------
  # Headers
  if (NOT 3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR OR NOT EXISTS "${3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR}")

    # set 3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR as not found, otherwise find_path can't assign a new value to 3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR
    set (3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR "3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR-NOTFOUND" CACHE FILEPATH "the path to ${HEADER_NAME}" FORCE)

    # suffix with SUBINC_NAME
    if ("${SUBINC_NAME}" STREQUAL "")
      message (STATUS "... Sub-dir hint for includes is empty")
      set (HEADER_SUFFIX_HINT inc include)
    else()
      message (STATUS "... Sub-dir hint for includes is ${SUBINC_NAME}")
      set (HEADER_SUFFIX_HINT inc/${SUBINC_NAME} include/${SUBINC_NAME})
    endif()

    if ("${HEADER_NAME}" STREQUAL "")
      message (STATUS "... No hint on header, so let's assume include directory ${3RDPARTY_${PRODUCT_NAME}_DIR}/include")
      set (3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR ${3RDPARTY_${PRODUCT_NAME}_DIR}/include CACHE FILEPATH "the path to ${HEADER_NAME}" FORCE)
    else ()
      message (STATUS "... Hint on header is ${HEADER_NAME}, so let's search for its containing directory with suffix hint ${HEADER_SUFFIX_HINT}")
      if (3RDPARTY_${PRODUCT_NAME}_DIR AND EXISTS "${3RDPARTY_${PRODUCT_NAME}_DIR}")
        find_path (3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR NAMES ${HEADER_NAME}
                                                        PATHS ${3RDPARTY_${PRODUCT_NAME}_DIR}
                                                        PATH_SUFFIXES ${HEADER_SUFFIX_HINT}
                                                        CMAKE_FIND_ROOT_PATH_BOTH
                                                        NO_DEFAULT_PATH)
      else()
        find_path (3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR NAMES ${HEADER_NAME}
                                                        PATH_SUFFIXES ${HEADER_SUFFIX_HINT}
                                                        CMAKE_FIND_ROOT_PATH_BOTH)
      endif()
    endif()
  endif()
  #
  if (3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR AND EXISTS "${3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR}")
    message (STATUS "... Include dir seems Ok")
    list (APPEND 3RDPARTY_INCLUDE_DIRS "${3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR}")
  else()
    message (STATUS "... Warning: include dir seems bad!")
    list (APPEND 3RDPARTY_NOT_INCLUDED 3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR)

    set (3RDPARTY_${PRODUCT_NAME}_INCLUDE_DIR "" CACHE FILEPATH "The path to ${HEADER_NAME}" FORCE)
  endif()

  #----------------------------------------------------------------------------
  # Libraries
  if (NOT 3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR OR NOT EXISTS "${3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR}")
    message (STATUS "... Attempting to find path by hint library \"${LIBRARY_NAME}\"")

    # suffix for searching of library dirs
    set (LIB_SUFFIX_HINT ${PLATFORM}${COMPILER_BITNESS}/${COMPILER}/lib lib)
    if ("${COMPILER_BITNESS}" STREQUAL "64")
      set (LIB_SUFFIX_HINT ${LIB_SUFFIX_HINT} lib/intel64/${COMPILER})
    else()
      set (LIB_SUFFIX_HINT ${LIB_SUFFIX_HINT} lib/ia32/${COMPILER})
    endif()

    message (STATUS "... Hint suffix for searching of libraries is ${LIB_SUFFIX_HINT}")

    set (3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR "3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR-NOTFOUND" CACHE FILEPATH "The path to ${PRODUCT_NAME} libraries" FORCE)

    if (3RDPARTY_${PRODUCT_NAME}_DIR AND EXISTS "${3RDPARTY_${PRODUCT_NAME}_DIR}")
      find_path (3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR NAMES ${LIBRARY_NAME}.lib ${LIBRARY_NAME}.so
                                                      PATHS ${3RDPARTY_${PRODUCT_NAME}_DIR}
                                                      PATH_SUFFIXES ${LIB_SUFFIX_HINT}
                                                      CMAKE_FIND_ROOT_PATH_BOTH
                                                      NO_DEFAULT_PATH)
    else()
      find_path (3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR NAMES ${LIBRARY_NAME}.lib ${LIBRARY_NAME}.so
                                                      PATH_SUFFIXES ${LIB_SUFFIX_HINT}
                                                      CMAKE_FIND_ROOT_PATH_BOTH)
    endif()
  endif()
  #
  if (3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR AND EXISTS "${3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR}")
    list (APPEND 3RDPARTY_LIBRARY_DIRS "${3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR}")
  else()
    list (APPEND 3RDPARTY_NOT_INCLUDED 3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR)

    set (3RDPARTY_${PRODUCT_NAME}_LIBRARY_DIR "" CACHE FILEPATH "The path to ${PRODUCT_NAME} libraries" FORCE)
  endif()

  #----------------------------------------------------------------------------
  # DLLs for windows
  if (WIN32)
    if (NOT 3RDPARTY_${PRODUCT_NAME}_DLL_DIR OR NOT EXISTS "${3RDPARTY_${PRODUCT_NAME}_DLL_DIR}")
      message (STATUS "... Attempting to find path by hint library \"${LIBRARY_NAME}\"")

      # suffix for searching of library dirs
      set (DLL_SUFFIX_HINT ${PLATFORM}${COMPILER_BITNESS}/${COMPILER}/bin bin)
      if ("${COMPILER_BITNESS}" STREQUAL "64")
        set (DLL_SUFFIX_HINT ${DLL_SUFFIX_HINT} bin/intel64/${COMPILER})
      else()
        set (DLL_SUFFIX_HINT ${DLL_SUFFIX_HINT} bin/ia32/${COMPILER})
      endif()

      message (STATUS "... Hint suffix for searching of DLLs is ${DLL_SUFFIX_HINT}")

      set (3RDPARTY_${PRODUCT_NAME}_DLL_DIR "3RDPARTY_${PRODUCT_NAME}_DLL_DIR-NOTFOUND" CACHE FILEPATH "The path to ${PRODUCT_NAME} DLLs" FORCE)

      if (3RDPARTY_${PRODUCT_NAME}_DIR AND EXISTS "${3RDPARTY_${PRODUCT_NAME}_DIR}")
        find_path (3RDPARTY_${PRODUCT_NAME}_DLL_DIR NAMES ${LIBRARY_NAME}.dll
                                                    PATHS ${3RDPARTY_${PRODUCT_NAME}_DIR}
                                                    PATH_SUFFIXES ${DLL_SUFFIX_HINT}
                                                    CMAKE_FIND_ROOT_PATH_BOTH
                                                    NO_DEFAULT_PATH)
      else()
        find_path (3RDPARTY_${PRODUCT_NAME}_DLL_DIR NAMES ${LIBRARY_NAME}.dll
                                                    PATH_SUFFIXES ${DLL_SUFFIX_HINT}
                                                    CMAKE_FIND_ROOT_PATH_BOTH)
      endif()
    endif()
    #
    if (NOT 3RDPARTY_${PRODUCT_NAME}_DLL_DIR OR NOT EXISTS "${3RDPARTY_${PRODUCT_NAME}_DLL_DIR}")
      set (3RDPARTY_${PRODUCT_NAME}_DLL_DIR "" CACHE FILEPATH "The path to ${PRODUCT_NAME} DLLs" FORCE)
    endif()
  endif()

endmacro()

#-------------------------------------------------------------------------------
# Name:    ASITUS_UNSET_3RDPARTY
# Purpose: unsets variables for a certain 3-rd party
#-------------------------------------------------------------------------------
macro (ASITUS_UNSET_3RDPARTY VARNAME)
  message (STATUS "... Unsetting 3RDPARTY_${VARNAME}_DIR")
  message (STATUS "... Unsetting 3RDPARTY_${VARNAME}_INCLUDE_DIR")
  message (STATUS "... Unsetting 3RDPARTY_${VARNAME}_LIBRARY_DIR")
  message (STATUS "... Unsetting 3RDPARTY_${VARNAME}_DLL_DIR")

  ASITUS_UNSET ("3RDPARTY_${VARNAME}_DIR")
  ASITUS_UNSET ("3RDPARTY_${VARNAME}_INCLUDE_DIR")
  ASITUS_UNSET ("3RDPARTY_${VARNAME}_LIBRARY_DIR")
  ASITUS_UNSET ("3RDPARTY_${VARNAME}_DLL_DIR")
endmacro()

#---------------------------------------------------------------------------------------------------
# Name:    ASITUS_INSTALL_3RDPARTY
# Purpose: Installs 3rd-party libraries to INSTALL_DIR
#---------------------------------------------------------------------------------------------------
macro (ASITUS_INSTALL_3RDPARTY LIBRARIES_LIST_NAME 3RDPARTY_NAME)
  set (LIBRARY_NAME_DEBUG_SUFFIX "")
  if ( ${ARGC} EQUAL 3 )
    set (LIBRARY_NAME_DEBUG_SUFFIX "${ARGV2}")
  endif()

  set (LINUX_LIBRARY_VERSION_SUFFIX "")
  if ( ${ARGC} EQUAL 4 )
    set (LIBRARY_NAME_DEBUG_SUFFIX "${ARGV2}")
    set (LINUX_LIBRARY_VERSION_SUFFIX "${ARGV3}")
  endif()

  set (IS_USE_RELEASE_LIB FALSE)
  if ( ${ARGC} EQUAL 5 )
    set (LIBRARY_NAME_DEBUG_SUFFIX "${ARGV2}")
    set (LINUX_LIBRARY_VERSION_SUFFIX "${ARGV3}")
    set (IS_USE_RELEASE_LIB ${ARGV4})
  endif()

  # Fill auxiliary lists
  if (EXISTS "${3RDPARTY_${3RDPARTY_NAME}_LIBRARY_DIR}")
    list (APPEND 3RDPARTY_LIBRARY_PATH "$<$<CONFIG:RELEASE>:${3RDPARTY_${3RDPARTY_NAME}_LIBRARY_DIR}>")
  endif()
  if (EXISTS "${3RDPARTY_${3RDPARTY_NAME}_LIBRARY_DIR_DEBUG}")
    list (APPEND 3RDPARTY_LIBRARY_PATH "$<$<NOT:$<CONFIG:RELEASE>>:${3RDPARTY_${3RDPARTY_NAME}_LIBRARY_DIR_DEBUG}>")
  endif()

  if (EXISTS "${3RDPARTY_${3RDPARTY_NAME}_DLL_DIR}")
    list (APPEND 3RDPARTY_DLL_PATH "${3RDPARTY_${3RDPARTY_NAME}_DLL_DIR}")
  endif()
  if (EXISTS "${3RDPARTY_${3RDPARTY_NAME}_DLL_DIR_DEBUG}")
    list (APPEND 3RDPARTY_DLL_PATH_DEBUG "${3RDPARTY_${3RDPARTY_NAME}_DLL_DIR_DEBUG}")
  endif()

  # Install includes for SDK
  get_filename_component(3RDPARTY_DIR_NAME "${3RDPARTY_${3RDPARTY_NAME}_DIR}" NAME)
  if (DISTRIBUTION_SDK_WITH_3RDPARTIES)
    install (DIRECTORY "${3RDPARTY_${3RDPARTY_NAME}_INCLUDE_DIR}/" DESTINATION ${SDK_INSTALL_SUBDIR}3rd-parties/${3RDPARTY_DIR_NAME}/include)
  endif()

  if (WIN32)
    foreach (LIB IN LISTS ${LIBRARIES_LIST_NAME})
      # As a software
      if (NOT BUILD_SDK_ONLY)
        # Release
        install (FILES "${3RDPARTY_${3RDPARTY_NAME}_DLL_DIR}/${LIB}.dll" DESTINATION bin CONFIGURATIONS Release OPTIONAL)

        # Release with debug info
        install (FILES "${3RDPARTY_${3RDPARTY_NAME}_DLL_DIR}/${LIB}.dll" DESTINATION bini CONFIGURATIONS RelWithDebInfo OPTIONAL)

        # Debug
        install (FILES "${3RDPARTY_${3RDPARTY_NAME}_DLL_DIR_DEBUG}/${LIB}${LIBRARY_NAME_DEBUG_SUFFIX}.dll" DESTINATION bind CONFIGURATIONS Debug OPTIONAL)
        install (FILES "${3RDPARTY_${3RDPARTY_NAME}_DLL_DIR_DEBUG}/${LIB}${LIBRARY_NAME_DEBUG_SUFFIX}.pdb" DESTINATION bind CONFIGURATIONS Debug OPTIONAL)
      endif()

      # As a framework
      if (DISTRIBUTION_SDK_WITH_3RDPARTIES)
        foreach (TYPE IN ITEMS "DLL" "LIBRARY")
          set (DEST_DIR "bin")
          set (ext "dll")
          set (isBin TRUE)
          if ("x${TYPE}" STREQUAL "xLIBRARY")
            set (DEST_DIR "lib")
            set (ext "lib")
            set (isBin FALSE)
          endif()

          # Release
          install (FILES "${3RDPARTY_${3RDPARTY_NAME}_${TYPE}_DIR}/${LIB}.${ext}" DESTINATION "${SDK_INSTALL_SUBDIR}3rd-parties/${3RDPARTY_DIR_NAME}/${DEST_DIR}" CONFIGURATIONS Release OPTIONAL)

          # Debug
          install (FILES "${3RDPARTY_${3RDPARTY_NAME}_${TYPE}_DIR_DEBUG}/${LIB}${LIBRARY_NAME_DEBUG_SUFFIX}.${ext}" DESTINATION "${SDK_INSTALL_SUBDIR}3rd-parties/${3RDPARTY_DIR_NAME}/${DEST_DIR}d" CONFIGURATIONS Debug OPTIONAL)
          if (isBin)
            install (FILES "${3RDPARTY_${3RDPARTY_NAME}_${TYPE}_DIR_DEBUG}/${LIB}${LIBRARY_NAME_DEBUG_SUFFIX}.pdb" DESTINATION "${SDK_INSTALL_SUBDIR}3rd-parties/${3RDPARTY_DIR_NAME}/${DEST_DIR}d" CONFIGURATIONS Debug OPTIONAL)
          endif()

          # RelWithDebInfo
          install (FILES "${3RDPARTY_${3RDPARTY_NAME}_${TYPE}_DIR}/${LIB}.${ext}" DESTINATION "${SDK_INSTALL_SUBDIR}3rd-parties/${3RDPARTY_DIR_NAME}/${DEST_DIR}i" CONFIGURATIONS RelWithDebInfo OPTIONAL)
        endforeach()
      endif()
    endforeach()
  else()
    foreach (LIB IN LISTS ${LIBRARIES_LIST_NAME})
      if (LINUX_LIBRARY_VERSION_SUFFIX)
        string(REPLACE "." ";" SUF_PARTS "${LINUX_LIBRARY_VERSION_SUFFIX}")
        set (SUF_CONCAT "")
        foreach (SUF_PART IN LISTS SUF_PARTS)
          if (NOT "x${SUF_PART}" STREQUAL "x")
            set (SUF_CONCAT "${SUF_CONCAT}.${SUF_PART}")
          endif()
          if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
            install (FILES "${3RDPARTY_${3RDPARTY_NAME}_LIBRARY_DIR}/lib${LIB}${LIBRARY_NAME_DEBUG_SUFFIX}.so${SUF_CONCAT}" DESTINATION bind OPTIONAL)
          endif()
          if ( (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug") OR IS_USE_RELEASE_LIB )
            install (FILES "${3RDPARTY_${3RDPARTY_NAME}_LIBRARY_DIR}/lib${LIB}.so${SUF_CONCAT}" DESTINATION bin OPTIONAL)
          endif()
        endforeach()
      else()
        if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
          install (FILES "${3RDPARTY_${3RDPARTY_NAME}_LIBRARY_DIR}/lib${LIB}${LIBRARY_NAME_DEBUG_SUFFIX}.so" DESTINATION bind)
        endif()
        if ( (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug") OR IS_USE_RELEASE_LIB )
          install (FILES "${3RDPARTY_${3RDPARTY_NAME}_LIBRARY_DIR}/lib${LIB}.so" DESTINATION bin)
        endif()
      endif()
    endforeach()
  endif()
endmacro()
