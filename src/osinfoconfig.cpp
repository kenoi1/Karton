// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "osinfoconfig.h"
#include "karton_debug.h"
#include <QString>

extern "C" // due to undefined references to libosinfo contents
{
#include <osinfo/osinfo.h>
}

OsinfoConfig::OsinfoConfig()
    : m_loader(nullptr), m_db(nullptr)
{
    initOsDb();
}

OsinfoConfig::~OsinfoConfig()
{
    if (m_loader)
    {
        g_object_unref(m_loader);
    }
}

bool OsinfoConfig::initOsDb()
{
    m_loader = osinfo_loader_new();
    if (!m_loader)
    {
        qCCritical(KARTON_DEBUG) << "failed to create osinfo loader";
        return false;
    }

    m_db = osinfo_loader_get_db(m_loader);
    if (!m_db)
    {
        qCCritical(KARTON_DEBUG) << "failed to get osinfo database";
        return false;
    }

    GError *error = NULL;
    osinfo_loader_process_default_path(m_loader, &error);
    if (error)
    {
        qCCritical(KARTON_DEBUG) << "failed to process default path:" << error->message;
        g_error_free(error);
        return false;
    }

    return true;
}

QString OsinfoConfig::getOsIdFromShortId(const QString &short_id)
{
    if (!m_db)
    {
        qCCritical(KARTON_DEBUG) << "OS database not initialized";
        return QString();
    }
    OsinfoOsList *osList = osinfo_db_get_os_list(m_db);
    gint len = osinfo_list_get_length(OSINFO_LIST(osList));

    for (gint i = 0; i < len; i++)
    {
        OsinfoOs *os = OSINFO_OS(osinfo_list_get_nth(OSINFO_LIST(osList), i));
        const gchar *id = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
        qCInfo(KARTON_DEBUG) << "OS SHORT ID:" << id;
        if (id == short_id.toStdString())
        {
            return QString::fromUtf8(osinfo_entity_get_id(OSINFO_ENTITY(os)));
        }
    }
    qCCritical(KARTON_DEBUG) << "Could not find os by short_id.";
    return QString();
}

QString OsinfoConfig::getShortIdFromId(const QString &id)
{
    OsinfoOs *os = OSINFO_OS(osinfo_db_get_os(m_db, id.toStdString().c_str()));
    if (!os) {
        return QString();
    }
    const gchar *idG = osinfo_product_get_short_id(OSINFO_PRODUCT(os));
    QString short_id = QString::fromUtf8(idG);
    return short_id;
}

QString OsinfoConfig::getOsIdFromDisk(const QString &isoDiskPath)
{
    if (!m_db)
    {
        qCCritical(KARTON_DEBUG) << "OS database not initialized";
        return QString();
    }

    std::string str = isoDiskPath.toStdString();
    const gchar *location = str.c_str();

    GError *error = NULL;
    OsinfoMedia *osMedia = osinfo_media_create_from_location(location, NULL, &error);

    if (error)
    {
        qCCritical(KARTON_DEBUG) << "os_media creation error:" << error->message;
        g_error_free(error);
        return QString();
    }

    if (!osinfo_db_identify_media(m_db, osMedia))
    {
        qCWarning(KARTON_DEBUG) << "could not identify media from disk:" << isoDiskPath;
        g_object_unref(osMedia);
        return QString();
    }

    const gchar *idG = osinfo_entity_get_id(OSINFO_ENTITY(osMedia));
    QString id = QString::fromUtf8(idG);
    qCWarning(KARTON_DEBUG) << "ID:" << id;
    g_object_unref(osMedia);
    id.chop(2);
    return id;
}

QString OsinfoConfig::getOsArchitecture(const QString &osId)
{
    if (!m_db)
    {
        qCCritical(KARTON_DEBUG) << "OS database not initialized";
        return QString();
    }

    std::string str = osId.toStdString();
    const gchar *os_id = str.c_str();

    OsinfoOs *libosinfo_os = osinfo_db_get_os(m_db, os_id);
    if (!libosinfo_os)
    {
        qCWarning(KARTON_DEBUG) << "could not find OS with ID:" << osId;
        return QString();
    }

    OsinfoImageList *images = osinfo_os_get_image_list(libosinfo_os);

    if (!images)
    {
        qCWarning(KARTON_DEBUG) << "OS has no image info:" << osId;
        return QString();
    }

    const gchar *os_arch = osinfo_image_get_architecture(OSINFO_IMAGE(osinfo_list_get_nth(OSINFO_LIST(images), 0)));
    if (!os_arch)
    { // the above code is cursed WIP
        qCWarning(KARTON_DEBUG) << "could not get architecture, default to x86_64.";
        return QString::fromUtf8("x86_64");
    }

    return QString::fromUtf8(os_arch);
}