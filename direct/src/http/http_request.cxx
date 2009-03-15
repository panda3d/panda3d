#include "socket_base.h"


#include "http_connection.h"
#include "baseincomingset.h"
#include "socket_base.h"


#include "http_request.h"

TypeHandle Http_Request::_type_handle;

typedef BaseIncomingSet< Http_Request , Socket_TCP_Listen , char [10240], char *>  Http_Source_BaseIncomingSet;
std::set< Http_Request * >                       Global_WebRequests_pendingNotify;
static Http_Source_BaseIncomingSet                      Global_HttpManager;

bool Http_Request::HttpManager_Initialize( unsigned short port)
{
    init_network();
    Socket_Address address;
    address.set_port(port);
    return Global_HttpManager.init(address);
}

Http_Request * Http_Request::HttpManager_GetARequest()
{
    Time_Clock  Know;
    Global_HttpManager.PumpAll(Know);
    Http_Request * answer = NULL;
    std::set< Http_Request * >::iterator ii = Global_WebRequests_pendingNotify.begin();
    if(ii != Global_WebRequests_pendingNotify.end())
    {   
        answer = *ii;
        Global_WebRequests_pendingNotify.erase(ii);   
    }
    return   answer;
}

