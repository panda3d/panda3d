// Filename: bamWriter.h
// Created by:  jason (08Jun00)
//

#ifndef __BAM_WRITER_
#define __BAM_WRITER_

#include <pandabase.h>
#include <notify.h>
#include <nspr.h>

#include "typedWriteable.h"
#include "datagramSink.h"
#include <deque>

#define WRITE_PTA(Manager, dest, Write_func, array)  \
  if (!Manager->register_pta(dest, array.p()))       \
  {                                                  \
    Write_func(dest, array);                         \
  }                                                  \


////////////////////////////////////////////////////////////////////
// 	 Class : BamWriter
// Description : This class manages all aspects of writing data 
//               structures to some Binary form.  It writes to a 
//               a DatagramSink which is an abstraction that could
//               be a file, the net, etc...   The two basic functions
//               used are write_object and write_pointer.  write_object
//               is called to write any TypedWriteable object out.  
//               BamWriter actually asks the object passed to write
//               itself out to a datagram, and then it writes that
//               datagram to the DatagramSink.   write_pointer is called
//               by the objects themselves to resolve writing out 
//               pointers to other objects.  BamWriter will handle all
//               circular references correctly
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BamWriter{
public:
  INLINE BamWriter(DatagramSink *sink);
  ~BamWriter(void);
  
  bool init(void);
  bool write_object(TypedWriteable *obj);
  void write_pointer(Datagram &packet, TypedWriteable *dest);
  
  //This function is provided for writing out shared pointers
  //to PTA's.  You should pass in a pointer to the PTA you want
  //to write.  If BamWriter has not already been asked to register
  //this pointer, it will register it, write the appropriate info
  //into the Datagram and return false.  If it has, it will write
  //the reference into the Datagram and return true.  If false is
  //returned, then the class registering the PTA should write out
  //the info into the Datagram itself
  bool register_pta(Datagram &packet, void* ptr);
   
  void write_handle(Datagram &packet, TypeHandle type);
private:
  class storeState {
  public:
    PN_uint16 objId;
    bool written;
    
    storeState(void) : objId(0), written(false) {}
    ~storeState(void) {}
  };
  
  void enqueue(TypedWriteable *obj);
  bool empty_queue(void);
  
  DatagramSink *_target;
  bool _writing;
  deque<TypedWriteable*> _deferred;
  set<int> _type_map;
  map<TypedWriteable*, storeState> _statemap;
  map<void*, int> _ptamap;
  PN_uint16 _current, _current_pta;
  bool _emptying;
};

#include "bamWriter.I"

#endif

