// Filename: guiItem.I
// Created by:  cary (01Nov00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiItem::GuiItem(void) : Namable("fubar"), _scale(1.), _scale_x(1.),
				_scale_y(1.), _scale_z(1.), _left(-1.),
				_right(1.), _bottom(-1.), _top(1.),
				_pri(P_Normal) {}

#include <get_rel_pos.h>

INLINE LVector3f GuiItem::get_graph_scale(void) {
  if (_alt_root.is_null())
    return LVector3f(1., 1., 1.);
  // pass NULL for the 'to' to go to root
  return get_rel_scale(_alt_root, (Node*)0L);
}

INLINE LPoint3f GuiItem::get_graph_pos(void) {
  if (_alt_root.is_null())
    return LPoint3f(0., 0., 0.);
  // pass NULL for the 'to' to go to root
  return get_rel_pos(_alt_root, (Node*)0L);
}

INLINE float GuiItem::get_scale(void) const {
  return _scale;
}

INLINE float GuiItem::get_scale_x(void) const {
  return _scale_x;
}

INLINE float GuiItem::get_scale_y(void) const {
  return _scale_y;
}

INLINE float GuiItem::get_scale_z(void) const {
  return _scale_z;
}

INLINE LVector3f GuiItem::get_pos(void) const {
  return _pos;
}

INLINE float GuiItem::get_left(void) const {
  return _left;
}

INLINE float GuiItem::get_right(void) const {
  return _right;
}

INLINE float GuiItem::get_bottom(void) const {
  return _bottom;
}

INLINE float GuiItem::get_top(void) const {
  return _top;
}

INLINE LVector4f GuiItem::get_frame(void) const {
  return LVector4f(_left, _right, _bottom, _top);
}

INLINE float GuiItem::get_width(void) const {
  return _right - _left;
}

INLINE float GuiItem::get_height(void) const {
  return _top - _bottom;
}

INLINE GuiItem::Priority GuiItem::get_priority(void) const {
  return _pri;
}

INLINE void GuiItem::recompute(void) {
  this->recompute_frame();
}

INLINE ostream& operator<<(ostream& os, GuiItem& item) {
  item.output(os);
  return os;
}
