// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

class CommandRunner : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    bool runCommand(const QString &command);

Q_SIGNALS:
    void commandFinished(int exitCode, const QString &output);
};