// Filename: lwoClip.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOCLIP_H
#define LWOCLIP_H

#include <pandatoolbase.h>

#include "lwoGroupChunk.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LwoClip
// Description : A single image file, or a numbered sequence of images
//               (e.g. a texture-flip animation).
////////////////////////////////////////////////////////////////////
class LwoClip : public LwoGroupChunk {
public:
  int _index;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

  virtual IffChunk *make_new_chunk(IffInputFile *in, IffId id);

private:
  static TypeHandle _type_handle;
};

#endif

  
