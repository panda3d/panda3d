// Filename: lwoSurfaceBlockChannel.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOSURFACEBLOCKCHANNEL_H
#define LWOSURFACEBLOCKCHANNEL_H

#include <pandatoolbase.h>

#include "lwoChunk.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoSurfaceBlockChannel
// Description : Indicates which channel the texture in this
//               LwoSurfaceBlock is applied to.  This is a subchunk of
//               LwoSurfaceBlockHeader.
////////////////////////////////////////////////////////////////////
class LwoSurfaceBlockChannel : public LwoChunk {
public:
  IffId _channel_id;

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
    register_type(_type_handle, "LwoSurfaceBlockChannel",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  
