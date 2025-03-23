# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>
# SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

cmake_minimum_required(VERSION 3.20)
project(karton)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(ECM 6.0.0 REQUIRED NO_MODULE)
# set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMFindQmlModule)
include(ECMAddAppIcon)
include(ECMQmlModule)
include(ECMQtDeclareLoggingCategory)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Quick
    Test
    Gui
    QuickControls2
    Widgets
)

find_package(Libvirt REQUIRED)

find_package(KF6 REQUIRED COMPONENTS
    Kirigami
    I18n
    CoreAddons
    QQC2DesktopStyle
    IconThemes
)
install(FILES
    img/karton_logo.png
    DESTINATION /usr/share/icons/hicolor/scalable/apps
)
install(FILES
	org.kde.karton.desktop
	DESTINATION /usr/share/applications
)


ecm_find_qmlmodule(org.kde.kirigami REQUIRED)

add_subdirectory(src)

install(PROGRAMS org.kde.karton.desktop DESTINATION ${KDE_INSTALL_APPDIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)