// Filename: guiItem.h
// Created by:  cary (01Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIITEM_H__
#define __GUIITEM_H__

#include "guiManager.h"

#include <eventHandler.h>

class EXPCL_PANDA GuiItem : public TypedReferenceCount, public Namable {
PUBLISHED:
  enum Priority { P_Lowest, P_Low, P_Normal, P_High, P_Highest };

protected:
  bool _added_hooks;
  float _scale, _scale_x, _scale_y, _scale_z, _left, _right, _bottom, _top;
  LVector3f _pos;
  GuiManager* _mgr;
  Priority _pri;
  PT_Node _alt_root;

  INLINE GuiItem(void);
  virtual void recompute_frame(void) = 0;
  virtual void adjust_region(void);

  INLINE void get_graph_mat(LMatrix4f&);

PUBLISHED:
  GuiItem(const string&);
  virtual ~GuiItem(void);

  virtual void manage(GuiManager*, EventHandler&) = 0;
  virtual void manage(GuiManager*, EventHandler&, Node*) = 0;
  virtual void unmanage(void) = 0;

  virtual int freeze();
  virtual int thaw();

  virtual void set_scale(float) = 0;
  virtual void set_scale(float, float, float) = 0;
  virtual void set_pos(const LVector3f&) = 0;
  virtual void set_priority(GuiLabel*, const Priority) = 0;
  virtual void set_priority(GuiItem*, const Priority) = 0;

  INLINE float get_scale(void) const;
  INLINE float get_scale_x(void) const;
  INLINE float get_scale_y(void) const;
  INLINE float get_scale_z(void) const;
  INLINE LVector3f get_pos(void) const;
  INLINE float get_left(void) const;
  INLINE float get_right(void) const;
  INLINE float get_bottom(void) const;
  INLINE float get_top(void) const;
  INLINE LVector4f get_frame(void) const;
  INLINE float get_width(void) const;
  INLINE float get_height(void) const;
  INLINE Priority get_priority(void) const;

  INLINE void recompute(void);

  virtual int set_draw_order(int) = 0;

  virtual void output(ostream&) const = 0;
public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GuiItem",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#include "guiItem.I"

#endif /* __GUIITEM_H__ */
