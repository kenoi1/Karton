# FindLibvirt.cmake
# Finds the libvirt library
#
# This will define the following variables:
#   LIBVIRT_FOUND        - True if libvirt is found
#   LIBVIRT_INCLUDE_DIRS - The libvirt include directories
#   LIBVIRT_LIBRARIES    - The libvirt libraries
#
# and the following imported targets:
#   Libvirt::Libvirt

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_LIBVIRT QUIET libvirt)
endif()

# Find the include directory
find_path(LIBVIRT_INCLUDE_DIR
  NAMES libvirt/libvirt.h
  PATHS ${PC_LIBVIRT_INCLUDE_DIRS}
  PATH_SUFFIXES libvirt
)

# Find the library
find_library(LIBVIRT_LIBRARY
  NAMES virt libvirt
  PATHS ${PC_LIBVIRT_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libvirt
  REQUIRED_VARS LIBVIRT_LIBRARY LIBVIRT_INCLUDE_DIR
  VERSION_VAR PC_LIBVIRT_VERSION
)

if(LIBVIRT_FOUND)
  set(LIBVIRT_LIBRARIES ${LIBVIRT_LIBRARY})
  set(LIBVIRT_INCLUDE_DIRS ${LIBVIRT_INCLUDE_DIR})
  
  if(NOT TARGET Libvirt::Libvirt)
    add_library(Libvirt::Libvirt UNKNOWN IMPORTED)
    set_target_properties(Libvirt::Libvirt PROPERTIES
      IMPORTED_LOCATION "${LIBVIRT_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${LIBVIRT_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(LIBVIRT_INCLUDE_DIR LIBVIRT_LIBRARY)