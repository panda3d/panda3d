// Filename: lwoLayer.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOLAYER_H
#define LWOLAYER_H

#include <pandatoolbase.h>

#include "lwoChunk.h"

#include <luse.h>

////////////////////////////////////////////////////////////////////
// 	 Class : LwoLayer
// Description : Signals the start of a new layer.  All the data
//               chunks which follow will be included in this layer
//               until another layer chunk is encountered.  If data is
//               encountered before a layer chunk, it goes into an
//               arbitrary layer.
////////////////////////////////////////////////////////////////////
class LwoLayer : public LwoChunk {
public:
  void make_generic();

  enum Flags {
    F_hidden   = 0x0001
  };

  int _number;
  int _flags;
  LPoint3f _pivot;
  string _name;
  int _parent;

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
    register_type(_type_handle, "LwoLayer",
		  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

  
