// Filename: lwoSurfaceBlockHeader.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOSURFACEBLOCKHEADER_H
#define LWOSURFACEBLOCKHEADER_H

#include <pandatoolbase.h>

#include "lwoGroupChunk.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoSurfaceBlockHeader
// Description : The header chunk within a LwoSurfaceBlock chunk.
////////////////////////////////////////////////////////////////////
class LwoSurfaceBlockHeader : public LwoGroupChunk {
public:
  string _ordinal;

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
    register_type(_type_handle, "LwoSurfaceBlockHeader",
                  LwoGroupChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  


