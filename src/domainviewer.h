// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#pragma once

#include <linux/input-event-codes.h>
#include <spice-client.h>

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QQuickItem>
#include <QSGNode>
#include <QSGTexture>

#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>

#include "commandrunner.h"
#include "domain.h"

class DomainViewer : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(Domain *domain READ domain WRITE setDomain NOTIFY domainChanged REQUIRED)
    Q_PROPERTY(QString host MEMBER m_host NOTIFY hostChanged)
    Q_PROPERTY(int port MEMBER m_port NOTIFY portChanged)

public:
    explicit DomainViewer(QQuickItem *parent = nullptr);
    ~DomainViewer();

    Domain *domain() const
    {
        return m_domain;
    }
    void setDomain(Domain *domain);

    void componentComplete() override;

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;
    bool setupSpiceSession();
    bool connectToSpice();
    void disconnectFromSpice();

    void stopAudio();

    void updateTexture();

    void checkChannelStatus();

    void mouseMoveEvent(QMouseEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

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

private Q_SLOTS:
    void handleHostPort(int exitCode, const QString &output);

private:
    static void channel_new_cb(SpiceSession *session, SpiceChannel *channel, gpointer user_data);
    static void
    display_primary_create_callback(SpiceChannel *channel, gint format, gint width, gint height, gint stride, gint shmid, gpointer imgdata, gpointer user_data);
    static void display_invalidate_callback(SpiceDisplayChannel *channel, gint x, gint y, gint width, gint height, gpointer user_data);
    static uint8_t evdevToPcXt(uint32_t evdev_scancode);

    static void playback_start_callback(SpicePlaybackChannel *channel, gint format, gint channels, gint rate, gpointer user_data);
    static void playback_data_callback(SpicePlaybackChannel *channel, gpointer data, gint size, gpointer user_data);
    static void playback_stop_callback(SpicePlaybackChannel *channel, gpointer user_data);

    CommandRunner *m_commandRunner;
    QColor m_color;
    Domain *m_domain;
    QString m_host;
    int m_port = 0;
    QString m_password;
    bool m_connected = false;

    int m_imageWidth;
    int m_imageHeight;
    QImage m_frame;
    uchar *m_frameBuffer = nullptr;
    QMutex m_frameLock;
    bool m_frameUpdated = false;

    SpiceSession *m_session = nullptr;
    SpiceChannel *m_display_channel = nullptr;
    SpiceInputsChannel *m_inputs_channel = nullptr;

    int m_current_button_mask = 0;

    SpiceAudio *m_audio;
    SpicePlaybackChannel *m_playback_channel;
    QAudioSink *m_audioSink;
    QIODevice *m_audioDevice;
    QAudioFormat m_audioFormat;

    static constexpr quint32 x11_wayland_evdev_offset = 8;
    // difference of 8 between x11 wayland.
    // see: https://wayland-devel.freedesktop.narkive.com/6dOtsFGc/gtk-hardware-scancodes-for-wayland-detecting-xwayland
};