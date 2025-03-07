add_executable(karton)

ecm_add_qml_module(karton
    URI
    org.kde.karton
)

target_sources(karton
    PRIVATE
    main.cpp
    karton.cpp
    vmlistmodel.cpp
)

ecm_target_qml_sources(karton
    SOURCES
    qml/Main.qml
    qml/VMListPage.qml
)

target_link_libraries(karton
    PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::QuickControls2
    Qt6::Widgets
    KF6::I18n
    KF6::CoreAddons
    KF6::IconThemes
    Libvirt::Libvirt
)

install(TARGETS karton ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
