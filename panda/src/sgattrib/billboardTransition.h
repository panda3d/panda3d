// Filename: billboardTransition.h
// Created by:  mike (19Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef BILLBOARDTRANSITION_H
#define BILLBOARDTRANSITION_H

#include <pandabase.h>

#include <immediateTransition.h>
#include <luse.h>

////////////////////////////////////////////////////////////////////
// 	 Class : BillboardTransition
// Description : This transition, when applied to an arc, causes that
//               arc and everything below it to be rendered so that it
//               always faces the camera.  There are all kinds of ways
//               that billboards can be set to rotate.
//
//               A BillboardTransition is neither on nor off, and it
//               does not compose with nested BillboardTransitions.
//               Instead, it has an immediate effect.  Once a
//               billboard transition is in place, it affects
//               everything below it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BillboardTransition : public ImmediateTransition {
public:
  INLINE BillboardTransition();
  INLINE static BillboardTransition axis(CoordinateSystem cs = CS_default);
  INLINE static BillboardTransition point_eye(CoordinateSystem cs = CS_default);
  INLINE static BillboardTransition point_world(CoordinateSystem cs = CS_default);

  INLINE void set_up_vector(const LVector3f &up_vector);
  INLINE LVector3f get_up_vector() const;

  INLINE void set_eye_relative(bool eye_relative);
  INLINE bool get_eye_relative() const;

  INLINE void set_axial_rotate(bool axial_rotate);
  INLINE bool get_axial_rotate() const;

  virtual NodeTransition *make_copy() const;

  virtual bool sub_render(NodeRelation *arc,
			  const AllAttributesWrapper &attrib,
			  AllTransitionsWrapper &trans,
			  GraphicsStateGuardianBase *gsgbase);
  virtual bool has_sub_render() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

private:
  LVector3f _up_vector;
  bool _eye_relative;
  bool _axial_rotate;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  static TypedWriteable *make_BillboardTransition(const FactoryParams &params);

protected:
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
    ImmediateTransition::init_type();
    register_type(_type_handle, "BillboardTransition",
		  ImmediateTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "billboardTransition.I"

#endif
