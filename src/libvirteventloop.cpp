// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "libvirteventloop.h"

#include <libvirt/libvirt.h>

#include <QTimer>
#include <iostream>

#include "karton_debug.h"

LibvirtEventLoop::LibvirtEventLoop(QObject *parent)
    : QObject(parent)
{
}
void LibvirtEventLoop::run()
{
    qCInfo(KARTON_DEBUG) << "Starting event loop";

    // TODO: In the future, implement a proper virEventRegisterImpl
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, []() {
        virEventRunDefaultImpl();
    });

    timer->setInterval(1000);
    timer->setSingleShot(false);

    timer->start();
    Q_EMIT result(true);
}
