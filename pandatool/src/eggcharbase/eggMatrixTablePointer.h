// Filename: eggMatrixTablePointer.h
// Created by:  drose (26Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGMATRIXTABLEPOINTER_H
#define EGGMATRIXTABLEPOINTER_H

#include <pandatoolbase.h>

#include "eggBackPointer.h"

#include <eggTable.h>
#include <eggXfmSAnim.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
// 	 Class : EggMatrixTablePointer
// Description : This stores a pointer back to an EggXfmSAnim table
//               (i.e. an <Xfm$Anim_S$> entry in an egg file),
//               corresponding to the animation data from a single
//               bundle for this joint.
////////////////////////////////////////////////////////////////////
class EggMatrixTablePointer : public EggBackPointer {
public:
  EggMatrixTablePointer(EggObject *object);

private:
  PT(EggTable) _table;
  PT(EggXfmSAnim) _xform;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggBackPointer::init_type();
    register_type(_type_handle, "EggMatrixTablePointer",
		  EggBackPointer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#endif


