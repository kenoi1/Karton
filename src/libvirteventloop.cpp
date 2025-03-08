// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "libvirteventloop.h"
#include <QHash>
#include <QObject>
#include <QTimer>
#include <QSocketNotifier>

QHash<int, QSocketNotifier*> LibvirtEventLoop::readNotifiers;
QHash<int, QSocketNotifier*> LibvirtEventLoop::writeNotifiers;
QHash<int, QSocketNotifier*> LibvirtEventLoop::exceptionNotifiers;
QHash<int, QTimer*> LibvirtEventLoop::timers;
QHash<int, std::function<void()>> LibvirtEventLoop::timerCallbacks;

int LibvirtEventLoop::addHandle(int fd, int events, virEventHandleCallback cb, void *opaque, virFreeCallback ff) {
    int watch = fd;
    
    if (events & VIR_EVENT_HANDLE_READABLE) {
        QSocketNotifier *readNotifier = new QSocketNotifier(fd, QSocketNotifier::Read);
        readNotifier->setEnabled(true);
        QObject::connect(readNotifier, &QSocketNotifier::activated,
                        [=](int socket) { cb(watch, socket, events, opaque); });
        readNotifiers[watch] = readNotifier;
    }
    
    if (events & VIR_EVENT_HANDLE_WRITABLE) {
        QSocketNotifier *writeNotifier = new QSocketNotifier(fd, QSocketNotifier::Write);
        writeNotifier->setEnabled(true);
        QObject::connect(writeNotifier, &QSocketNotifier::activated,
                        [=](int socket) { cb(watch, socket, events, opaque); });
        writeNotifiers[watch] = writeNotifier;
    }
    
    return watch;
}
void LibvirtEventLoop::updateHandle(int watch, int events) {
        if (readNotifiers.contains(watch)) {
            readNotifiers[watch]->setEnabled(events & VIR_EVENT_HANDLE_READABLE);
        }
        
        if (writeNotifiers.contains(watch)) {
            writeNotifiers[watch]->setEnabled(events & VIR_EVENT_HANDLE_WRITABLE);
        }
}
int LibvirtEventLoop::removeHandle(int watch) {
        if (readNotifiers.contains(watch)) {
            delete readNotifiers[watch];
            readNotifiers.remove(watch);
        }
        
        if (writeNotifiers.contains(watch)) {
            delete writeNotifiers[watch];
            writeNotifiers.remove(watch);
        }
        
        return 0;
}
int LibvirtEventLoop::addTimeout(int milliseconds, virEventTimeoutCallback cb, void *opaque, virFreeCallback ff) {
        // Create a timer ID
        static int timerCount = 0;
        int timer = ++timerCount;
        
        // Create a timer and save the callback
        QTimer *qTimer = new QTimer();
        qTimer->setSingleShot(true);
        
        // Store the callback for later use
        timerCallbacks[timer] = [=]() { cb(timer, opaque); };
        
        // Connect timer timeout to the callback
        QObject::connect(qTimer, &QTimer::timeout, timerCallbacks[timer]);
        
        // Start the timer
        qTimer->start(milliseconds);
        timers[timer] = qTimer;
        
        return timer;
    }
    
void LibvirtEventLoop::updateTimeout(int timer, int milliseconds) {
    if (timers.contains(timer)) {
        timers[timer]->start(milliseconds);
    }
}
int LibvirtEventLoop::removeTimeout(int timer) {
    if (timers.contains(timer)) {
        delete timers[timer];
        timers.remove(timer);
        timerCallbacks.remove(timer);
    }
    return 0;
}
void LibvirtEventLoop::registerQtEventLoop() {
        virEventRegisterImpl(
            addHandle,
            updateHandle,
            removeHandle,
            addTimeout,
            updateTimeout,
            removeTimeout
        );
}
