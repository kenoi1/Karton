// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "libvirteventloop.h"
#include <iostream>
#include <QDebug>
#include <libvirt/libvirt.h>

LibvirtEventLoop::LibvirtEventLoop(QObject *parent)
    : QThread(parent)
{
}
void LibvirtEventLoop::run()
{
    qDebug() << "starting event loop";

    if (virInitialize() == 0) {
        int registered = virEventRegisterDefaultImpl();
        Q_EMIT result(registered == 0);
        // TODO: run in a qTimer infinite loop
        // virEventRunDefaultImpl();

    } else {
        Q_EMIT result(false);
    }
}
