// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include <QThread>

class LibvirtEventLoop : public QThread
{
    Q_OBJECT

public:
    explicit LibvirtEventLoop(QObject *parent = nullptr);
    void run();

Q_SIGNALS:
    void result(bool);
};
