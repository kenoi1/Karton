// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domainviewer.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSGSimpleTextureNode>

#include <rhi/qrhi.h> // need
#include <spice-client.h>

#include <QGuiApplication>
#include <QQuickWindow>
#include <QString>
#include <QUrl>

#include "domain.h"
#include "glib.h"
#include "karton_debug.h"

#include "evdev_to_xtkbd_map.h"

DomainViewer::DomainViewer(QQuickItem *parent)
    : QQuickItem(parent)
    , m_commandRunner(new CommandRunner(this))
    , m_domain(nullptr)
    , m_spiceUri(QString())
    , m_host(QStringLiteral("localhost"))
    , m_port(5900)
    , m_password(QString())
    , m_connected(false)
    , m_imageWidth(0)
    , m_imageHeight(0)
    , m_scanout({})
    , m_hasScanout(false)
    , m_eglImage(EGL_NO_IMAGE_KHR)
    , m_texId(0)
    , m_session(nullptr)
    , m_display_channel(nullptr)
    , m_inputs_channel(nullptr)
    , m_playback_channel(nullptr)
    , m_current_button_mask(0)
    , m_audio(nullptr)
    , m_audioSink(nullptr)
    , m_audioDevice(nullptr)
    , m_audioFormat()
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setFlag(ItemIsFocusScope, true);

    connect(m_commandRunner, &CommandRunner::commandFinished, this, &DomainViewer::handleHostPort);
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
            setupSpiceSession();
        } else {
            qCDebug(KARTON_DEBUG) << "setDomain(): null domain assigned";
        }
    }
}

void DomainViewer::componentComplete()
{
    QQuickItem::componentComplete();
    if (m_domain) {
        setupSpiceSession();
    }
}

void DomainViewer::handleHostPort(int exitCode, const QString &output)
{
    if (exitCode != 0 || output.isEmpty()) {
        qCCritical(KARTON_DEBUG) << "handleHostPort: virsh domdisplay call failed";
        return;
    }

    QString trimmedOutput = output.trimmed();
    QUrl url(output.trimmed());

    if (!url.isValid()) {
        qCCritical(KARTON_DEBUG) << "Invalid SPICE URI given to virsh domdisplay";
        return;
    }
    m_host = url.host();
    m_port = url.port(); // defaults to -1 if not found
    m_spiceUri = url.toString();

    if (url.toString().startsWith(QStringLiteral("spice+unix:///tmp/spice"))) {
        qCInfo(KARTON_DEBUG) << "Detected UNIX socket. Connection parameters set.";
    } else {
        qCInfo(KARTON_DEBUG) << "Detected network TCP socket. Connection parameters set.";
    }

    if (!connectToSpice()) {
        qCCritical(KARTON_DEBUG) << "Failed to connect to SPICE";
    }
}

bool DomainViewer::setupSpiceSession()
{
    // TODO: replace virsh CLI, use libvirt API: https://libvirt.org/html/libvirt-libvirt-domain.html#VIR_MIGRATE_PARAM_GRAPHICS_URI
    // once finished, handleHostPort() will set the host and port provided by the output.
    bool commandStarted = m_commandRunner->runCommand(QStringLiteral("virsh domdisplay %1").arg(m_domain->config()->name()));
    return commandStarted;
}

bool DomainViewer::connectToSpice()
{
    qCInfo(KARTON_DEBUG) << "Connecting to SPICE...";
    qCInfo(KARTON_DEBUG) << "   SPICE URI:" << m_spiceUri;
    qCInfo(KARTON_DEBUG) << "   host: " << m_host;
    qCInfo(KARTON_DEBUG) << "   post" << m_port;

    if (!m_domain) {
        qCCritical(KARTON_DEBUG) << "connectToSpice() called but domain is null!";
        return false;
    }

    disconnectFromSpice();
    m_session = spice_session_new();

    qCInfo(KARTON_DEBUG) << "Connecting to URI! -" << m_spiceUri;
    g_object_set(m_session, "uri", m_spiceUri.toUtf8().constData(), NULL);

    g_signal_connect(m_session, "channel-new", G_CALLBACK(DomainViewer::channel_new_callback), this);

    if (!spice_session_connect(m_session)) {
        g_object_unref(m_session);
        m_session = nullptr;
        m_audio = nullptr;
        return false;
    }
    qCInfo(KARTON_DEBUG) << "Established connection! connected to " << domain()->config()->name();
    m_connected = true;

    return true;
}

void DomainViewer::disconnectFromSpice()
{
    stopAudio();

    if (m_session) {
        spice_session_disconnect(m_session);

        g_object_unref(m_session);
        m_session = nullptr;
        m_display_channel = nullptr;
        m_audio = nullptr;
        m_playback_channel = nullptr;
        m_connected = false;
    }
}

void DomainViewer::channel_new_callback(SpiceSession *session, SpiceChannel *channel, gpointer user_data)
{
    Q_UNUSED(session);

    DomainViewer *item = static_cast<DomainViewer *>(user_data);

    item->checkChannelStatus(); // uncomment for channel debug msgs
    if (SPICE_IS_DISPLAY_CHANNEL(channel)) {
        qCInfo(KARTON_DEBUG) << "SPICE display connected";

        spice_channel_connect(channel);
        item->m_display_channel = channel;

        g_signal_connect(channel, "gl-draw", G_CALLBACK(gl_draw_callback), item);
        // TODO: Might need to check if gl is enabled. Domains created by virt-manager are not accel3d by default.

    } else if (SPICE_IS_INPUTS_CHANNEL(channel)) {
        qCInfo(KARTON_DEBUG) << "SPICE: Inputs connected";
        spice_channel_connect(channel);
        item->m_inputs_channel = SPICE_INPUTS_CHANNEL(channel);
    } else if (SPICE_IS_PLAYBACK_CHANNEL(channel)) {
        qCInfo(KARTON_DEBUG) << "SPICE: Audio playback connected";
        spice_channel_connect(channel);
        item->m_playback_channel = SPICE_PLAYBACK_CHANNEL(channel);

        g_signal_connect(channel, "playback-start", G_CALLBACK(playback_start_callback), item);
        g_signal_connect(channel, "playback-data", G_CALLBACK(playback_data_callback), item);
        g_signal_connect(channel, "playback-stop", G_CALLBACK(playback_stop_callback), item);
    } else {
        qCWarning(KARTON_DEBUG) << "Unrecognised SPICE channel type";
    }
}

// ========================== Display rendering  ========================

void DomainViewer::gl_draw_callback(SpiceDisplayChannel *channel, guint x, guint y, guint width, guint height, gpointer user_data)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);

    DomainViewer *item = static_cast<DomainViewer *>(user_data);
    auto scanout = spice_display_get_gl_scanout(channel);
    if (!scanout)
        return;
    item->handleGlScanout(scanout);
    spice_display_gl_draw_done(channel); // releases the GL resource
}
void DomainViewer::handleGlScanout(const SpiceGlScanout *scanout)
{
    if (m_hasScanout && m_scanout.fd >= 0) {
        close(m_scanout.fd);
        m_scanout.fd = -1;
    }

    cleanupEGLImage();

    m_scanout = *scanout; // struct copy

    // duplicate the file descriptor if exists
    // we will be using the duplicate, and the original is freed by SPICE (gl_draw_done).
    if (scanout->fd >= 0) {
        m_scanout.fd = dup(scanout->fd);
        if (m_scanout.fd < 0) {
            qCWarning(KARTON_DEBUG) << "Failed to duplicate scanout FD";
            return;
        }
    }

    m_imageHeight = scanout->height;
    m_imageWidth = scanout->width;
    setImplicitWidth(m_imageWidth);
    setImplicitHeight(m_imageHeight);
    m_hasScanout = true;

    update();
}

void DomainViewer::createTextureFromScanout(const SpiceGlScanout *scanout)
{
    qCDebug(KARTON_DEBUG) << "=== createTextureFromScanout()";
    qCDebug(KARTON_DEBUG) << "FD:" << scanout->fd;
    qCDebug(KARTON_DEBUG) << "Size:" << scanout->width << "x" << scanout->height;
    qCDebug(KARTON_DEBUG) << "Format:" << QStringLiteral("0x%1").arg(scanout->format, 0, 16);
    qCDebug(KARTON_DEBUG) << "Stride:" << scanout->stride;

    if (scanout->fd == -1) {
        return;
    }

    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (!context) {
        qCDebug(KARTON_DEBUG) << "No current OpenGL context";
        return;
    }

    QOpenGLFunctions *gl = context->functions();
    if (!gl) {
        qCDebug(KARTON_DEBUG) << "Failed to get OpenGL functions";
        return;
    }

    // generate texture if empty
    if (m_texId == 0) {
        gl->glGenTextures(1, &m_texId);
        if (m_texId == 0) { // glGenTextures modifies m_texId
            qCDebug(KARTON_DEBUG) << "Failed to generate texture";
            return;
        }
    }

    // setup EGL display
    EGLDisplay display = eglGetCurrentDisplay();
    if (display == EGL_NO_DISPLAY) {
        qCDebug(KARTON_DEBUG) << "Failed to get EGL display";
        return;
    }

    EGLint attrs[] = {EGL_DMA_BUF_PLANE0_FD_EXT,
                      scanout->fd,
                      EGL_DMA_BUF_PLANE0_PITCH_EXT,
                      static_cast<EGLint>(scanout->stride),
                      EGL_DMA_BUF_PLANE0_OFFSET_EXT,
                      0,
                      EGL_WIDTH,
                      static_cast<EGLint>(scanout->width),
                      EGL_HEIGHT,
                      static_cast<EGLint>(scanout->height),
                      EGL_LINUX_DRM_FOURCC_EXT,
                      static_cast<EGLint>(scanout->format),
                      EGL_NONE};

    // create egl image
    if (!m_eglCreateImageKHR) {
        m_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
        if (!m_eglCreateImageKHR) {
            qCDebug(KARTON_DEBUG) << "eglCreateImageKHR not available";
            return;
        }
    }
    m_eglImage = m_eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, attrs);

    if (m_eglImage == EGL_NO_IMAGE_KHR) {
        EGLint error = eglGetError();
        qCDebug(KARTON_DEBUG) << "Failed to create EGL image, error:" << QStringLiteral("0x%1").arg(error, 0, 16);
        return;
    }

    gl->glBindTexture(GL_TEXTURE_2D, m_texId);

    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // set image texture
    if (!m_glEGLImageTargetTexture2DOES) {
        m_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
        if (!m_glEGLImageTargetTexture2DOES) {
            qCDebug(KARTON_DEBUG) << "glEGLImageTargetTexture2DOES not supported";
            return;
        }
    }
    m_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)m_eglImage);

    GLenum glError = gl->glGetError();
    if (glError != GL_NO_ERROR) {
        qCDebug(KARTON_DEBUG) << "OpenGL error after glEGLImageTargetTexture2DOES:" << QStringLiteral("0x%1").arg(glError, 0, 16);
    }

    gl->glBindTexture(GL_TEXTURE_2D, 0); // unbind texture
}

// triggered by update()
QSGNode *DomainViewer::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // create opengl texture from scanout data
    if (m_hasScanout && m_texId == 0) {
        createTextureFromScanout(&m_scanout);
    }

    if (!m_texId) {
        qCDebug(KARTON_DEBUG) << "No texture available yet";
        delete oldNode;
        return nullptr;
    }

    auto *textureNode = static_cast<QSGSimpleTextureNode *>(oldNode);
    if (!textureNode) {
        textureNode = new QSGSimpleTextureNode();
        textureNode->setOwnsTexture(true);
    }

    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (!context) {
        qCDebug(KARTON_DEBUG) << "No current OpenGL context in updatePaintNode";
        return textureNode;
    }

    QOpenGLFunctions *gl = context->functions();
    if (gl) {
        gl->glBindTexture(GL_TEXTURE_2D, m_texId);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glBindTexture(GL_TEXTURE_2D, 0);
    }

    // loading the gl image texture onto the QSG
    QQuickWindow::CreateTextureOptions options;
    QRhi *rhi = window()->rhi();
    if (!rhi) {
        qCDebug(KARTON_DEBUG) << "No RHI available";
        return textureNode;
    }

    QRhiTexture::Format rhiFormat = QRhiTexture::RGBA8;
    QRhiTexture *rhiTexture = rhi->newTexture(rhiFormat, QSize(m_imageWidth, m_imageHeight));
    if (!rhiTexture) {
        qCDebug(KARTON_DEBUG) << "Failed to create RHI texture";
        return textureNode;
    }

    // create native texture to be contained in RHI
    QRhiTexture::NativeTexture nativeTex;
    nativeTex.object = m_texId;
    nativeTex.layout = 0;

    if (!rhiTexture->createFrom(nativeTex)) {
        qCDebug(KARTON_DEBUG) << "Failed to create RHI texture from native";
        delete rhiTexture;
        return textureNode;
    }

    QSGTexture *texture = window()->createTextureFromRhiTexture(rhiTexture, options);
    if (!texture) {
        qCDebug(KARTON_DEBUG) << "Failed to create QSG texture";
        delete rhiTexture;
        return textureNode;
    }

    textureNode->setTexture(texture);
    textureNode->setRect(boundingRect());

    qCDebug(KARTON_DEBUG) << m_domain->config()->name() << ": Successfully updated canvas.";
    qCDebug(KARTON_DEBUG) << "  SPICE Graphics URI: " << m_spiceUri;
    qCDebug(KARTON_DEBUG) << "  Texture ID:" << m_texId;
    qCDebug(KARTON_DEBUG) << "  Size:" << m_imageWidth << "x" << m_imageHeight;

    return textureNode;
}

// cleans up scanout and egl image, textures
void DomainViewer::cleanupEGLResources()
{
    if (m_hasScanout && m_scanout.fd >= 0) {
        close(m_scanout.fd);
        m_scanout.fd = -1;
    }
    m_hasScanout = false;

    cleanupEGLImage();

    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (m_texId && context) {
        QOpenGLFunctions *gl = context->functions();
        if (gl) {
            gl->glDeleteTextures(1, &m_texId);
        }
        m_texId = 0;
    }
}

// for cleanup of duplicate
void DomainViewer::cleanupEGLImage()
{
    if (m_eglImage != EGL_NO_IMAGE_KHR) {
        EGLDisplay display = eglGetCurrentDisplay();
        if (display != EGL_NO_DISPLAY) {
            if (!m_eglDestroyImageKHR) {
                m_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
            }
            if (m_eglDestroyImageKHR) {
                m_eglDestroyImageKHR(display, m_eglImage);
            }
        }
        m_eglImage = EGL_NO_IMAGE_KHR;
    }
}

void DomainViewer::saveFrameToDomain()
{
    if (!window()) {
        qCDebug(KARTON_DEBUG) << "saveFrameToDomain: No window available";
        return;
    }

    if (!isVisible() || width() <= 0 || height() <= 0) {
        qCDebug(KARTON_DEBUG) << "saveFrameToDomain: Item not ready for grabbing";
        return;
    }

    // returns the image of the full window
    QImage fullImage = window()->grabWindow();

    if (!fullImage.isNull()) {
        // crop it to the actual QQI
        QRectF itemBounds = mapRectToScene(boundingRect());
        QRect cropRect = itemBounds.toRect().intersected(fullImage.rect());
        QImage finalImage = cropRect.isEmpty() ? fullImage : fullImage.copy(cropRect);

        qCDebug(KARTON_DEBUG) << "Sending QImage frame to save... - " << finalImage.size() << ", " << m_domain->config()->name();
        m_domain->savePreviewFrame(finalImage.convertToFormat(QImage::Format_RGB32));
    } else {
        qCDebug(KARTON_DEBUG) << "saveFrameToDomain: grabWindow returned null image";
    }
}
// ========================== Audio callbacks =====================

void DomainViewer::playback_start_callback(SpicePlaybackChannel *channel, gint format, gint channels, gint rate, gpointer user_data)
{
    Q_UNUSED(channel);

    DomainViewer *item = static_cast<DomainViewer *>(user_data);
    qCInfo(KARTON_DEBUG) << "Audio playback starting - Format:" << format << "Channels:" << channels << "Rate:" << rate;

    item->stopAudio();

    item->m_audioFormat.setSampleRate(rate);
    item->m_audioFormat.setChannelCount(channels);
    item->m_audioFormat.setSampleFormat(QAudioFormat::Int16);

    item->m_audioSink = new QAudioSink(item->m_audioFormat, item);
    item->m_audioDevice = item->m_audioSink->start();

    qCInfo(KARTON_DEBUG) << "Audio output started successfully";
}

void DomainViewer::playback_data_callback(SpicePlaybackChannel *channel, gpointer data, gint size, gpointer user_data)
{
    Q_UNUSED(channel);

    DomainViewer *item = static_cast<DomainViewer *>(user_data);
    if (item->m_audioDevice) {
        item->m_audioDevice->write(static_cast<const char *>(data), size);
    }
}

void DomainViewer::playback_stop_callback(SpicePlaybackChannel *channel, gpointer user_data)
{
    Q_UNUSED(channel);

    DomainViewer *item = static_cast<DomainViewer *>(user_data);
    qCInfo(KARTON_DEBUG) << "Audio playback stopping";
    item->stopAudio();
}

void DomainViewer::stopAudio()
{
    if (m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
    }

    m_audioDevice = nullptr;
}

// ========================== Input handling =============================

// maps qt provided scancode to pcxt
uint8_t DomainViewer::evdevToPcXt(uint32_t evdev_scancode)
{
    if (evdev_scancode >= CODE_MAP_LINUX_TO_XTKBD_LEN) {
        qCWarning(KARTON_DEBUG) << "Scancode out of range:" << evdev_scancode;
        return 0;
    }

    uint8_t result = code_map_linux_to_xtkbd[evdev_scancode] & 0xFF;
    if (result == 0) {
        qCWarning(KARTON_DEBUG) << "Unknown evdev scancode:" << evdev_scancode;
        return 0;
    }

    qCDebug(KARTON_DEBUG) << "Mapped evdev" << evdev_scancode << "to PC XT" << QString::number(result, 16);
    return result;
}

void DomainViewer::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    quint32 evdev_scancode;
    if (QGuiApplication::platformName() == QStringLiteral("xcb")) { // check if X11
        evdev_scancode = event->nativeScanCode();
    } else { // wayland probably
        evdev_scancode = event->nativeScanCode() - x11_wayland_evdev_offset;
    }

    uint8_t pcxt_scancode = DomainViewer::evdevToPcXt(evdev_scancode); // SPICE accepts PC XT: see inputs channel docs
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
    qreal x = event->position().x();
    qreal y = event->position().y();

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
        // TODO: find horizontal in spice protocol
    } else if (angleDelta.x() < 0) { // scroll left
    }

    qCDebug(KARTON_DEBUG) << "wheel event at (" << x << "," << y << ") delta:" << angleDelta;
}

void DomainViewer::mouseMoveEvent(QMouseEvent *event)
{
    int button_mask = 0;
    if (event->buttons() & Qt::LeftButton)
        button_mask |= SPICE_MOUSE_BUTTON_MASK_LEFT;
    if (event->buttons() & Qt::MiddleButton)
        button_mask |= SPICE_MOUSE_BUTTON_MASK_MIDDLE;
    if (event->buttons() & Qt::RightButton)
        button_mask |= SPICE_MOUSE_BUTTON_MASK_RIGHT;

    qreal x = event->position().x();
    qreal y = event->position().y();

    if (width() > 0 && height() > 0) {
        x = (x * m_imageWidth) / width();
        y = (y * m_imageHeight) / height();
    }

    // note: theres a warning that it's deprecated, but newer version has some bug with drag.
    spice_inputs_position(m_inputs_channel, x, y, 0, button_mask);
    static int moveCounter = 0;
    if (++moveCounter % 10 == 0) {
        qCDebug(KARTON_DEBUG) << "Mouse drag at (" << x << "," << y << "), mask: " << m_current_button_mask;
    }

    event->accept();
}
void DomainViewer::hoverMoveEvent(QHoverEvent *event)
{
    static int hoverCounter = 0;
    if (++hoverCounter % 20 == 0) {
        qCInfo(KARTON_DEBUG) << "Mouse hover at (" << event->position().x() << "," << event->position().y() << ")";
    }
    if (m_inputs_channel && m_connected) {
        qreal x = event->position().x();
        qreal y = event->position().y();

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

    switch (event->button()) {
    case Qt::LeftButton:
        m_current_button_mask |= SPICE_MOUSE_BUTTON_MASK_LEFT;
        spice_inputs_channel_button_press(m_inputs_channel, SPICE_MOUSE_BUTTON_LEFT, m_current_button_mask);
        break;
    case Qt::MiddleButton:
        m_current_button_mask |= SPICE_MOUSE_BUTTON_MASK_MIDDLE;
        spice_inputs_channel_button_press(m_inputs_channel, SPICE_MOUSE_BUTTON_MIDDLE, m_current_button_mask);
        break;
    case Qt::RightButton:
        m_current_button_mask |= SPICE_MOUSE_BUTTON_MASK_RIGHT;
        spice_inputs_channel_button_press(m_inputs_channel, SPICE_MOUSE_BUTTON_RIGHT, m_current_button_mask);
        break;
    default:
        return;
    }

    event->accept();
}

void DomainViewer::mouseReleaseEvent(QMouseEvent *event)
{
    qCInfo(KARTON_DEBUG) << "Mouse release at (" << event->position().x() << "," << event->position().y() << ") button:" << event->button();
    int button = 0;
    switch (event->button()) {
    case Qt::LeftButton:
        button = SPICE_MOUSE_BUTTON_LEFT;
        m_current_button_mask &= ~SPICE_MOUSE_BUTTON_MASK_LEFT;
        break;
    case Qt::MiddleButton:
        button = SPICE_MOUSE_BUTTON_MIDDLE;
        m_current_button_mask &= ~SPICE_MOUSE_BUTTON_MASK_MIDDLE;
        break;
    case Qt::RightButton:
        button = SPICE_MOUSE_BUTTON_RIGHT;
        m_current_button_mask &= ~SPICE_MOUSE_BUTTON_MASK_RIGHT;
        break;
    default:
        return;
    }

    spice_inputs_channel_button_release(m_inputs_channel, button, m_current_button_mask);

    event->accept();
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

