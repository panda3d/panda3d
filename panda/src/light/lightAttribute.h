// Filename: lightAttribute.h
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LIGHTATTRIBUTE_H
#define LIGHTATTRIBUTE_H

#include <pandabase.h>

#include "light.h"
#include "pt_Light.h"
#include "vector_PT_Light.h"
#include "lightNameClass.h"

#include <multiAttribute.h>
#include <pointerTo.h>

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define MULTIATTRIBUTE_LIGHT MultiAttribute<PT_Light, LightNameClass>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MULTIATTRIBUTE_LIGHT);

////////////////////////////////////////////////////////////////////
// 	 Class : LightAttribute
// Description : See LightTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LightAttribute : public MultiAttribute<PT_Light, LightNameClass> {
public:
  INLINE LightAttribute();

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void output_property(ostream &out, const PT_Light &prop) const;
  virtual void write_property(ostream &out, const PT_Light &prop,
			      int indent_level) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MultiAttribute<PT_Light, LightNameClass>::init_type();
    register_type(_type_handle, "LightAttribute",
		  MultiAttribute<PT_Light, LightNameClass>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "lightAttribute.I"

#endif
