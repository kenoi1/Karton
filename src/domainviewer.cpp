// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domainviewer.h"

#include <spice-client.h>

#include <QGuiApplication>
#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QString>

#include "domain.h"
#include "glib.h"
#include "karton_debug.h"

DomainViewer::DomainViewer(QQuickItem *parent)
    : QQuickItem(parent)
    , m_domain(nullptr)
    , m_host(QStringLiteral("localhost"))
    , m_port(5900)
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
    if (m_domain == domain) {
        return;
    }
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

// maps qt provided scancode to pcxt
uint8_t DomainViewer::evdevToPcXt(uint32_t evdev_scancode)
{
    static const QHash<uint32_t, uint8_t> scancode_map = {
        {KEY_RESERVED, 0x00},   {KEY_ESC, 0x01},        {KEY_1, 0x02},         {KEY_2, 0x03},          {KEY_3, 0x04},          {KEY_4, 0x05},
        {KEY_5, 0x06},          {KEY_6, 0x07},          {KEY_7, 0x08},         {KEY_8, 0x09},          {KEY_9, 0x0A},          {KEY_0, 0x0B},
        {KEY_MINUS, 0x0C},      {KEY_EQUAL, 0x0D},      {KEY_BACKSPACE, 0x0E}, {KEY_TAB, 0x0F},        {KEY_Q, 0x10},          {KEY_W, 0x11},
        {KEY_E, 0x12},          {KEY_R, 0x13},          {KEY_T, 0x14},         {KEY_Y, 0x15},          {KEY_U, 0x16},          {KEY_I, 0x17},
        {KEY_O, 0x18},          {KEY_P, 0x19},          {KEY_LEFTBRACE, 0x1A}, {KEY_RIGHTBRACE, 0x1B}, {KEY_ENTER, 0x1C},      {KEY_LEFTCTRL, 0x1D},
        {KEY_A, 0x1E},          {KEY_S, 0x1F},          {KEY_D, 0x20},         {KEY_F, 0x21},          {KEY_G, 0x22},          {KEY_H, 0x23},
        {KEY_J, 0x24},          {KEY_K, 0x25},          {KEY_L, 0x26},         {KEY_SEMICOLON, 0x27},  {KEY_APOSTROPHE, 0x28}, {KEY_GRAVE, 0x29},
        {KEY_LEFTSHIFT, 0x2A},  {KEY_BACKSLASH, 0x2B},  {KEY_Z, 0x2C},         {KEY_X, 0x2D},          {KEY_C, 0x2E},          {KEY_V, 0x2F},
        {KEY_B, 0x30},          {KEY_N, 0x31},          {KEY_M, 0x32},         {KEY_COMMA, 0x33},      {KEY_DOT, 0x34},        {KEY_SLASH, 0x35},
        {KEY_RIGHTSHIFT, 0x36}, {KEY_KPASTERISK, 0x37}, {KEY_LEFTALT, 0x38},   {KEY_SPACE, 0x39},      {KEY_CAPSLOCK, 0x3A},   {KEY_F1, 0x3B},
        {KEY_F2, 0x3C},         {KEY_F3, 0x3D},         {KEY_F4, 0x3E},        {KEY_F5, 0x3F},         {KEY_F6, 0x40},         {KEY_F7, 0x41},
        {KEY_F8, 0x42},         {KEY_F9, 0x43},         {KEY_F10, 0x44},       {KEY_NUMLOCK, 0x45},    {KEY_SCROLLLOCK, 0x46}, {KEY_KP7, 0x47},
        {KEY_KP8, 0x48},        {KEY_KP9, 0x49},        {KEY_KPMINUS, 0x4A},   {KEY_KP4, 0x4B},        {KEY_KP5, 0x4C},        {KEY_KP6, 0x4D},
        {KEY_KPPLUS, 0x4E},     {KEY_KP1, 0x4F},        {KEY_KP2, 0x50},       {KEY_KP3, 0x51},        {KEY_KP0, 0x52},        {KEY_KPDOT, 0x53},
        {KEY_F11, 0x57},        {KEY_F12, 0x58},        {KEY_UP, 0x48},        {KEY_DOWN, 0x50},       {KEY_LEFT, 0x4B},       {KEY_RIGHT, 0x4D}};

    auto it = scancode_map.find(evdev_scancode);
    if (it != scancode_map.end()) {
        qCDebug(KARTON_DEBUG) << "Mapped evdev" << evdev_scancode << "to PC XT" << QString::number(it.value(), 16);
        return it.value();
    }

    qCWarning(KARTON_DEBUG) << "Unknown evdev scancode:" << evdev_scancode;
    return 0;
}

void DomainViewer::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    quint32 evdev_scancode;
    if (QGuiApplication::platformName() == QStringLiteral("xcb")) { // check if x11
        evdev_scancode = event->nativeScanCode();
    } else { // wayland probably
        evdev_scancode = event->nativeScanCode() - x11_wayland_evdev_offset;
    }

    uint8_t pcxt_scancode = DomainViewer::evdevToPcXt(evdev_scancode); // spice accepts PC XT: see inputs channel docs
    qCDebug(KARTON_DEBUG) << "key press: " << event->text() << evdev_scancode << pcxt_scancode;

    if (m_inputs_channel && m_connected && pcxt_scancode != 0) {
        spice_inputs_channel_key_press(m_inputs_channel, pcxt_scancode);
    }
}

void DomainViewer::keyReleaseEvent(QKeyEvent *event)
{
    event->accept();

    quint32 evdev_scancode;
    if (QGuiApplication::platformName() == QStringLiteral("xcb")) {
        evdev_scancode = event->nativeScanCode();
    } else {
        evdev_scancode = event->nativeScanCode() - x11_wayland_evdev_offset;
    }

    uint8_t pcxt_scancode = DomainViewer::evdevToPcXt(evdev_scancode);

    if (m_inputs_channel && m_connected && pcxt_scancode != 0) {
        spice_inputs_channel_key_release(m_inputs_channel, pcxt_scancode);
    }
}

void DomainViewer::wheelEvent(QWheelEvent *event)
{
    event->accept();

    if (!m_inputs_channel || !m_connected) {
        return;
    }
    int x = event->position().x();
    int y = event->position().y();

    if (m_imageWidth > 0 && m_imageHeight > 0 && width() > 0 && height() > 0) {
        x = (x * m_imageWidth) / width();
        y = (y * m_imageHeight) / height();
    }

    spice_inputs_channel_position(m_inputs_channel, x, y, 0, 0);

    QPoint angleDelta = event->angleDelta();
    // for more info on constants see: (Inputs channel definition) https://www.spice-space.org/spice-protocol.html
    if (angleDelta.y() > 0) { // scroll up
        spice_inputs_channel_button_press(m_inputs_channel, SPICE_MOUSE_BUTTON_UP, 0);
        spice_inputs_channel_button_release(m_inputs_channel, SPICE_MOUSE_BUTTON_UP, 0);
    } else if (angleDelta.y() < 0) { // scroll down
        spice_inputs_channel_button_press(m_inputs_channel, SPICE_MOUSE_BUTTON_DOWN, 0);
        spice_inputs_channel_button_release(m_inputs_channel, SPICE_MOUSE_BUTTON_DOWN, 0);
    }

    if (angleDelta.x() > 0) { // scroll right
        spice_inputs_channel_button_press(m_inputs_channel, SPICE_MOUSE_BUTTON_RIGHT, 0);
        spice_inputs_channel_button_release(m_inputs_channel, SPICE_MOUSE_BUTTON_RIGHT, 0);
    } else if (angleDelta.x() < 0) { // scroll left
        spice_inputs_channel_button_press(m_inputs_channel, SPICE_MOUSE_BUTTON_LEFT, 0);
        spice_inputs_channel_button_release(m_inputs_channel, SPICE_MOUSE_BUTTON_LEFT, 0);
    }

    qCDebug(KARTON_DEBUG) << "wheel event at (" << x << "," << y << ") delta:" << angleDelta;
}
void DomainViewer::mouseMoveEvent(QMouseEvent *event)
{ // todo send spice
    event->accept();
    static int moveCounter = 0;
    if (++moveCounter % 5 == 0) {
        qCInfo(KARTON_DEBUG) << "Mouse Drag: at (" << event->position().x() << "," << event->position().y() << ")";
    }
}

void DomainViewer::hoverMoveEvent(QHoverEvent *event)
{
    static int hoverCounter = 0;
    if (++hoverCounter % 20 == 0) {
        qCInfo(KARTON_DEBUG) << "Mouse hover at (" << event->position().x() << "," << event->position().y() << ")";
    }
    if (m_inputs_channel && m_connected) {
        int x = event->position().x();
        int y = event->position().y();

        if (m_imageWidth > 0 && m_imageHeight > 0 && width() > 0 && height() > 0) {
            x = (x * m_imageWidth) / width();
            y = (y * m_imageHeight) / height();
        }

        spice_inputs_channel_position(m_inputs_channel, x, y, 0, 0);
    }
}

void DomainViewer::mousePressEvent(QMouseEvent *event)
{
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
        qCWarning(KARTON_DEBUG) << "mousepressevent: Unknown button click";
        return;
    }

    int button_mask = 0;
    if (event->buttons() & Qt::LeftButton)
        button_mask |= SPICE_MOUSE_BUTTON_MASK_LEFT;
    if (event->buttons() & Qt::MiddleButton)
        button_mask |= SPICE_MOUSE_BUTTON_MASK_MIDDLE;
    if (event->buttons() & Qt::RightButton)
        button_mask |= SPICE_MOUSE_BUTTON_MASK_RIGHT;

    spice_inputs_channel_button_press(m_inputs_channel, button, button_mask);
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
    qCInfo(KARTON_DEBUG) << "updatePaintNode received frame: size proport." << m_frame.size() << ", is this null??:" << m_frame.isNull();

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

    item->m_frameBuffer = static_cast<uchar *>(imgdata);
    item->m_imageWidth = width;
    item->m_imageHeight = height;
    item->m_frame = QImage(width, height, QImage::Format_RGB32);

    item->m_frameUpdated = true;
    QMetaObject::invokeMethod(item, "frameUpdated", Qt::QueuedConnection); // could also do queued
    QMetaObject::invokeMethod(item, "update", Qt::QueuedConnection);
}

void DomainViewer::display_invalidate_callback(SpiceDisplayChannel *channel, gint x, gint y, gint width, gint height, gpointer user_data)
{
    DomainViewer *item = static_cast<DomainViewer *>(user_data);
    item->m_frameUpdated = true;

    // Copy from spice-glib framebuffer to the QImage to render - inefficient, might want to switch to another approach (partial render?)
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
