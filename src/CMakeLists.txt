# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>
# SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

add_executable(karton)

ecm_add_qml_module(karton
    GENERATE_PLUGIN_SOURCE
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/karton
    URI org.kde.karton
)

target_sources(karton
    PRIVATE
    main.cpp
    karton.cpp
    vmlistmodel.cpp
    domain.cpp
    domainconfig.cpp
    libvirtmonitor.cpp
    libvirteventloop.cpp
    domainviewer.cpp
    domaininstaller.cpp
    osinfoconfig.cpp
    domainxmlreader.cpp
    commandrunner.cpp
    evdev_to_xtkbd_map.c
    evdev_to_xtkbd_map.h
)

ecm_target_qml_sources(karton
    SOURCES
    qml/Main.qml
    qml/VMListPage.qml
    qml/VMViewerWindow.qml
)

ecm_qt_declare_logging_category(
    karton
    HEADER karton_debug.h
    IDENTIFIER KARTON_DEBUG
    CATEGORY_NAME org.kde.karton
    DESCRIPTION "Karton Virtual Machine Manager"
    EXPORT Karton
)

ecm_qt_install_logging_categories(
    EXPORT Karton
    FILE karton.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

target_link_libraries(karton
    PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Xml
    Qt6::Multimedia
    KF6::I18n
    KF6::CoreAddons
    KF6::IconThemes
    Libvirt::Libvirt
    Spiceglib::Spiceglib
    Libosinfo::Libosinfo
    Glib::Glib
)

install(TARGETS karton ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
