/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buffered_datagramconnection.h
 * @author drose
 * @date 2007-03-05
 */


#ifndef __BUFFERED_DATAGRAM_CONNECTION_H__
#define __BUFFERED_DATAGRAM_CONNECTION_H__
/*
 * Ok here is the base behavior.. A message IO engin that is Smart enough to
 * Do 1. Non Blocking Connect .. and Buffer the writes if needed 2. Handle 1
 * to N targets for the connection.. 3. Handle Framing and Unframing properly
 * ..
 */

#include "pandabase.h"
#include "socket_base.h"
#include "datagram.h"
#include "pvector.h"
#include "buffered_datagramreader.h"
#include "buffered_datagramwriter.h"
#include "config_nativenet.h"

// there are 3 states 1. Socket not even assigned,,,, 2. Socket Assigned and
// trying to get a active connect open 3. Socket is open and  writable.. (
// Fully powered up )...
class EXPCL_PANDA_NATIVENET Buffered_DatagramConnection : public Socket_TCP
{
private:
  struct AddressQueue : private pvector<Socket_Address> // this is used to do a round robin for addres to connect to ..
  {
    size_t _active_index;

    INLINE AddressQueue() : _active_index(0) {}

    bool GetNext(Socket_Address &out) {
      size_t the_size = size();
      if (the_size == 0) {
        return false;
      }

      if (_active_index >= the_size) {
        _active_index = 0;
      }

      out = (*this)[_active_index++];
      return true;
    }

    INLINE void clear() {
      pvector<Socket_Address>::clear();
    }

    void push_back(Socket_Address &address) {
      iterator ii;
      for(ii = begin(); ii != end(); ii++)
        if(*ii == address)
          return;
      pvector<Socket_Address>::push_back(address);
    }

    size_t size() { return pvector<Socket_Address>::size(); };
  };

protected:
  // c++ upcalls for
  virtual void PostConnect(void) { };
  virtual void NewWriteBuffer(void) { };

  inline void ClearAll(void);

  inline bool SendMessageBufferOnly(Datagram &msg); // do not use this .. this is a way for the the COnnecting UPcall to drop messages in queue first..
PUBLISHED:
  inline bool GetMessage(Datagram &val);
  inline bool DoConnect(void);           // all the real state magic is in here
  inline bool IsConnected(void);
  inline explicit Buffered_DatagramConnection(int rbufsize, int wbufsize, int write_flush_point) ;
  virtual ~Buffered_DatagramConnection(void) ;
  // the reason thsi all exists
  bool SendMessage(const Datagram &msg);
  inline bool Flush(void);
  inline void Reset(void);

  // int WaitFor_Read_Error(const Socket_fdset & fd, const Time_Span &
  // timeout);

  inline void WaitForNetworkReadEvent(PN_stdfloat MaxTime)
  {
    Socket_fdset  fdset;
    fdset.setForSocket(*this);
    Socket_Selector  selector;
    Time_Span   waittime(MaxTime);
    selector.WaitFor_Read_Error(fdset,waittime);
  }

  // address queue stuff
  inline size_t AddressQueueSize() { return _Addresslist.size(); };
  inline void AddAddress(Socket_Address &inadr);
  inline void ClearAddresses(void);
private:
  Buffered_DatagramWriter _Writer;      // buffered writing
  Buffered_DatagramReader _Reader;      // buffered reader
  AddressQueue            _Addresslist;   // the location of the round robin address list
  Socket_Address          _Adddress;    // the conection address ( active one from list being used)

  friend class Buffered_DatagramReader;
  friend class Buffered_DatagramWriter;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Socket_IP::init_type();
    register_type(_type_handle, "Buffered_DatagramConnection",
                  Socket_IP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

/**
 * used to do a full reset of buffers
 */
inline void Buffered_DatagramConnection::ClearAll(void) {
  nativenet_cat.error() << "Buffered_DatagramConnection::ClearAll Starting Auto Reset\n";
  Close();
  _Writer.ReSet();
  _Reader.ReSet();
}

inline bool Buffered_DatagramConnection::DoConnect(void) {
  if(!_Addresslist.GetNext(_Adddress)) // lookup the proper value...
     return false;

  if(ActiveOpen(_Adddress,true) == true) {
    SetNoDelay();
    SetNonBlocking(); // maybe should be blocking?
    NewWriteBuffer();
    return true;
  }

  return false;

}

/**
 * This is the function that does the connection for us
 */
/*
inline bool Buffered_DatagramConnection::DoConnect(void) {
  if(Active() != true) {
    if(_LastConnectTry.Expired() != true)
      return true;

    if(!_Addresslist.GetNext(_Adddress)) // lookup the proper value...
      return false;

    if(ActiveOpen(_Adddress) == true) {
      _LastConnectTry.ReStart();
      _tryingToOpen = true; // set the flag indicating we are trying to open up
      SetNonBlocking(); // maybe should be blocking?
      SetSendBufferSize(1024*50);  // we need to hand tune these for the os we are using
      SetRecvBufferSize(1024*50);
      NewWriteBuffer();
      return true;
    }

    return true;
  }

  if(_tryingToOpen) {  // okay handle the  i am connecting state....
    Socket_fdset  fdset;
    fdset.setForSocket(*this);
    Socket_Selector  selector;
    if(selector.WaitFor_All(fdset,0) >0) {
      _tryingToOpen = false;
      if(selector._error.IsSetFor(*this) == true) { // means we are in errorconnected. else writable
        ClearAll();
        return false;  // error on connect
      }
      PostConnect();
      return true;  // just got connected
    }
    return true; // still connecting
  }
  return true;
}

*/

/**
 *
 */
inline Buffered_DatagramConnection::~Buffered_DatagramConnection(void)
{
    Close();
}
/**
 *
 */
inline Buffered_DatagramConnection::Buffered_DatagramConnection(int rbufsize, int wbufsize, int write_flush_point)
    :  _Writer(wbufsize,write_flush_point) , _Reader(rbufsize)
{
  nativenet_cat.error() << "Buffered_DatagramConnection Constructor rbufsize = " << rbufsize
                        << " wbufsize = " << wbufsize << " write_flush_point = " << write_flush_point << "\n";
}

inline bool  Buffered_DatagramConnection::SendMessageBufferOnly(Datagram &msg)
{
    int val = _Writer.AddData(msg.get_data(),msg.get_length());
    if(val >= 0)
        return true;

    nativenet_cat.error() << "Buffered_DatagramConnection::SendMessageBufferOnly->Error On Write--Out Buffer = " << _Writer.AmountBuffered() << "\n";
    ClearAll();
    return false;
}

/**
 * must be called to set value to the server
 */
inline void Buffered_DatagramConnection::AddAddress(Socket_Address &inadr)
{
    _Addresslist.push_back(inadr);
}

inline void Buffered_DatagramConnection::ClearAddresses(void)
{
    _Addresslist.clear();
}
/**
 * Reads a message.  Returns false on failure.
 */
inline bool Buffered_DatagramConnection::GetMessage(Datagram  &val)
{
  if(IsConnected())
  {
    int ans1 = _Reader.PumpMessageReader(val,*this);
    if(ans1 == 0)
      return false;
    if(ans1 <0) {
      nativenet_cat.error() << "Buffered_DatagramConnection::GetMessage->Error On PumpMessageReader--Out Buffer = " << _Writer.AmountBuffered() << "\n";
      ClearAll();
      return false;
    }
    return true;
  }
  return false;
}



/**
 * Flush all writes.
 */
bool Buffered_DatagramConnection::Flush(void)
{
    if (IsConnected())
    {
        int flush_resp = _Writer.FlushNoBlock(*this);
        if(flush_resp < 0)
        {
                  nativenet_cat.error() << "Buffered_DatagramConnection::Flush->Error On Flush [" <<GetLastError() << "]\n"

                                       << "Buffered_DatagramConnection::Flush->Error ..Write--Out Buffer = " << _Writer.AmountBuffered() << "\n";
            ClearAll();
            return false;
        }
        return true;
    }
    return false;
}

/**
 * Reset
 */
inline void Buffered_DatagramConnection::Reset() {
  nativenet_cat.error() << "Buffered_DatagramConnection::Reset()\n";
  ClearAll();
}

inline bool Buffered_DatagramConnection::IsConnected(void) {
  return (Active() == true);
}

#endif //__BUFFERED_DATAGRAM_CONNECTION_H__
