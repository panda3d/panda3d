// Filename: lwoInputFile.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOINPUTFILE_H
#define LWOINPUTFILE_H

#include <pandatoolbase.h>

#include "iffInputFile.h"

#include <luse.h>

////////////////////////////////////////////////////////////////////
// 	 Class : LwoInputFile
// Description : A specialization of IffInputFile to handle reading a
//               Lightwave Object file.
////////////////////////////////////////////////////////////////////
class LwoInputFile : public IffInputFile {
public:
  LwoInputFile();
  ~LwoInputFile();

  int get_vx();
  LVecBase3f get_vec3();
  Filename get_filename();

protected:
  virtual IffChunk *make_new_chunk(IffId id);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    IffInputFile::init_type();
    register_type(_type_handle, "LwoInputFile",
		  IffInputFile::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  
