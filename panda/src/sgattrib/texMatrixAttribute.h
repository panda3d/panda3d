// Filename: texMatrixAttribute.h
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXMATRIXATTRIBUTE_H
#define TEXMATRIXATTRIBUTE_H

#include <pandabase.h>

#include <lmatrix4fTransition.h>
#include <lmatrix.h>

////////////////////////////////////////////////////////////////////
// 	 Class : TexMatrixAttribute
// Description : See TexMatrixTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexMatrixAttribute : public LMatrix4fAttribute {
public:
  INLINE TexMatrixAttribute();

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LMatrix4fAttribute::init_type();
    register_type(_type_handle, "TexMatrixAttribute",
		  LMatrix4fAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "texMatrixAttribute.I"

#endif
