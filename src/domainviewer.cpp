#include <spice-client.h>
#include "karton_debug.h"
#include "domainviewer.h"
#include "glib.h"
#include <QString>

DomainViewer::DomainViewer()
{
}

DomainViewer::~DomainViewer()
{
}

bool DomainViewer::connect() {
    SpiceSession *session = spice_session_new();
    if (!spice_session_connect(session)) {
      return false;  
    }
    qCInfo(KARTON_DEBUG) << "yay! connected";


    // SpiceURI *proxy_uri = spice_session_get_proxy_uri(session);
    // gchar *guri = spice_uri_to_string(proxy_uri);
    // QString uri = QString::fromStdString(session->name);
    // free(guri);
    // qCInfo(KARTON_DEBUG) << uri;
    return true;
}