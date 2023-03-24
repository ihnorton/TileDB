##############################################################################
# Compute derived vars
##############################################################################

if (DEFINED CMAKE_OSX_ARCHITECTURES)
  list(LENGTH CMAKE_OSX_ARCHITECTURES _length)
  if (_length GREATER 1)
    set(COMPUTED_NAME "${CMAKE_SYSTEM_NAME}-Universal")
  else()
  set(COMPUTED_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
  endif()
else()
  set(COMPUTED_NAME "${CMAKE_SYSTEM_NAME}")
endif()


##############################################################################
# Set vars
##############################################################################

# - CPACK_PACKAGE_VERSION is set in project() call to TILEDB_VERSION
set(CPACK_PACKAGE_VENDOR TileDB)
set(CPACK_SYSTEM_NAME "${COMPUTED_NAME}")
