// Filename: lwoStillImage.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOSTILLIMAGE_H
#define LWOSTILLIMAGE_H

#include <pandatoolbase.h>

#include "lwoChunk.h"

#include <filename.h>

////////////////////////////////////////////////////////////////////
// 	 Class : LwoStillImage
// Description : A single still image associated with a LwoClip chunk.
////////////////////////////////////////////////////////////////////
class LwoStillImage : public LwoChunk {
public:
  Filename _filename;

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
    register_type(_type_handle, "LwoStillImage",
		  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  
