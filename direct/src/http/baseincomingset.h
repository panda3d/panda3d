#ifndef __BASEINCOMINGSET_H__
#define __BASEINCOMINGSET_H__

#include <list>
#include "socket_base.h"

enum CloseState
{
    ConnectionDoNotClose,
    ConnectionDoClose
};
// RHH
////////////////////////////////////////////////////////////////////
//   Template :BaseIncomingSet
//
// Description :  A base structre for a listening socket and a
//              set of connection that have been received with there read functions..
//
//  Think of this like a web server with 1 listening socket and 0-n open reacting conections..
//
//  The general operation if get connection..
//          do you have a message
//          process message
//          go back to do you have a message or close connection
//
//
////////////////////////////////////////////////////////////////////
template < class _INCLASS1,class _IN_LISTEN, class MESSAGE_READER_BUF, class MESSAGE_READER_UPPASS> class BaseIncomingSet : public  std::list<_INCLASS1 *>
{
    typedef std::list<_INCLASS1 *> BaseClass;
    typedef TYPENAME BaseClass::iterator iterator;
    _IN_LISTEN                  _Listener;

    inline void AddFromListener(void);
    inline int PumpReader(Time_Clock  &currentTime);
    inline void AddAConection(_INCLASS1 * newt);

public:

//  typedef typename BaseIncomingSet<_INCLASS1, _IN_LISTEN, MESSAGE_READER_BUF, MESSAGE_READER_UPPASS>::LinkNode LinkNode;

//  typedef SentDblLinkListNode_Gm   SentDblLinkListNode_Gm;
    inline BaseIncomingSet(void);
    inline BaseIncomingSet(BaseIncomingSet &in);
    virtual ~BaseIncomingSet();

    inline _IN_LISTEN & GetListener(void);
    inline bool init(Socket_Address &WhereFrom);
    inline void PumpAll(Time_Clock  &currentTime);
    virtual CloseState ProcessNewConnection(SOCKET  socket);
    inline  void AddToFDSet(Socket_fdset &set);

//  inline  LinkNode *          GetRoot(void) {  return &this->sentenal; };
    BaseIncomingSet &operator=( BaseIncomingSet &inval);
    void Reset();
};

#include "baseincomingset.i"

#endif //__BASEINCOMINGSET_H__



