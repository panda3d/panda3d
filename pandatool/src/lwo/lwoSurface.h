// Filename: lwoSurface.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOSURFACE_H
#define LWOSURFACE_H

#include <pandatoolbase.h>

#include "lwoGroupChunk.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LwoSurface
// Description : Describes the shading attributes of a surface.  This
//               is similar to the concept usually called a "material"
//               in other file formats.
////////////////////////////////////////////////////////////////////
class LwoSurface : public LwoGroupChunk {
public:
  string _name;
  string _source;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

  virtual IffChunk *make_new_chunk(IffInputFile *in, IffId id);

private:
  static TypeHandle _type_handle;
};

#endif

  
