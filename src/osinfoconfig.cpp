// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "osinfoconfig.h"

#include "karton_debug.h"
#include <QFileInfo>
extern "C" // due to undefined references to libosinfo contents
{
#include <osinfo/osinfo.h>
}

OsinfoConfig::OsinfoConfig()
    : m_loader([]() -> OsinfoLoader * {
        if (auto loader = osinfo_loader_new(); loader) {
            g_autoptr(GError) error = nullptr;
            osinfo_loader_process_default_path(loader, &error);
            if (error) {
                qCCritical(KARTON_DEBUG) << "failed to process default path:" << error->message;
                return nullptr;
            }
            return loader;
        }
        qCCritical(KARTON_DEBUG) << "failed to create osinfo loader";
        return nullptr;
    }())
    , m_db([this]() -> OsinfoDb * {
        if (auto db = osinfo_loader_get_db(m_loader.get()); db) {
            return db;
        }
        qCCritical(KARTON_DEBUG) << "failed to get osinfo database";
        return nullptr;
    }())
{
}

OsinfoConfig::~OsinfoConfig() = default;

OsinfoConfig *OsinfoConfig::self()
{
    static OsinfoConfig *o = new OsinfoConfig();
    return o;
}

OsinfoConfig *OsinfoConfig::create(QQmlEngine *qmlEngine, QJSEngine *)
{
    Q_UNUSED(qmlEngine);
    return OsinfoConfig::self();
}

QString OsinfoConfig::getOsIdFromShortId(const QString &short_id)
{
    if (!m_db) {
        qCCritical(KARTON_DEBUG) << "OS database not initialized";
        return QString();
    }
    GObjectPtr<OsinfoOsList> osList(osinfo_db_get_os_list(m_db.get()));
    gint len = osinfo_list_get_length(OSINFO_LIST(osList.get()));

    for (gint i = 0; i < len; i++) {
        OsinfoOs *os = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(osList.get()), i));
        const gchar *id = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
        if (id == short_id.toStdString()) {
            qCInfo(KARTON_DEBUG) << "Found! OS short id:" << id;
            return QString::fromUtf8(osinfo_entity_get_id(OSINFO_ENTITY(os)));
        }
    }
    qCCritical(KARTON_DEBUG) << "Could not find os by short id.";
    return QString();
}

QString OsinfoConfig::getShortIdFromId(const QString &id)
{
    OsinfoOs *os = OSINFO_OS(osinfo_db_get_os(m_db.get(), id.toStdString().c_str()));
    if (!os) {
        return QString();
    }
    const gchar *idG = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
    QString short_id = QString::fromUtf8(idG);
    return short_id;
}

QString OsinfoConfig::getOsIdFromDisk(const QString &isoDiskPath)
{
    if (!m_db) {
        qCCritical(KARTON_DEBUG) << "OS database not initialized";
        return QString();
    }

    QFileInfo fileInfo(isoDiskPath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qCCritical(KARTON_DEBUG) << "file path does not exist:" << isoDiskPath;
        return QString();
    }

    const gchar *location = qUtf8Printable(fileInfo.absoluteFilePath());

    g_autoptr(GError) error = nullptr;
    GObjectPtr<OsinfoMedia> osMedia(osinfo_media_create_from_location(location, NULL, &error));

    if (error) {
        qCCritical(KARTON_DEBUG) << "os_media creation error:" << error->message;
        return QString();
    }

    if (!osinfo_db_identify_media(m_db.get(), osMedia.get())) {
        qCWarning(KARTON_DEBUG) << "could not identify media from disk:" << isoDiskPath;
        return QString();
    }

    const gchar *idG = osinfo_entity_get_id(OSINFO_ENTITY(osMedia.get()));
    QString id = QString::fromUtf8(idG);
    qCInfo(KARTON_DEBUG) << "ID from disk image:" << id;
    id.chop(2);

    return id;
}

QString OsinfoConfig::getOsArchitecture(const QString &osId)
{
    if (!m_db) {
        qCCritical(KARTON_DEBUG) << "OS database not initialized";
        return QString();
    }

    std::string str = osId.toStdString();
    const gchar *os_id = str.c_str();

    OsinfoOs *libosinfo_os = osinfo_db_get_os(m_db.get(), os_id);
    if (!libosinfo_os) {
        qCWarning(KARTON_DEBUG) << "could not find OS with ID:" << osId;
        return QString();
    }

    OsinfoImageList *images = osinfo_os_get_image_list(libosinfo_os);

    if (!images) {
        qCWarning(KARTON_DEBUG) << "OS has no image info:" << osId;
        return QString();
    }

    const gchar *os_arch = osinfo_image_get_architecture(OSINFO_IMAGE(osinfo_list_get_nth(OSINFO_LIST(images), 0)));
    if (!os_arch) { // TODO: not parameterized properly, permanently defaults
        qCWarning(KARTON_DEBUG) << "could not get architecture, default to x86_64.";
        return QString::fromUtf8("x86_64");
    }

    return QString::fromUtf8(os_arch);
}

QStringList OsinfoConfig::getOsVariants()
{
    QStringList osList;

    if (!m_db) {
        qCCritical(KARTON_DEBUG) << "OS database not initialized";
        return osList;
    }

    GObjectPtr<OsinfoOsList> list(osinfo_db_get_os_list(m_db.get()));
    gint len = osinfo_list_get_length(OSINFO_LIST(list.get()));

    for (gint i = 0; i < len; i++) {
        OsinfoOs *os = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(list.get()), i));
        // const gchar *id = osinfo_entity_get_id(OSINFO_ENTITY(os));
        const gchar *id = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
        osList.append(QString::fromUtf8(id));
    }
    osList.sort();
    return osList;
}
