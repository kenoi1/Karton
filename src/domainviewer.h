// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

// #include <spice-client.h>

#pragma once

#include <spice-client.h>

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QQuickItem>
#include <QSGNode>
#include <QSGTexture>

#include "domain.h"

class DomainViewer : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(Domain *domain READ domain WRITE setDomain NOTIFY domainChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    static void channel_new_cb(SpiceSession *session, SpiceChannel *channel, gpointer user_data);

public:
    DomainViewer(QQuickItem *parent = nullptr);
    ~DomainViewer();

    Domain *domain() const
    {
        return m_domain;
    }
    void setDomain(Domain *domain);

    void componentComplete() override;

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;
    bool connectToSpice();
    void disconnectFromSpice();
    static void
    display_primary_create_callback(SpiceChannel *channel, gint format, gint width, gint height, gint stride, gint shmid, gpointer imgdata, gpointer user_data);
    static void display_invalidate_callback(SpiceDisplayChannel *channel, gint x, gint y, gint width, gint height, gpointer user_data);

    void updateTexture();

    void checkChannelStatus();

    void handleMouseEvent(QMouseEvent *event);
    void handleKeyEvent(QKeyEvent *event);

    QString host() const
    {
        return m_host;
    }
    void setHost(const QString &host)
    {
        if (m_host != host) {
            m_host = host;
            Q_EMIT hostChanged();
        }
    }

    int port() const
    {
        return m_port;
    }
    void setPort(int port)
    {
        if (m_port != port) {
            m_port = port;
            Q_EMIT portChanged();
        }
    }

Q_SIGNALS:
    void domainChanged();
    void frameUpdated();
    void connectionChanged(bool connected);

    void portChanged();
    void hostChanged();

private:
    QColor m_color;
    Domain *m_domain;
    bool m_connected = false;

    QImage m_frameBuffer;
    QMutex m_frameLock;
    bool m_frameUpdated = false;

    SpiceSession *m_session = nullptr;
    SpiceChannel *m_display_channel = nullptr;
    QString m_host;
    int m_port = 0;
    QString m_password;
};