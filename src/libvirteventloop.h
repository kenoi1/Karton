// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <QTimer>
#include <QHash>
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include <functional>

class LibvirtEventLoop : public QObject {
    Q_OBJECT
    
private:
    static QHash<int, QSocketNotifier*> readNotifiers;
    static QHash<int, QSocketNotifier*> writeNotifiers;
    static QHash<int, QSocketNotifier*> exceptionNotifiers;
    static QHash<int, QTimer*> timers;
    static QHash<int, std::function<void()>> timerCallbacks;
    
public:
    static int addHandle(int fd, int events, virEventHandleCallback cb, void *opaque, virFreeCallback ff);
     
    
    static void updateHandle(int watch, int events);
    
    static int removeHandle(int watch);
    
    static int addTimeout(int milliseconds, virEventTimeoutCallback cb, void *opaque, virFreeCallback ff);

    static void updateTimeout(int timer, int milliseconds);
    
    static int removeTimeout(int timer);
    
    static void registerQtEventLoop();
};