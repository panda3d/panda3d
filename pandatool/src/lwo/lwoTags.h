// Filename: lwoTags.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOTAGS_H
#define LWOTAGS_H

#include <pandatoolbase.h>

#include "lwoChunk.h"

#include <luse.h>
#include <vector_string.h>

////////////////////////////////////////////////////////////////////
// 	 Class : LwoTags
// Description : An array of tag strings that will be referenced by
//               later chunks.
////////////////////////////////////////////////////////////////////
class LwoTags : public LwoChunk {
public:
  int get_num_tags() const;
  string get_tag(int n) const;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  typedef vector_string Tags;
  Tags _tags;
  
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
    register_type(_type_handle, "LwoTags",
		  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  
