// Filename: eggXfmSAnim.h
// Created by:  drose (19Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGXFMSANIM_H
#define EGGXFMSANIM_H


#include <pandabase.h>

#include "eggGroupNode.h"

class EggXfmAnimData;

////////////////////////////////////////////////////////////////////
// 	 Class : EggXfmSAnim
// Description : This corresponds to an <Xfm$Anim_S$> entry, which is
//               a collection of up to nine <S$Anim> entries that
//               specify the nine components of a transformation.
//               It's implemented as a group that can contain
//               any number of EggSAnimData children.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggXfmSAnim : public EggGroupNode {
public:
  INLINE EggXfmSAnim(const string &name = "",
		     CoordinateSystem cs = CS_default);
  EggXfmSAnim(const EggXfmAnimData &convert_from);

  INLINE EggXfmSAnim(const EggXfmSAnim &copy);
  INLINE EggXfmSAnim &operator = (const EggXfmSAnim &copy);

  INLINE void set_fps(double type);
  INLINE void clear_fps();
  INLINE bool has_fps() const;
  INLINE double get_fps() const;

  INLINE void set_order(const string &order);
  INLINE void clear_order();
  INLINE bool has_order() const;
  INLINE const string &get_order() const;

  INLINE CoordinateSystem get_coordinate_system() const;

  void optimize();
  void normalize();

  int get_num_rows() const;
  void get_value(int row, LMatrix4d &mat) const;
  bool set_value(int row, const LMatrix4d &mat);

  INLINE void clear_data();
  bool add_data(const LMatrix4d &mat);

  virtual void write(ostream &out, int indent_level) const;

protected:
  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
			   CoordinateSystem to_cs);
  virtual void r_mark_coordsys(CoordinateSystem cs);

private:
  double _fps;
  bool _has_fps;
  string _order;
  CoordinateSystem _coordsys;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggGroupNode::init_type();
    register_type(_type_handle, "EggXfmSAnim",
                  EggGroupNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "eggXfmSAnim.I"

#endif


