// Filename: showHideTransition.h
// Created by:  drose (26Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SHOWHIDETRANSITION_H
#define SHOWHIDETRANSITION_H

#include <pandabase.h>

#include "showHideNameClass.h"

#include <multiTransition.h>
#include <pointerTo.h>
#include <camera.h>

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define MULTITRANSITION_CAMERA MultiTransition<PT(Camera), ShowHideNameClass>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MULTITRANSITION_CAMERA);

////////////////////////////////////////////////////////////////////
// 	 Class : ShowHideTransition
// Description : This transition controls which cameras can view the
//               geometry.  By default, all cameras see everything;
//               you can restrict the visibility of a particular
//               subgraph to an arbitrary subset of cameras (or to no
//               cameras at all).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ShowHideTransition : public MultiTransition<PT(Camera), ShowHideNameClass> {
public:
  INLINE ShowHideTransition();
  INLINE static ShowHideTransition all_off();
  INLINE static ShowHideTransition all_on();
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;
  virtual NodeTransition *make_identity() const;

protected:
  virtual void output_property(ostream &out, const PT(Camera) &prop) const;
  virtual void write_property(ostream &out, const PT(Camera) &prop,
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
    MultiTransition<PT(Camera), ShowHideNameClass>::init_type();
    register_type(_type_handle, "ShowHideTransition",
		  MultiTransition<PT(Camera), ShowHideNameClass>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class ShowHideAttribute;
};

#include "showHideTransition.I"

#endif


