// Filename: lwoSurfaceBlockTMap.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOSURFACEBLOCKTMAP_H
#define LWOSURFACEBLOCKTMAP_H

#include <pandatoolbase.h>

#include "lwoGroupChunk.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LwoSurfaceBlockTMap
// Description : The tMap chunk within a LwoSurfaceBlock chunk.
////////////////////////////////////////////////////////////////////
class LwoSurfaceBlockTMap : public LwoGroupChunk {
public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

  virtual IffChunk *make_new_chunk(IffInputFile *in, IffId id);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LwoGroupChunk::init_type();
    register_type(_type_handle, "LwoSurfaceBlockTMap",
		  LwoGroupChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  


