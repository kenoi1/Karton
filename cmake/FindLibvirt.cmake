# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_LIBVIRT QUIET libvirt)
endif()

find_path(LIBVIRT_INCLUDE_DIR
  NAMES libvirt/libvirt.h
  PATHS ${PC_LIBVIRT_INCLUDE_DIRS}
  PATH_SUFFIXES libvirt
)

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