// Filename: iffGenericChunk.h
// Created by:  drose (23Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef IFFGENERICCHUNK_H
#define IFFGENERICCHUNK_H

#include <pandatoolbase.h>

#include "iffChunk.h"

#include <datagram.h>


////////////////////////////////////////////////////////////////////
//       Class : IffGenericChunk
// Description : A class for a generic kind of IffChunk that is not
//               understood by a particular IffReader.  It remembers
//               its entire contents.
////////////////////////////////////////////////////////////////////
class IffGenericChunk : public IffChunk {
public:
  INLINE IffGenericChunk();

  INLINE const Datagram &get_data() const;
  INLINE void set_data(const Datagram &data);

  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  Datagram _data;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    IffChunk::init_type();
    register_type(_type_handle, "IffGenericChunk",
                  IffChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "iffGenericChunk.I"

#endif

  
