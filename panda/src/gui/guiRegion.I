// Filename: guiRegion.I
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

INLINE GuiRegion::GuiRegion(void) {
}

INLINE GuiRegion::GuiRegion(const string& name, float l, float r, float b,
			    float t, bool buttons) : Namable(name), _left(l),
						     _right(r), _bottom(b),
						     _top(t) {
  _region = new MouseWatcherRegion(name, l, r, b, t);
  _region->set_suppress_below(buttons);
}

INLINE MouseWatcherRegion* GuiRegion::get_region(void) const {
  test_ref_count_integrity();
  _region->test_ref_count_integrity();
  return _region;
}

INLINE void GuiRegion::trap_clicks(bool t) {
  test_ref_count_integrity();
  _region->test_ref_count_integrity();
  _region->set_suppress_below(t);
}

INLINE void GuiRegion::set_region(float left, float right, float bottom,
				  float top) {
  test_ref_count_integrity();
  _region->test_ref_count_integrity();
  _region->set_frame(left, right, bottom, top);
  _left = left;
  _right = right;
  _bottom = bottom;
  _top = top;
}

INLINE LVector4f GuiRegion::get_frame(void) const {
  test_ref_count_integrity();
  _region->test_ref_count_integrity();
  return LVector4f(_left, _right, _bottom, _top);
}
