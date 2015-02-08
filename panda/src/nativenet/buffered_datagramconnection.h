#ifndef __NONECLOCKING_CONNECTTION_H_
#define __NONECLOCKING_CONNECTTION_H_
////////////////////////////////////////////////////////////////////
// 
// Ok here is the base behavior..
//     A message IO engin that is Smart enough to Do 
//
//  1. Non Blocking Connect .. and Buffer the writes if needed
//  2. Handle 1 to N targets for the connection.. 
//
//  3. Handle Framing and Unframing properly ..
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "socket_base.h"
#include "datagram.h"
#include "pvector.h"
#include "buffered_datagramreader.h"
#include "buffered_datagramwriter.h"
#include "config_nativenet.h"

#ifdef HAVE_PYTHON
#include "py_panda.h"
#endif

////////////////////////////////////////////////////////////////
// there are 3 states
//
//      1. Socket not even assigned,,,,
//      2. Socket Assigned and trying to get a active connect open
//      3. Socket is open and  writable.. ( Fully powered up )...
//
///////////////////////////////////////////////////////////////
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
  ///////////////////////////////////////////
  inline void ClearAll(void);

  inline bool SendMessageBufferOnly(Datagram &msg); // do not use this .. this is a way for the the COnnecting UPcall to drop messages in queue first..
PUBLISHED:
  inline bool GetMessage(Datagram &val);
  inline bool DoConnect(void);           // all the real state magic is in here
  inline bool IsConnected(void); 
  inline Buffered_DatagramConnection(int rbufsize, int wbufsize, int write_flush_point) ;
  virtual ~Buffered_DatagramConnection(void) ;
  // the reason thsi all exists
  inline bool SendMessage(const Datagram &msg);
  inline bool Flush(void);
  inline void Reset(void);

//  int WaitFor_Read_Error(const Socket_fdset & fd, const Time_Span & timeout);

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

////////////////////////////////////////////////////////////////////
// Function name    : Buffered_DatagramConnection::ClearAll
// Description      :  used to do a full reset of buffers
//  
// Return type      : inline void 
// Argument         : void
////////////////////////////////////////////////////////////////////
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

/*
////////////////////////////////////////////////////////////////////
// Function name    : Buffered_DatagramConnection::DoConnect
// Description      : This is the function thah does the conection for us
//  
// Return type      : inline bool 
// Argument         : void
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
// Function name    : Buffered_DatagramConnection::~Buffered_DatagramConnection
// Description      : 
//  
// Return type      : inline 
// Argument         : void
////////////////////////////////////////////////////////////////////
inline Buffered_DatagramConnection::~Buffered_DatagramConnection(void) 
{
    Close();
}
////////////////////////////////////////////////////////////////////
// Function name    : Buffered_DatagramConnection::Buffered_DatagramConnection
// Description      : 
//  
// Return type      : inline 
// Argument         : bool do_blocking_writes
// Argument         : int rbufsize
// Argument         : int wbufsize
////////////////////////////////////////////////////////////////////
inline Buffered_DatagramConnection::Buffered_DatagramConnection(int rbufsize, int wbufsize, int write_flush_point) 
    :  _Writer(wbufsize,write_flush_point) , _Reader(rbufsize) 
{
  nativenet_cat.error() << "Buffered_DatagramConnection Constructor rbufsize = " << rbufsize 
                        << " wbufsize = " << wbufsize << " write_flush_point = " << write_flush_point << "\n";
}
////////////////////////////////////////////////////////////////////
// Function name    :  Buffered_DatagramConnection::SendMessage
// Description      : send the message 
//  
// Return type      : inline bool 
// Argument         : DataGram &msg
////////////////////////////////////////////////////////////////////
inline bool  Buffered_DatagramConnection::SendMessage(const Datagram &msg)
{
      if(IsConnected()) {
//        printf(" DO SendMessage %d\n",msg.get_length()); 
        int val = 0;        
        val = _Writer.AddData(msg.get_data(),msg.get_length(),*this);
        if(val >= 0)
          return true;
        // Raise an exception to give us more information at the python level
        nativenet_cat.warning() << "Buffered_DatagramConnection::SendMessage->Error On Write--Out Buffer = " << _Writer.AmountBuffered() << "\n";
        #ifdef HAVE_PYTHON
        ostringstream s;

#if PY_MAJOR_VERSION >= 3
        PyObject *exc_type = PyExc_ConnectionError;
#else
        PyObject *exc_type = PyExc_StandardError;
#endif
        
        s << endl << "Error sending message: " << endl;
        msg.dump_hex(s);
        
        s << "Message data: " << msg.get_data() << endl;
        
        string message = s.str();
        PyErr_SetString(exc_type, message.c_str());
        #endif
    
        ClearAll();
      }
      return false;
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

////////////////////////////////////////////////////////////////////
// Function name    : Buffered_DatagramConnection::Init
// Description      :  must be called to set value to the server
//  
// Return type      : inline void 
// Argument         : Socket_Address &inadr
////////////////////////////////////////////////////////////////////
inline void Buffered_DatagramConnection::AddAddress(Socket_Address &inadr)
{
    _Addresslist.push_back(inadr);
}

inline void Buffered_DatagramConnection::ClearAddresses(void)
{
    _Addresslist.clear();
}
////////////////////////////////////////////////////////////////////
// Function name    : Buffered_DatagramConnection::GetMessage
// Description      :  read a message
//  
//  false means something bad happened..
//
//
// Return type      : inline bool 
// Argument         : Datagram &val
////////////////////////////////////////////////////////////////////
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



////////////////////////////////////////////////////////////////////
// Function name    : Buffered_DatagramConnection::Flush
// Description      : flush all wrightes
//  
// Return type      : bool 
// Argument         : void
////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////
// Function name    : Buffered_DatagramConnection::Reset
// Description      : Reset 
//  
// Return type      : void 
// Argument         : void
////////////////////////////////////////////////////////////////////
inline void Buffered_DatagramConnection::Reset()
{
  nativenet_cat.error() << "Buffered_DatagramConnection::Reset()\n";
  ClearAll();
};


inline bool Buffered_DatagramConnection::IsConnected(void) {
  return ( Active() == true );
}


#endif //__NONECLOCKING_CONNECTTION_H_

