// Filename: bamReader.h
// Created by:  jason (12Jun00)
//

#ifndef __BAM_READER_
#define __BAM_READER_

#include <pandabase.h>
#include <notify.h>

#include "typedWriteable.h"
#include "datagramGenerator.h"
#include "datagramIterator.h"
#include "writeableParam.h"
#include "bamReaderParam.h"
#include "factory.h"
#include "vector_ushort.h"
#include <deque>

//Useful define for reading pta's
#define READ_PTA(Manager, source, Read_func, array)   \
{                                                     \
  void *t;                                            \
  if ((t = Manager->get_pta(source)) == (void*)NULL)  \
  {                                                   \
    array = Read_func(source);                        \
    Manager->register_pta(array.get_void_ptr());      \
  }                                                   \
  else                                                \
  {                                                   \
    array.set_void_ptr(t);                            \
  }                                                   \
}

////////////////////////////////////////////////////////////////////
// 	 Class : BamReader
// Description : This class manages all aspects of reading the data
//               structure from a DatagramGenerator  Requests are made to
//               read an object out of the source.  This class will then
//               read in all the necessary data and pass that data into
//               a factory object for creating TypedWriteable objects.
//               The factory, will call the particular "make" function
//               of the class of the object being created.
//               If that object has pointers to any objects that are
//               also stored in the binary source, then that object will
//               make a request to BamReader for that object to be
//               read and register itself with BamReader.  Once BamReader
//               has read in the objects requested, resolve can be called
//               to go back and complete the pointer requests.  BamReader's
//               interface back into the functions is complete_pointers, 
//               which is defined in TypedWriteable
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BamReader {
public:
  typedef Factory<TypedWriteable> WriteableFactory;
  static BamReader* const Null;
  static WriteableFactory* const NullFactory;

  INLINE BamReader(DatagramGenerator *generator);
  ~BamReader(void);

  bool init(void);
  TypedWriteable* read_object(void);
  //When a client class asks BamReader to read out an object that it
  //points to, it needs to pass a reference to itself, that BamReader
  //can register in itself for later "fixing"
  void read_pointer(DatagramIterator &scan, TypedWriteable* forWhom);
  void read_pointers(DatagramIterator &scan, TypedWriteable* forWhom, int count);

  //At any time you can call this function to try and resolve all
  //outstanding pointer requests.  Will resolve all requests that
  //it can, and ignores those it can't.   This allows for immediate
  //use of part of a full definition if that is wanted.
  bool resolve(void);

  //There are some classes that need to perform certain tasks before they 
  //are useable, but those taks can not be performed before all pointer 
  //references have been completed.  So this function is provided as an 
  //interface to allow each class to register itself to be finalized
  void register_finalize(TypedWriteable *whom);

  //Since there may be an ordering dependency on who gets finalized first,
  //either the parent or the children.  So this function is provided as an
  //interface to force finalization of an object
  void finalize_now(TypedWriteable *whom);

  void *get_pta(DatagramIterator &scan);
  void register_pta(void *ptr);

  TypeHandle read_handle(DatagramIterator &scan);
  

  //Version access functions
  INLINE int get_file_major_ver(void);
  INLINE int get_file_minor_ver(void);

  INLINE int get_current_major_ver(void);
  INLINE int get_current_minor_ver(void);
public:
  INLINE static WriteableFactory* get_factory(void);
private:
  INLINE static void create_factory(void);

private:
  INLINE void queue(PN_uint16);
  void empty_queue(void);
  INLINE void clear_queue(void);
  void finalize_this(TypedWriteable *whom);
  void finalize(void);

private:
  typedef map<int, TypedWriteable*> Created;
  typedef map<TypedWriteable*, vector_ushort> Requests;
  typedef set<TypedWriteable*> Finalize;
  typedef deque<PN_uint16> DeferredReads;
 
  static WriteableFactory *_factory;

  DatagramGenerator *_source;
  //Map of old TypeHandle index number to current TypeHandle
  map<int, TypeHandle> _index_map;
  //Map of unique object ID in Binary structure to created object
  Created _created_objs;
  //Map of Objects that need pointers completed, to the
  //object ID of the objects they need
  Requests _deferred_pointers;
  DeferredReads _deferred_reads;

  //Keep track of all objects needing to be finalized
  Finalize _finalize_list;

  //Map of unique PTA ID's to the PTA's
  map<int, void*> _ptamap;

  //When register_pta is called, this is the unique
  //PTA ID that was stored during the call to 
  //get_pta that failed.  
  int _pta_id;

  int _file_major, _file_minor;
  static const int _cur_major;
  static const int _cur_minor;
};

typedef BamReader::WriteableFactory WriteableFactory;

//Useful function for taking apart the Factory Params
//in the static functions that need to be defined in each
//writeable class that will be generated by a factory.  Sets a
//datagram and a BamReader
INLINE void parse_params(const FactoryParams &, BamReader* &, Datagram &);

#include "bamReader.I"

#endif
