// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>

#include "domain.h"
#include "domainconfig.h"
#include <QString>

Domain::Domain(QObject *parent)
    : QObject(parent)
    , m_domainPtr(nullptr)
    , m_config(new DomainConfig(this))
{
}

Domain::Domain(const virDomainPtr domainPtr,
               const Domain *config,
               QObject *parent)
    : QObject(parent)
    , m_domainPtr(domainPtr)
    , m_config(config)
{
}

Domain::~Domain()
{
    if (m_domainPtr) {
        virDomainFree(m_domainPtr);
    }
}
