// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

#include "configlists.h"

ConfigLists::ConfigLists(QObject *parent)
    : QObject{parent}
{
    // TODO: Add more operating systems here.
    m_osList = QStringList({
        QStringLiteral("ArchLinux"),
        QStringLiteral("Debian"),
        QStringLiteral("Devuan"),
        QStringLiteral("Fedora"),
        QStringLiteral("FreeDOS"),
        QStringLiteral("Haiku"),
        QStringLiteral("KaOS"),
        QStringLiteral("NixOS"),
        QStringLiteral("OpenIndiana"),
        QStringLiteral("openSUSE"),
        QStringLiteral("RHEL"),
        QStringLiteral("Ubuntu"),
        QStringLiteral("Windows"),
        QStringLiteral("Other"),
    });

    // TODO: Eventually it would be nice for Libreboot to be supported.
    m_firmwareList = QStringList({
        QStringLiteral("BIOS"),
        QStringLiteral("UEFI"),
    });
}

QStringList ConfigLists::osList() {
    return m_osList;
}
QStringList ConfigLists::firmwareList() {
    return m_firmwareList;
}
