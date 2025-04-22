// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Derek Lin <derekhongdalin@gmail.com>


// #include <spice-client.h>

#pragma once

#include <QObject>

class DomainViewer : public QObject
{
    Q_OBJECT

public:
    DomainViewer();
    ~DomainViewer();

    bool connect();
};