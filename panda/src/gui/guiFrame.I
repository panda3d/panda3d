// Filename: guiFrame.I
// Created by:  cary (01Nov00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiFrame::GuiFrame(void) {}

INLINE bool GuiFrame::is_aligned_left(void) const {
  return _align_to_left;
}

INLINE bool GuiFrame::is_aligned_right(void) const {
  return _align_to_right;
}

INLINE bool GuiFrame::is_aligned_top(void) const {
  return _align_to_top;
}

INLINE bool GuiFrame::is_aligned_bottom(void) const {
  return _align_to_bottom;
}

INLINE float GuiFrame::get_left_gap(void) const {
  return _left_gap;
}

INLINE float GuiFrame::get_right_gap(void) const {
  return _right_gap;
}

INLINE float GuiFrame::get_top_gap(void) const {
  return _top_gap;
}

INLINE float GuiFrame::get_bottom_gap(void) const {
  return _bottom_gap;
}

INLINE void GuiFrame::clear_left_alignment(void) {
  _align_to_left = false;
}

INLINE void GuiFrame::clear_right_alignment(void) {
  _align_to_right = false;
}

INLINE void GuiFrame::clear_top_alignment(void) {
  _align_to_top = false;
}

INLINE void GuiFrame::clear_bottom_alignment(void) {
  _align_to_bottom = false;
}

INLINE void GuiFrame::clear_all_alignment(void) {
  clear_left_alignment();
  clear_right_alignment();
  clear_top_alignment();
  clear_bottom_alignment();
}

INLINE void GuiFrame::align_to_left(float g) {
  _align_to_right = false;
  _align_to_left = true;
  _left_gap = g;
}

INLINE void GuiFrame::align_to_right(float g) {
  _align_to_left = false;
  _align_to_right = true;
  _right_gap = g;
}

INLINE void GuiFrame::align_to_top(float g) {
  _align_to_bottom = false;
  _align_to_top = true;
  _top_gap = g;
}

INLINE void GuiFrame::align_to_bottom(float g) {
  _align_to_top = false;
  _align_to_bottom = true;
  _bottom_gap = g;
}

