// Filename: guiItem.I
// Created by:  cary (01Nov00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiItem::GuiItem(void) : Namable("fubar"), _left(-1.), _right(1.),
				_bottom(-1.), _top(1.), _pri(P_Normal) {}

INLINE float GuiItem::get_scale(void) const {
  return _scale;
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
