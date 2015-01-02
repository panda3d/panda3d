////////////////////////////////////////////////////////////////////
// Function name    : BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::AddFromListener
// Description      : Read incoming connections off the listener
//
// Return type      : inline void
// Argument         : void
////////////////////////////////////////////////////////////////////
template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
inline void BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::AddFromListener(void)
{
    Socket_Address  Addr1;
    SOCKET          newsck;


    while(_Listener.GetIncomingConnection(newsck,Addr1) == true)
    {
        CloseState cl= ProcessNewConnection(newsck);
        if(cl == ConnectionDoNotClose)
        {
            _INCLASS1 * newt = new _INCLASS1(newsck,Addr1);
            AddAConection(newt);
        }
        else
            DO_CLOSE(newsck);
    }

}
////////////////////////////////////////////////////////////////////
// Function name    : BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::PumpReader
// Description      : Tries to read a record off the incoming socket
//
// Return type      : inline void
// Argument         : Time_Clock  currentTime
////////////////////////////////////////////////////////////////////
template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
inline int BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::PumpReader(Time_Clock  &currentTime)
{
    MESSAGE_READER_BUF      message;

    iterator lpNext, lp;
    for (lpNext  = lp = BaseClass::begin(); lp != BaseClass::end() ; lp = lpNext)
    {
        lpNext++;

        int ans = (*lp)->ReadIt(message, sizeof(message),currentTime);
        if(ans < 0)
        {
            delete *lp;
            this->erase(lp);
        }
        if(ans > 0)
        {
            CloseState cs = (*lp)->ProcessMessage(message,currentTime);
            if( cs == ConnectionDoClose)
            {
                delete *lp;
                this->erase(lp);
            }
        }
    }
    return 0;
}
////////////////////////////////////////////////////////////////////
// Function name    : BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::AddAConection
// Description      : Adds a member to the base container
//
// Return type      : inline void
// Argument         : _INCLASS1 * newt
////////////////////////////////////////////////////////////////////
template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
inline void BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::AddAConection(_INCLASS1 * newt)
{
    this->push_back(newt);
}
////////////////////////////////////////////////////////////////////
// Function name    : BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::BaseIncomingSet
// Description      :  core constructor
//
// Return type      : inline
// Argument         : void
////////////////////////////////////////////////////////////////////
template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
inline BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::BaseIncomingSet(void)
{

}

template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::BaseIncomingSet(BaseIncomingSet &in)
{

}
////////////////////////////////////////////////////////////////////
// Function name    : BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::~BaseIncomingSet
// Description      : The Destructot .. will delete all members.. ??
//
// Return type      :
////////////////////////////////////////////////////////////////////
template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::~BaseIncomingSet()
{
    for(iterator ii = BaseClass::begin(); ii != BaseClass::end(); ii++)
        delete *ii;
}
////////////////////////////////////////////////////////////////////
// Function name    : & BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::GetListener
// Description      : Retyurns a pointer to the listener in this class
//
// Return type      : inline _IN_LISTEN
// Argument         : void
////////////////////////////////////////////////////////////////////
template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
inline _IN_LISTEN & BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::GetListener(void)
{
    return this->Listener;
};
////////////////////////////////////////////////////////////////////
// Function name    : BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::init
// Description      : the second part of the 2 phase power up.. Opends the listener
//
// Return type      : inline bool
// Argument         : Socket_Address &WhereFrom
////////////////////////////////////////////////////////////////////
template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
inline bool BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::init(Socket_Address &WhereFrom)
{
    if(_Listener.OpenForListen(WhereFrom,true) != true)
        return false;
    _Listener.SetNonBlocking();
    return true;
}
////////////////////////////////////////////////////////////////////
// Function name    : BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::PumpAll
// Description      : THis is the polled interface to this class
//
// Return type      : inline void
// Argument         : Time_Clock  &currentTime
////////////////////////////////////////////////////////////////////
template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
inline void BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::PumpAll(Time_Clock  &currentTime)
{
    PumpReader(currentTime); // I MOVED ORDER TO FINE TUNE THE READ OPERATIONS
    AddFromListener();
}

////////////////////////////////////////////////////////////////////
// Function name    : BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::ProcessNewConnection
// Description      :  this is the vertual function call when a new connection is created
//                      manly here for loging if needed...
//
// Return type      : CloseState
// Argument         : SOCKET  socket
////////////////////////////////////////////////////////////////////
template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
CloseState BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::ProcessNewConnection(SOCKET  socket)
{
    return ConnectionDoNotClose;
}
////////////////////////////////////////////////////////////////////
// Function name    : void BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::AddToFDSet
// Description      : Add LIstener and Client to the FD set for polled reading
//
// Return type      : inline
// Argument         : Socket_Selector &set
////////////////////////////////////////////////////////////////////
template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
inline  void BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::AddToFDSet(Socket_fdset &set1)
{
    if(_Listener.Active())
        set1.setForSocket(_Listener);
    iterator lp;

    for (lp = BaseClass::begin(); lp != BaseClass::end(); lp = lp++)
        set1.setForSocket((*lp)->val);
}

template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
inline BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS> &BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::operator=
    (BaseIncomingSet &inval)
{
     if (&inval == this) return *this;
    _Listener = inval._Listener;
    return *this;
}


template <class _INCLASS1,class _IN_LISTEN,typename  MESSAGE_READER_BUF, typename  MESSAGE_READER_UPPASS>
inline void BaseIncomingSet<_INCLASS1,_IN_LISTEN,MESSAGE_READER_BUF,MESSAGE_READER_UPPASS>::Reset()
{
    _Listener.Close();
    iterator lpNext, lp;
    for (lpNext  = lp = BaseClass::begin(); lp != BaseClass::end() ; lp = lpNext)
    {
        lpNext++;
        (*lp)->Reset();
        delete *lp;
        this->erase(lp);
    }
}



