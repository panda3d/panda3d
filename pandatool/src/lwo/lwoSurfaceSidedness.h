// Filename: lwoSurfaceSidedness.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOSURFACESIDEDNESS_H
#define LWOSURFACESIDEDNESS_H

#include <pandatoolbase.h>

#include "lwoChunk.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoSurfaceSidedness
// Description : Records whether polygons are frontfacing only or
//               backfacing also.  This is associated with the
//               LwoSurface chunk.
////////////////////////////////////////////////////////////////////
class LwoSurfaceSidedness : public LwoChunk {
public:
  enum Sidedness {
    S_front          = 1,
    S_front_and_back = 3
  };

  Sidedness _sidedness;

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
    register_type(_type_handle, "LwoSurfaceSidedness",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  
