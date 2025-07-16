// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "commandrunner.h"
#include "karton_debug.h"

#include <QProcess>

bool CommandRunner::runCommand(const QString &command)
{
    // FIXME make this async to not block the gui thread
    qCDebug(KARTON_DEBUG) << "Running Command:" << command;
    auto process = new QProcess(this);
    connect(process, &QProcess::finished, [this, process](int exitCode, QProcess::ExitStatus) {
        QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
        Q_EMIT commandFinished(exitCode, output);
    });
    process->startCommand(command);

    return process->waitForStarted();
}
