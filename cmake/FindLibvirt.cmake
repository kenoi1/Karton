# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBVIRT REQUIRED libvirt IMPORTED_TARGET)

if(TARGET PkgConfig::LIBVIRT AND NOT TARGET Libvirt::Libvirt)
  add_library(Libvirt::Libvirt ALIAS PkgConfig::LIBVIRT)
endif()

mark_as_advanced(LIBVIRT_INCLUDE_DIR LIBVIRT_LIBRARY)
