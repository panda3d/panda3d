// Filename: lwoSurfaceBlockVMapName.h
// Created by:  drose (30Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOSURFACEBLOCKVMAPNAME_H
#define LWOSURFACEBLOCKVMAPNAME_H

#include <pandatoolbase.h>

#include "lwoChunk.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoSurfaceBlockVMapName
// Description : Specifies the name of a set of UV's defined on the
//               polygons that use this model.
////////////////////////////////////////////////////////////////////
class LwoSurfaceBlockVMapName : public LwoChunk {
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
    register_type(_type_handle, "LwoSurfaceBlockVMapName",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  
