#ifndef __SOCKET_SELECTOR_H__
#define __SOCKET_SELECTOR_H__

////////////////////////////////////////////////////
// This is a structure on purpose. only used as a helper class to save on typing
//
////////////////////////////////////////////////////
struct Socket_Selector
{
    Socket_fdset _read;
    Socket_fdset _write;
    Socket_fdset _error;
    int         _answer;
    
    Socket_Selector() : _answer( -1)
    {
    }
    
    Socket_Selector(const Socket_fdset &fd) : _read(fd), _write(fd), _error(fd) , _answer( -1)
    {
    }
    
    int WaitFor(const Time_Span &timeout);
    int WaitFor_All(const Socket_fdset & fd, const Time_Span & timeout);
    int WaitFor_Read_Error(const Socket_fdset & fd, const Time_Span & timeout);
    int WaitFor_Write_Error(const Socket_fdset & fd, const Time_Span & timeout);
};

//////////////////////////////////////////////////////////////
// Function name : Socket_Selector::WaitFor
// Description   : This function is the reason this call exists..
//      It will wait for a read, write or error condition
//      on a socket or it will time out
//////////////////////////////////////////////////////////////
inline int Socket_Selector::WaitFor(const Time_Span &timeout)
{
    SOCKET local_max = 0;
    if (local_max < _read._maxid)
        local_max = _read._maxid;
    if (local_max < _write._maxid)
        local_max = _write._maxid;
    if (local_max < _error._maxid)
        local_max = _error._maxid;
    
    timeval localtv = timeout.GetTval();
    _answer = DO_SELECT(local_max + 1, &_read._the_set, &_write._the_set, &_error._the_set, &localtv);
    return _answer;
}

//////////////////////////////////////////////////////////////
// Function name : Socket_Selector::WaitFor_All
// Description     : Helper function to utilize the WaitFor function
//////////////////////////////////////////////////////////////
inline int Socket_Selector::WaitFor_All(const Socket_fdset & fd, const Time_Span & timeout)
{
    _read = fd;
    _write = fd;
    _error = fd;
    return WaitFor(timeout);
}

//////////////////////////////////////////////////////////////
// Function name : Socket_Selector::WaitFor_Read_Error
// Description     : Helper function for WaitFor
//      Only looks for readability and errors
//////////////////////////////////////////////////////////////
inline int Socket_Selector::WaitFor_Read_Error(const Socket_fdset & fd, const Time_Span & timeout)
{
    _read = fd;
    _write.clear();
    _error = fd;
    return WaitFor(timeout);
}

//////////////////////////////////////////////////////////////
// Function name : Socket_Selector::WaitFor_Write_Error
// Description     : Helper function for WaitFor
//      Only looks for writability and errors
//////////////////////////////////////////////////////////////
inline int Socket_Selector::WaitFor_Write_Error(const Socket_fdset & fd, const Time_Span & timeout)
{
    _read.clear();
    _write = fd;
    _error = fd;
    return WaitFor(timeout);
}

#endif //__SOCKET_SELECTOR_H__
