// Filename: lwoChunk.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOCHUNK_H
#define LWOCHUNK_H

#include <pandatoolbase.h>

#include "iffChunk.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LwoChunk
// Description : A specialization of IffChunk for Lightwave Object
//               files.  Each kind of chunk that is specific to a
//               Lightwave file should inherit directly or indirectly
//               from LwoChunk.
////////////////////////////////////////////////////////////////////
class LwoChunk : public IffChunk {
public:
  // No particular interface here.

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
    register_type(_type_handle, "LwoChunk",
		  IffChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "lwoChunk.I"

#endif

  
