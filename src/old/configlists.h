// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>

#ifndef CONFIGLISTS_H
#define CONFIGLISTS_H

#include <QObject>

class ConfigLists : public QObject
{
    Q_OBJECT
public:
    explicit ConfigLists(QObject *parent = nullptr);
    QStringList osList();
    QStringList firmwareList();

private:
    QStringList m_osList;
    QStringList m_firmwareList;
};

#endif // CONFIGLISTS_H
