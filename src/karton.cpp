#include "karton.h"
#include <QDebug>

Karton::Karton(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this)) {

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus) {
            QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
            Q_EMIT commandFinished(exitCode, output);
        });
}
Karton::~Karton() {
    if (m_process->state() == QProcess::Running) {
        m_process->terminate();
        m_process->waitForFinished(1000);
    }
}

bool Karton::runVM(const QString &command)
{
    qDebug() << "Running VM:" << command;
    m_process->start(command);
    return m_process->waitForStarted();
}