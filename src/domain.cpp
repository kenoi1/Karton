// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domain.h"
#include <QDebug>
#include <QString>


Domain::Domain(const QString& name, const QString& uuid, bool isActive) 
    : m_name(name), m_uuid(uuid), m_isActive(isActive) {
}

