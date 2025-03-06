#pragma once

#include <QObject>
#include <QProcess>
#include <libvirt/libvirt.h>

class Karton : public QObject {
    Q_OBJECT

    public:
        explicit Karton(QObject *parent = nullptr);
        ~Karton();

    public Q_SLOTS:
        Q_INVOKABLE bool runVM(const QString &command);

    Q_SIGNALS:
        void commandFinished(int exitCode, const QString &output);

    private:
        QProcess *m_process;
        virConnectPtr m_conn;
        bool init();
};
