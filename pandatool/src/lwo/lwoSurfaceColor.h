// Filename: lwoSurfaceColor.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOSURFACECOLOR_H
#define LWOSURFACECOLOR_H

#include <pandatoolbase.h>

#include "lwoChunk.h"

#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : LwoSurfaceColor
// Description : Records the base color of a surface, as an entry
//               within a LwoSurface chunk.
////////////////////////////////////////////////////////////////////
class LwoSurfaceColor : public LwoChunk {
public:
  RGBColorf _color;
  int _envelope;

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
    register_type(_type_handle, "LwoSurfaceColor",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  
