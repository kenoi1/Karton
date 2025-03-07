# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

cmake_minimum_required(VERSION 3.16)

project(karton VERSION 0.1 LANGUAGES CXX)

set(QT_MIN_VERSOIN "6.2.0")
set(KF6_DEP_VERSION "6.0.0")
set(QT_MAJOR_VERSION "6")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(ECM ${KF6_DEP_VERSION} QUIET REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(FeatureSummary)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(ECMFindQmlModule)
include(KDECMakeSettings)
include(ECMAddTests)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(KDEGitCommitHooks)
include(ECMCheckOutboundLicense)
include(ECMQtDeclareLoggingCategory)
include(ECMAddAndroidApk)
include(KDEClangFormat)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Quick QuickControls2 Widgets)
find_package(KF6 ${KF6_DEP_VERSION} COMPONENTS Config CoreAddons Kirigami)

qt_add_executable(karton
    main.cpp
)

qt_add_qml_module(karton
    URI karton
    VERSION 1.0
    QML_FILES
      Main.qml
      VmConfigDialog.qml
    RESOURCES
      resources.qrc
    SOURCES
      backendengine.h
      backendengine.cpp
      kartonmachine.h
      kartonmachine.cpp
      kartonerror.h
      kartonerror.cpp
      configlists.h
      configlists.cpp
)

target_link_libraries(karton
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE Qt6::Widgets
    PRIVATE KF6::ConfigCore
    PRIVATE KF6::Kirigami
)

include(GNUInstallDirs)
install(TARGETS karton
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES
	img/karton-archlinux.svg
	img/karton-debian.svg
	img/karton-devuan.svg
	img/karton-fedora.svg
	img/karton-freedos.svg
	img/karton-haiku.svg
	img/karton-kaos.svg
	img/karton_logo.png
	img/karton-nixos.svg
	img/karton-openindiana.svg
	img/karton-opensuse.svg
	img/karton-other.svg
	img/karton-rhel.svg
	img/karton-ubuntu.svg
	img/karton-windows.svg
	DESTINATION /usr/share/icons/hicolor/scalable/apps
)
install(FILES
	karton.desktop
	DESTINATION /usr/share/applications
)
