// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domainviewer.h"

#include <spice-client.h>

#include <QPainter>
#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QString>
#include <QTimer>

#include "domain.h"
#include "glib.h"
#include "karton_debug.h"

DomainViewer::DomainViewer(QQuickItem *parent)
    : QQuickItem(parent)
    , m_domain(nullptr)
    , m_host(QStringLiteral("localhost"))
    , // Default host
    m_port(5900)
    , m_connected(false)
    , m_frameUpdated(false)
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setFlag(ItemIsFocusScope, true);

    qDebug() << "DomainViewer constructor - default host:" << m_host << "port:" << m_port;
}

DomainViewer::~DomainViewer()
{
    disconnectFromSpice();
}

void DomainViewer::setDomain(Domain *domain)
{
    if (m_domain != domain) {
        if (m_domain) {
            disconnectFromSpice();
        }

        m_domain = domain;
        Q_EMIT domainChanged();

        if (isComponentComplete() && m_domain) {
            if (m_domain) {
                connectToSpice();
            } else {
                qCDebug(KARTON_DEBUG) << "setDomain(): null domain assigned";
            }
        }
    }
}

void DomainViewer::mouseMoveEvent(QMouseEvent *event)
{ // todo
    static int moveCounter = 0;
    if (++moveCounter % 5 == 0) {
        qCInfo(KARTON_DEBUG) << "Mouse Drag: at (" << event->position().x() << "," << event->position().y() << ")";
    }
    event->accept();
}

void DomainViewer::hoverMoveEvent(QHoverEvent *event)
{
    static int hoverCounter = 0;
    if (++hoverCounter % 20 == 0) {
        qCInfo(KARTON_DEBUG) << "Mouse hover at (" << event->position().x() << "," << event->position().y() << ")";
    }
    // send to spice
    if (m_inputs_channel && m_connected) {
        int x = event->position().x();
        int y = event->position().y();

        if (m_imageWidth > 0 && m_imageHeight > 0 && width() > 0 && height() > 0) {
            x = (x * m_imageWidth) / width();
            y = (y * m_imageHeight) / height();
        }

        spice_inputs_position(m_inputs_channel, x, y, 0, 0);
    }
}

void DomainViewer::mousePressEvent(QMouseEvent *event)
{ // todo
    qCInfo(KARTON_DEBUG) << "Mouse click at (" << event->position().x() << "," << event->position().y() << ") button:" << event->button();
    setFocus(true);
    int button = 0;
    switch (event->button()) {
    case Qt::LeftButton:
        button = SPICE_MOUSE_BUTTON_LEFT;
        break;
    case Qt::RightButton:
        button = SPICE_MOUSE_BUTTON_RIGHT;
        break;
    case Qt::MiddleButton:
        button = SPICE_MOUSE_BUTTON_MIDDLE;
        break;
    default:
        return;
    }

    int button_mask = 0;
    if (event->buttons() & Qt::LeftButton)
        button_mask |= SPICE_MOUSE_BUTTON_MASK_LEFT;
    if (event->buttons() & Qt::MiddleButton)
        button_mask |= SPICE_MOUSE_BUTTON_MASK_MIDDLE;
    if (event->buttons() & Qt::RightButton)
        button_mask |= SPICE_MOUSE_BUTTON_MASK_RIGHT;

    spice_inputs_button_press(m_inputs_channel, button, button_mask);
    event->accept();
}

void DomainViewer::componentComplete()
{
    qCCritical(KARTON_DEBUG) << "run?!";
    QQuickItem::componentComplete();
    if (m_domain) {
        connectToSpice();
    }
}

QSGNode *DomainViewer::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QMutexLocker locker(&m_frameLock);
    // qCInfo(KARTON_DEBUG) << "updatePaintNode received frame: size proport." << m_frame.size() << ", is this null??:" << m_frame.isNull();

    // checkChannelStatus();

    // prevent render if not updated or valid
    if (!m_frameUpdated || m_frame.isNull() || m_frame.width() <= 0 || m_frame.height() <= 0) {
        delete oldNode;
        return nullptr;
    }

    QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);

    if (!node) {
        node = new QSGSimpleTextureNode();
        node->setOwnsTexture(true);
    }

    QSGTexture *texture = window()->createTextureFromImage(m_frame);
    if (texture) {
        node->setTexture(texture);
        node->setRect(boundingRect());
        m_frameUpdated = false;
    }

    return node;
}

bool DomainViewer::connectToSpice()
{
    qCCritical(KARTON_DEBUG) << "Running connecte to spice! " << m_host << ", Port is:" << m_port;
    if (!m_domain) {
        qCCritical(KARTON_DEBUG) << "connectToSpice() called but domain is null!";
        return false;
    }

    disconnectFromSpice();

    m_session = spice_session_new();

    QString uri = QString::fromUtf8("spice://%1:%2").arg(m_host).arg(m_port);
    g_object_set(m_session, "uri", uri.toUtf8().constData(), NULL);
    // could use SpiceURI directly also

    g_signal_connect(m_session, "channel-new", G_CALLBACK(DomainViewer::channel_new_cb), this);

    if (!spice_session_connect(m_session)) {
        g_object_unref(m_session);
        m_session = nullptr;
        return false;
    }
    qCInfo(KARTON_DEBUG) << "yay! connected to " << domain()->config()->name();
    m_connected = true;

    return true;
}

void DomainViewer::disconnectFromSpice()
{
    if (m_session) {
        spice_session_disconnect(m_session);
        g_object_unref(m_session);
        m_session = nullptr;
        m_display_channel = nullptr;
        m_connected = false;
    }
}

void DomainViewer::channel_new_cb(SpiceSession *session, SpiceChannel *channel, gpointer user_data)
{
    DomainViewer *item = static_cast<DomainViewer *>(user_data);

    // checkChannelStatus(); // debug msgs.
    if (SPICE_IS_DISPLAY_CHANNEL(channel)) {
        qCInfo(KARTON_DEBUG) << "SPICE display connected";

        spice_channel_connect(channel);
        item->m_display_channel = channel;

        g_signal_connect(channel, "display-primary-create", G_CALLBACK(display_primary_create_callback), item);
        g_signal_connect(channel, "display-invalidate", G_CALLBACK(display_invalidate_callback), item);
    } else if (SPICE_IS_INPUTS_CHANNEL(channel)) {
        qCInfo(KARTON_DEBUG) << "SPICE: Inputs connected";
        spice_channel_connect(channel);
        item->m_inputs_channel = SPICE_INPUTS_CHANNEL(channel);
    }
}
void DomainViewer::display_primary_create_callback(SpiceChannel *channel,
                                                   gint format,
                                                   gint width,
                                                   gint height,
                                                   gint stride,
                                                   gint shmid,
                                                   gpointer imgdata,
                                                   gpointer user_data)
{
    DomainViewer *item = static_cast<DomainViewer *>(user_data);
    qCInfo(KARTON_DEBUG) << "SPICE: primary framebuffer received! size:" << width << "x" << height;
    qCInfo(KARTON_DEBUG) << "SPICE: format is:" << format;
    QMutexLocker locker(&item->m_frameLock);

    // item->m_frameBuffer = QImage((uchar *)imgdata, width, height, stride, QImage::Format_RGB32); // TRANSPARENT
    // item->m_frameBuffer = item->processSpiceImage(format, width, height, stride, imgdata); // GIVES BLACK IMAGE
    // item->m_frameBuffer = QImage(QStringLiteral("/home/dereklin/Pictures/picture_2025-05-17_23-14-40.jpg")).convertToFormat(QImage::Format_RGB32); // WORKS
    // GOOD

    item->m_frameBuffer = static_cast<uchar *>(imgdata);
    item->m_imageWidth = width;
    item->m_imageHeight = height;
    item->m_frame = QImage(width, height, QImage::Format_RGB32);
    // item->m_frameBuffer = QImage((uchar *)imgdata, width, height, stride, QImage::Format_RGB888); // Black/white but good opacity, weird render because 24b
    // item->m_frameBuffer = QImage((uchar *)imgdata, width, height, stride, QImage::Format_RGBX8888);
    // item->m_frameBuffer = QImage((uchar *)imgdata, width, height, stride, QImage::Format_ARGB32_Premultiplied); // good color, weird brightness
    // item->m_frameBuffer = QImage((uchar *)imgdata, width, height, stride, QImage::Format_BGR888).copy();

    item->m_frameUpdated = true;
    QMetaObject::invokeMethod(item, "frameUpdated", Qt::QueuedConnection); // could also do queued
    QMetaObject::invokeMethod(item, "update", Qt::QueuedConnection);
}

void DomainViewer::display_invalidate_callback(SpiceDisplayChannel *channel, gint x, gint y, gint width, gint height, gpointer user_data)
{
    DomainViewer *item = static_cast<DomainViewer *>(user_data);
    item->m_frameUpdated = true;

    // Copy from spice-glib framebuffer to the QImage to render
    uint *source = reinterpret_cast<uint *>(item->m_frameBuffer);
    for (int i = y; i < y + height; ++i) {
        for (int j = x; j < x + width; ++j) {
            item->m_frame.setPixel(j, i, source[item->m_imageWidth * i + j]);
        }
    }

    QMetaObject::invokeMethod(item, "update", Qt::QueuedConnection);
}

void DomainViewer::checkChannelStatus()
{
    if (!m_session) {
        qCInfo(KARTON_DEBUG) << "Channel check: No active session";
        return;
    }
    if (!SPICE_IS_SESSION(m_session)) {
        qCWarning(KARTON_DEBUG) << "Session pointer is invalid!";
        m_session = nullptr;
        return;
    }

    GList *channels = spice_session_get_channels(m_session);
    qCInfo(KARTON_DEBUG) << "Channel check: Found" << (channels ? g_list_length(channels) : 0) << "channels";

    bool hasDisplayChannel = false;
    for (GList *iter = channels; iter; iter = iter->next) {
        SpiceChannel *channel = SPICE_CHANNEL(iter->data);
        gint type;
        g_object_get(channel, "channel-type", &type, NULL);
        const gchar *type_str = spice_channel_type_to_string(type);

        qCInfo(KARTON_DEBUG) << "  - Channel type:" << QString::fromUtf8(type_str);

        if (SPICE_IS_DISPLAY_CHANNEL(channel)) {
            hasDisplayChannel = true;
            qCInfo(KARTON_DEBUG) << "    Found display channel";
            gint channel_id;
            g_object_get(channel, "channel-id", &channel_id, NULL);
            qCInfo(KARTON_DEBUG) << "    Display channel ID:" << channel_id;
        }
    }

    if (!hasDisplayChannel) {
        qCWarning(KARTON_DEBUG) << "No display channel found after" << (m_connected ? "successful connection!" : "failed connection");
    }
}
