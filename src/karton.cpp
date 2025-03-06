#include "karton.h"
#include <QDebug>
#include <libvirt/libvirt.h>
#include <iostream>

Karton::Karton(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this)) {

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus) {
            QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
            Q_EMIT commandFinished(exitCode, output);
        });

    init();
}

Karton::~Karton() {
    virConnectClose(m_conn);
    if (m_process->state() == QProcess::Running) {
        m_process->terminate();
        m_process->waitForFinished(1000);
    }
}
bool Karton::init() {
    m_conn = virConnectOpen("qemu:///session");
    if (!m_conn) {
        std::cerr << "Failed to connect to hypervisor" << std::endl;
        return false;
    }
    
    std::cout << "Connected to hypervisor" << std::endl;

    // Print vms
    virDomainPtr *domains = nullptr;
    int numDomains = virConnectListAllDomains(m_conn, &domains, 0);
    
    if (numDomains < 0) {
        std::cerr << "Failed to list domains" << std::endl;
    } else {
        std::cout << "Total VMs: " << numDomains << std::endl;
        
        for (int i = 0; i < numDomains; i++) {
            const char* name = virDomainGetName(domains[i]);
            bool isActive = virDomainIsActive(domains[i]);
            
            std::cout << "  VM: " << name 
                     << " (" << (isActive ? "running" : "stopped") << ")" 
                     << std::endl;
            
            virDomainFree(domains[i]);
        }
        
        free(domains);
    }
    //
    return true;
}

bool Karton::runVM(const QString &command)
{
    qDebug() << "Running VM:" << command;
    m_process->start(command);
    return m_process->waitForStarted();
}