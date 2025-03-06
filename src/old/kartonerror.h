// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

#ifndef KARTONERROR_H
#define KARTONERROR_H

#include <QObject>
#include <QQmlEngine>

class KartonError : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum VmError {
        // Good
        None,

        // Failed start
        LaunchFailed,
        LaunchNoDisk,
        LaunchNoCd,
        LaunchCdMissing,

        // Failed delete
        DeleteFailed,
        DeleteRunningVm
    };
    Q_ENUMS(VmError)
    explicit KartonError(QObject *parent = nullptr);
};

#endif // KARTONERROR_H
