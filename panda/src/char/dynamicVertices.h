// Filename: dynamicVertices.h
// Created by:  drose (01Mar99)
//
////////////////////////////////////////////////////////////////////

#ifndef DYNAMICVERTICES_H
#define DYNAMICVERTICES_H

#include <pandabase.h>

#include <pointerToArray.h>
#include <typedObject.h>
#include <luse.h>
#include <pta_Vertexf.h>
#include <pta_Normalf.h>
#include <pta_Colorf.h>
#include <pta_TexCoordf.h>
#include <typedWritable.h>

class BamReader;

////////////////////////////////////////////////////////////////////
//       Class : DynamicVertices
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DynamicVertices : public TypedWritable {
public:
  DynamicVertices();
  DynamicVertices(const DynamicVertices &copy);
  static DynamicVertices deep_copy(const DynamicVertices &copy);

  PTA_Vertexf _coords;
  PTA_Normalf _norms;
  PTA_Colorf _colors;
  PTA_TexCoordf _texcoords;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);  
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "DynamicVertices",
		  TypedWritable::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

