// Filename: lwoSurfaceBlockRefObj.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOSURFACEBLOCKREFOBJ_H
#define LWOSURFACEBLOCKREFOBJ_H

#include <pandatoolbase.h>

#include "lwoChunk.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoSurfaceBlockRefObj
// Description : Specifies a reference object that the texture UV's
//               are to be computed relative to.
////////////////////////////////////////////////////////////////////
class LwoSurfaceBlockRefObj : public LwoChunk {
public:
  string _name;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;
  
public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LwoChunk::init_type();
    register_type(_type_handle, "LwoSurfaceBlockRefObj",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  
