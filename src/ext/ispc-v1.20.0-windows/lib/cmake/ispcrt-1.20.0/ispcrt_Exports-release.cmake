#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ispcrt::ispcrt" for configuration "Release"
set_property(TARGET ispcrt::ispcrt APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ispcrt::ispcrt PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/ispcrt.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/ispcrt.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS ispcrt::ispcrt )
list(APPEND _IMPORT_CHECK_FILES_FOR_ispcrt::ispcrt "${_IMPORT_PREFIX}/lib/ispcrt.lib" "${_IMPORT_PREFIX}/bin/ispcrt.dll" )

# Import target "ispcrt::ispcrt_static" for configuration "Release"
set_property(TARGET ispcrt::ispcrt_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ispcrt::ispcrt_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX;RC"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/ispcrt_static.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS ispcrt::ispcrt_static )
list(APPEND _IMPORT_CHECK_FILES_FOR_ispcrt::ispcrt_static "${_IMPORT_PREFIX}/lib/ispcrt_static.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
